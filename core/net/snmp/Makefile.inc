
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/snmp
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/snmp/src
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/snmp/src/transports
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/snmp/include

PROJECT_SOURCEFILES += snmpc.c

PROJECT_SOURCEFILES += snmpSocketBaseDomain.c
PROJECT_SOURCEFILES += snmpUDPBaseDomain.c
PROJECT_SOURCEFILES += snmpUDPDomain.c
PROJECT_SOURCEFILES += snmpUDPIPv4BaseDomain.c
PROJECT_SOURCEFILES += snmpIPv4BaseDomain.c

MODULES += core/net/snmp/src
