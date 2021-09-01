
PROJECT_DIRS += $(ROOT)/core/net/zrtp/include
PROJECT_DIRS += $(ROOT)/core/net/zrtp/src
PROJECT_DIRS += $(ROOT)/core/net/zrtp/third_party/bgaes
PROJECT_DIRS += $(ROOT)/core/net/zrtp/third_party/bnlib

MODULES += core/net/zrtp/third_party/bgaes

# bnlib
PROJECT_SOURCEFILES += bninit32.c
PROJECT_SOURCEFILES += lbn00.c
PROJECT_SOURCEFILES += bn.c
PROJECT_SOURCEFILES += lbnmem.c
PROJECT_SOURCEFILES += sieve.c
PROJECT_SOURCEFILES += prime.c
PROJECT_SOURCEFILES += bnprint.c
PROJECT_SOURCEFILES += jacobi.c
PROJECT_SOURCEFILES += germain.c

MODULES += core/net/zrtp/src

