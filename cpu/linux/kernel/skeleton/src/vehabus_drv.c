/**
 * \file vehabus_drv.c        \brief VEHABUS kernel module
 */

#include "kernel_common.h"
#include "vehabus_drv.h"

#define TRACE_TAG "vehabus"

//
// Device variables
//
static struct device *dev = NULL;
static struct class *dev_class = NULL;
static int dev_major;
static spinlock_t dev_lock;

/** Refresh timer */
static struct timer_list poll_timer;


static int device_open(struct inode* inode, struct file* filp)
{
   TRACE("Device open");
	return 0;
}

static int device_close(struct inode* inode, struct file* filp)
{
   TRACE("Device close");
	return 0;
}

static ssize_t device_read(struct file* filp, char __user *buffer, size_t length, loff_t* offset)
{
   TRACE("Device read");
   return length;
}

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset)
{
   TRACE("Device write length: %d", length);
   return length;
}

static long device_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
   TRACE("Device ioctl");
   return -EINVAL;
}

/**
 * The file_operation scructure tells the kernel which device operations are handled.
 * For a list of available file operations, see http://lwn.net/images/pdf/LDD3/ch03.pdf 
 */
static struct file_operations fops =
{
   .unlocked_ioctl = device_ioctl,
   .read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_close
};


static void poll_timer_callback(unsigned long param)
{
   unsigned long flags;

   spin_lock_irqsave(&dev_lock, flags);
   
   TRACE("vehabus tick");
   

   // Restart timer
   mod_timer(&poll_timer, jiffies + msecs_to_jiffies(VEHABUS_POLL_TIMER_INTERVAL));

   spin_unlock_irqrestore(&dev_lock, flags);
}


/** Initialize the device driver module */
static int __init module_driver_init(void)
{
	int retval;

   TRACE("%s driver init, ver. %s - %s %s", DEVICE_NAME, DRIVER_VERSION, __DATE__, __TIME__);

   spin_lock_init(&dev_lock);

	// First, see if we can dynamically allocate a major for our device
	dev_major = register_chrdev(0, DEVICE_NAME, &fops);
	if (dev_major < 0)
	{
		TRACE_ERROR("Failed to register device: error %d", dev_major);
		retval = dev_major;
		goto failed_chrdevreg;
	}

	// We can either tie our device to a bus (existing, or one that we create)
   // or use a "virtual" device class. For this example, we choose the latter
	dev_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(dev_class))
	{
		TRACE_ERROR("Failed to register device class '%s'", CLASS_NAME);
		retval = PTR_ERR(dev_class);
		goto failed_classreg;
	}

	// With a class, the easiest way to instantiate a device is to call device_create()
	dev = device_create(dev_class, NULL, MKDEV(dev_major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(dev))
	{
		TRACE_ERROR("Failed to create device '%s'", DEVICE_NAME);
		retval = PTR_ERR(dev);
		goto failed_devreg;
	}

   // Start poll timer
   setup_timer(&poll_timer, poll_timer_callback, 0);
   mod_timer(&poll_timer, jiffies + msecs_to_jiffies(VEHABUS_POLL_TIMER_INTERVAL));

	return 0;

failed_devreg:
	class_unregister(dev_class);
	class_destroy(dev_class);
failed_classreg:
	unregister_chrdev(dev_major, DEVICE_NAME);
failed_chrdevreg:
	return -1;
}

/** Cleanup - unregister the driver */
static void __exit module_driver_exit(void)
{
   unsigned long flags;

   spin_lock_irqsave(&dev_lock, flags);

   // Stop refresh tasklet
   del_timer(&poll_timer);

   spin_unlock_irqrestore(&dev_lock, flags);

   device_destroy(dev_class, MKDEV(dev_major, 0));
	class_unregister(dev_class);
	class_destroy(dev_class);
	unregister_chrdev(dev_major, DEVICE_NAME);

   TRACE("Driver cleanup");
}

module_init(module_driver_init);
module_exit(module_driver_exit);


//
// Modules info
//
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
