
#ifndef __COMPILER_H
#define __COMPILER_H

#define throw_exception(_e) goto _e

#define atomic(_ex) do { \
   DISABLE_INTERRUPTS(); \
   _ex; \
   ENABLE_INTERRUPTS(); \
} while(0)

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifndef __PACKED__
#define __PACKED__   __attribute__ ((__packed__))
#endif   // __PACKED__

#endif // __COMPILER_H
