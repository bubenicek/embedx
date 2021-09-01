
PROJECT_DIRS += $(EMBEDX_ROOT)/core/lib/video/rxs_streamer/include
PROJECT_DIRS += $(EMBEDX_ROOT)/core/lib/video/rxs_streamer/src

PROJECT_SOURCEFILES += rxs_encoder.c
PROJECT_SOURCEFILES += rxs_packetizer.c
PROJECT_SOURCEFILES += rxs_sender_raw.c

LIBS += -luv -lhiredis -lvpx
