
#ifndef __KERNEL_COMMON_H
#define __KERNEL_COMMON_H

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>

#define TRACE(_format, ...)         printk(KERN_INFO TRACE_TAG ": " _format "\n", ## __VA_ARGS__)
#define TRACE_ERROR(_format, ...)   printk(KERN_ALERT TRACE_TAG ": *E " _format "\n", ## __VA_ARGS__)

#define os_malloc(_size)            kmalloc(_size, GFP_KERNEL)
#define os_free(_ptr)               kfree(_ptr)

#define CRITICAL_SECTION_BEGIN()    volatile unsigned long irq_flags; local_irq_save(irq_flags)
#define CRITICAL_SECTION_END()      local_irq_restore(irq_flags)

#endif   // __KERNEL_COMMON_H

