/**
 * \file sbrk.c         \brief syscals functions implementation
 */

#include "system.h"

/**
 *  This is _sbrk implementation based on the following articles:
 *
 *  newlib sbrk documentation (file newlib-1.20.0\newlib\libc\sys.tex)
 *
 *  http://ieee.uwaterloo.ca/coldfire/gcc-doc/docs/porting_3.html
 *
 *  http://www.freertos.org/FreeRTOS_Support_Forum_Archive/November_2009/freertos_sbrk_and_newlib_-_is_any_malloc_or_free_used_3447091.html
 *
 *
 *  For more info about malloc see:
 *
 *  http://balakrishnan-tech.blogspot.cz/
 *
 *  http://www.cs.princeton.edu/courses/archive/fall07/cos217/lectures/15Memory-3x1.pdf
 */
caddr_t _sbrk(int incr)
{
    extern char _end;      /* end of bss section, defined in linker script */
    extern char _eheap;    /* end of heap, defined in linker script */
    static caddr_t heap_end = NULL;
    caddr_t prev_heap_end;
    caddr_t next_heap_end;

    if (NULL == heap_end)
    {
        heap_end = (caddr_t)&_end;
    }

    prev_heap_end = heap_end;

    /* align to 8 byte boundary */
    next_heap_end = (caddr_t) (((unsigned int) (heap_end + incr) + 7) & ~7);

    if (next_heap_end >= (caddr_t)&_eheap)
    {
        /* not enough memory */
        return ((caddr_t)0);
    }
    else
    {
        heap_end = next_heap_end;

        return (caddr_t) prev_heap_end;
    }
}
