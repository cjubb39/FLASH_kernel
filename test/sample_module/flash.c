/*
 * Device driver for FLASH:
 * Fast Linux Advanced Scheduling Hardware
 *
 * A Platform device implemented using the misc subsystem
 *
 * Chae Jubb
 * Adapted from code by Stephen A. Edwards
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "flash.h"

#include <linux/delay.h>

#define DRIVER_NAME "flash"

/*
 * Information about our device
 */
struct flash_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
} dev;

static void change_write_to_flash(flash_arg_t vla)
{
	u64 message = 0;

	message |= ((u64) vla.type  << 0);
	message |= ((u64) vla.pid   << 8);
	message |= ((u64) vla.pri   << 24);
	message |= ((u64) vla.state << 32);

	iowrite32((u32) message,         dev.virtbase + CHANGE_REQ);
	iowrite32((u32) (message >> 32), dev.virtbase + CHANGE_REQ);
}

static void sched_write_to_flash(flash_arg_t vla)
{
	u32 message = 0;
	iowrite32(message, dev.virtbase + SCHED_REQ);
}

/*
 * Handle ioctl() calls from userspace
 */
static long flash_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	flash_arg_t vla;

	switch (cmd) {
	case FLASH_WRITE:
		if (copy_from_user(&vla, (flash_arg_t *) arg,
				   sizeof(flash_arg_t)))
			return -EACCES;
		change_write_to_flash(vla);
		break;
	case FLASH_SCHED:
		/* just need to notify device that we want a process */
		sched_write_to_flash(vla);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/* The operations our device knows how to do */
static const struct file_operations flash_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = flash_ioctl,
};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice flash_misc_device = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRIVER_NAME,
	.fops		= &flash_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init flash_probe(struct platform_device *pdev)
{
	int ret;

	/* Register ourselves as a misc device: creates /dev/flash */
	ret = misc_register(&flash_misc_device);

	/* Get the address of our registers from the device tree */
	ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
	if (ret) {
		ret = -ENOENT;
		goto out_deregister;
	}

	/* Make sure we can use these registers */
	if (request_mem_region(dev.res.start, resource_size(&dev.res),
			       DRIVER_NAME) == NULL) {
		ret = -EBUSY;
		goto out_deregister;
	}

	/* Arrange access to our registers */
	dev.virtbase = of_iomap(pdev->dev.of_node, 0);
	if (dev.virtbase == NULL) {
		ret = -ENOMEM;
		goto out_release_mem_region;
	}

	return 0;

out_release_mem_region:
	release_mem_region(dev.res.start, resource_size(&dev.res));
out_deregister:
	misc_deregister(&flash_misc_device);
	return ret;
}

/* Clean-up code: release resources */
static int flash_remove(struct platform_device *pdev)
{
	iounmap(dev.virtbase);
	release_mem_region(dev.res.start, resource_size(&dev.res));
	misc_deregister(&flash_misc_device);
	return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id flash_of_match[] = {
	{ .compatible = "altr,flash" },
	{},
};
MODULE_DEVICE_TABLE(of, flash_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver flash_driver = {
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(flash_of_match),
	},
	.remove	= __exit_p(flash_remove),
};

/* Called when the module is loaded: set things up */
static int __init flash_init(void)
{
	pr_info(DRIVER_NAME ": init\n");
	return platform_driver_probe(&flash_driver, flash_probe);
}

/* Called when the module is unloaded: release resources */
static void __exit flash_exit(void)
{
	platform_driver_unregister(&flash_driver);
	pr_info(DRIVER_NAME ": exit\n");
}

module_init(flash_init);
module_exit(flash_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chae Jubb");
MODULE_DESCRIPTION("FLASH: Fast Linux Advanced Scheduling Hardware");
