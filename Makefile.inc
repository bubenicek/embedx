# -*- makefile -*-

ifndef EMBEDX_ROOT
  EMBEDX_ROOT=../..
  ${info Using default EMBEDX_ROOT}
endif

include $(EMBEDX_ROOT)/MakeVars

ifeq ($(TARGET),)
  -include Makefile.target
  ifeq ($(TARGET),)
    ${info TARGET not defined, using target 'linux'}
    TARGET=linux
  else
    ${info using saved target '$(TARGET)'}
  endif
endif

BUILD_DIR = build
PROJECT_TARGET=$(BUILD_DIR)/$(PROJECT).$(TARGET).elf

ifeq ($(DEFINES),)
  -include Makefile.$(TARGET).defines
  ifneq ($(DEFINES),)
    ${info using saved defines '$(DEFINES)'}
  endif
endif

ifndef HOST_OS
  ifeq ($(OS),Windows_NT)
  ## TODO: detect more specific Windows set-ups,
  ## e.g. CygWin, MingW, VisualC, Watcom, Interix
    HOST_OS := Windows
  else
    HOST_OS := $(shell uname)
  endif
endif

#More debug information when running in CI
ifdef CI
  ifeq ($(CI),true)
    V = 1
  endif
endif

usage:
	@echo "make MAKETARGETS... [TARGET=(TARGET)] [savetarget] [targets]"

targets:
	@ls -1 $(EMBEDX_ROOT)/platform $(TARGETDIRS) | grep -v CVS

savetarget:
	-@rm -f Makefile.target
	@echo "saving Makefile.target"
	@echo >Makefile.target "TARGET = $(TARGET)"

savedefines:
	-@rm -f Makefile.$(TARGET).defines
	@echo "saving Makefile.$(TARGET).defines"
	@echo >Makefile.$(TARGET).defines "DEFINES = $(DEFINES)"

OBJECTDIR = $(BUILD_DIR)/obj_$(TARGET)

LOWERCASE = -abcdefghijklmnopqrstuvwxyz
UPPERCASE = _ABCDEFGHIJKLMNOPQRSTUVWXYZ
TARGET_UPPERCASE := ${strip ${shell echo $(TARGET) | sed y!$(LOWERCASE)!$(UPPERCASE)!}}
CFLAGS += -DPROJECT_TARGET_$(TARGET_UPPERCASE)=1

MODULES += core/sys core/dev core/lib core/net platform cpu

PROJECT_DIRS += ${addprefix $(EMBEDX_ROOT)/core/,dev lib sys . }

oname = ${patsubst %.c,%.o,${patsubst %.S,%.o,$(1)}}

PROJECT_OBJECTFILES += ${addprefix $(OBJECTDIR)/,${call oname, $(PROJECT_SOURCEFILES)}}

# Provide way to create $(OBJECTDIR) if it has been removed by make clean
$(OBJECTDIR):
	$(Q) mkdir -p $(BUILD_DIR)
	$(Q) mkdir -p $@

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

### Include application makefiles

ifdef APPS
  APPDS = ${wildcard ${foreach DIR, $(APPDIRS), ${addprefix $(DIR)/, $(APPS)}}} \
             ${wildcard ${addprefix $(EMBEDX_ROOT)/apps/, $(APPS)} \
             ${addprefix $(EMBEDX_ROOT)/platform/$(TARGET)/apps/, $(APPS)} \
             $(APPS)}
  APPINCLUDES = ${foreach APP, $(APPS), ${wildcard ${foreach DIR, $(APPDS), $(DIR)/Makefile.$(APP)}}}
  -include $(APPINCLUDES)
  APP_SOURCES = ${foreach APP, $(APPS), $($(APP)_src)}
  DSC_SOURCES = ${foreach APP, $(APPS), $($(APP)_dsc)}
  PROJECT_SOURCEFILES += $(APP_SOURCES) $(DSC_SOURCES)
endif

### Include target makefile (TODO Unsafe?)

target_makefile := $(wildcard $(EMBEDX_ROOT)/platform/$(TARGET)/Makefile.$(TARGET) ${foreach TDIR, $(TARGETDIRS), $(TDIR)/$(TARGET)/Makefile.$(TARGET)})

