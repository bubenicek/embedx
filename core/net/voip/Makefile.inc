
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/codec
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/codec/g711
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/driver
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/driver/test
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/driver/max9860
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/driver/camera
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/rtp
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/sip
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/pj
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/pj/pj
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/pj/pjmedia

PROJECT_SOURCEFILES += voip_timer.c
PROJECT_SOURCEFILES += voip_audio.c
PROJECT_SOURCEFILES += voip_video.c
PROJECT_SOURCEFILES += voip_phone.c

# PJ
PROJECT_SOURCEFILES += sock_stm32.c
PROJECT_SOURCEFILES += rand.c
PROJECT_SOURCEFILES += os_time_stm32.c
PROJECT_SOURCEFILES += os_timestamp_stm32.c
PROJECT_SOURCEFILES += pj_rtp.c
PROJECT_SOURCEFILES += pj_rtcp.c

# Audiocodec
PROJECT_SOURCEFILES += audio_codec.c
PROJECT_SOURCEFILES += audio_codec_g711.c

# Audio driver
PROJECT_SOURCEFILES += audio_driver.c
PROJECT_SOURCEFILES += video_driver.c
PROJECT_SOURCEFILES += test_video_driver.c
PROJECT_SOURCEFILES += camera_video_driver.c
PROJECT_SOURCEFILES += max9860_audio_driver.c


MODULES += core/net/voip/sip
MODULES += core/net/voip/rtp

ifeq ($(TARGET),linux)
	PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/platform/linux
	PROJECT_SOURCEFILES += voip_linux_platform.c
endif

ifeq ($(TARGET),esp32)
	PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/platform/esp32
	PROJECT_SOURCEFILES += voip_platform_esp32.c
endif

ifeq ($(TARGET),esp32-cam)
	PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/voip/platform/esp32
	PROJECT_SOURCEFILES += voip_platform_esp32.c
endif