# Check if the target makefile exists, and create the object directory if necessary.
ifeq ($(strip $(target_makefile)),)
  ## Try find target makefile in custom app directory
  target_makefile := $(wildcard ../../platform/$(TARGET)/Makefile.$(TARGET) ${foreach TDIR, $(TARGETDIRS), $(TDIR)/$(TARGET)/Makefile.$(TARGET)})
  ifeq ($(strip $(target_makefile)),)
    ${error The target platform "$(TARGET)" does not exist (maybe it was misspelled?)}
  endif
endif
  
ifneq (1, ${words $(target_makefile)})
  ${error More than one TARGET Makefile found: $(target_makefile)}
endif
include $(target_makefile)

ifdef MODULES
  UNIQUEMODULES = $(call uniq,$(MODULES))
  MODULEDIRS = ${wildcard ${addprefix $(EMBEDX_ROOT)/, $(UNIQUEMODULES)}}
  MODULES_SOURCES = ${foreach d, $(MODULEDIRS), ${subst ${d}/,,${wildcard $(d)/*.c}}}
  PROJECT_SOURCEFILES += $(MODULES_SOURCES)
  APPDS += $(MODULEDIRS)
endif


### Verbosity control. Use  make V=1  to get verbose builds.
ifeq ($(V),1)
  TRACE_CC =
  TRACE_CXX =
  TRACE_LD =
  TRACE_AR =
  TRACE_AS =
  Q=
else
  TRACE_CC = @echo "  CC       " $<
  TRACE_CXX = @echo " CXX      " $<
  TRACE_LD = @echo "  LD       " $@
  TRACE_AR = @echo "  AR       " $@
  TRACE_AS = @echo "  AS       " $<
  Q=@
endif

### Forward comma-separated list of arbitrary defines to the compiler

COMMA := ,
CFLAGS += ${addprefix -D,${subst $(COMMA), ,$(DEFINES)}}

### Setup directory search path for source and header files

PROJECT_TARGET_DIRS_CONCAT = ${addprefix ${dir $(target_makefile)}, $(PROJECT_TARGET_DIRS)}
PROJECT_CPU_DIRS_CONCAT    = ${addprefix $(PROJECT_CPU)/, $(PROJECT_CPU_DIRS)}

SOURCEDIRS = . $(PROJECT_DIRS) $(PROJECT_TARGET_DIRS_CONCAT) $(PROJECT_CPU_DIRS_CONCAT) $(APPDS) ${dir $(target_makefile)}

vpath %.c $(SOURCEDIRS)
vpath %.S $(SOURCEDIRS)

CFLAGS += ${addprefix -I,$(SOURCEDIRS) $(EMBEDX_ROOT)}

### Automatic dependency generation

ifneq ($(MAKECMDGOALS),clean)
-include ${addprefix $(OBJECTDIR)/, $(PROJECT_SOURCEFILES:.c=.d)}
endif

### See http://make.paulandlesley.org/autodep.html#advanced

define FINALIZE_DEPENDENCY
cp $(@:.o=.d) $(@:.o=.$$$$); \
sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
    -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.$$$$) >> $(@:.o=.d); \
rm -f $(@:.o=.$$$$)
endef

clean:
	$(Q)rm -rf $(BUILD_DIR)

ifndef CUSTOM_RULE_C_TO_CE
%.ce: %.c
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) -c $< -o $@
	$(STRIP) --strip-unneeded -g -x $@
endif

ifndef CUSTOM_RULE_C_TO_OBJECTDIR_O
$(OBJECTDIR)/%.o: %.c | $(OBJECTDIR)
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) -MMD -c $< -o $@
	@$(FINALIZE_DEPENDENCY)
endif

ifndef CUSTOM_RULE_S_TO_OBJECTDIR_O
$(OBJECTDIR)/%.o: %.S | $(OBJECTDIR)
	$(TRACE_AS)
	$(Q)$(AS) $(ASFLAGS) -o $@ $<
endif

ifndef CUSTOM_RULE_C_TO_O
%.o: %.c
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) -c $< -o $@
endif

ifndef AROPTS
  AROPTS = rcf
endif

ifndef LD
  LD = $(CC)
endif

ifndef CUSTOM_RULE_LINK
$(PROJECT): $(PROJECT_OBJECTFILES) $(PROJECT_LIBRARIES)
	$(TRACE_LD)
	$(Q)$(LD) $(LDFLAGS) $^ $(LIBS) -o $(PROJECT_TARGET)
endif
