#define DEBUG
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/sysfs.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>

#define DEV_NAME					"clkout2"

#define CM_CLKOUT_CTRL				(0)

#define CLKOUT2EN					(1 << 7)
#define CLKOUT2DIV_MASK				(0x07 << 3)
#define CLKOUT2DIV_DIV4				(3 << 3)
#define CLKOUT2DIV_DIV5				(4 << 3)
#define CLKOUT2DIV_DIV6				(5 << 3)
#define CLKOUT2DIV_DIV7				(6 << 3)
#define CLKOUT2SOURCE_MASK			(0x7 << 0)
#define CLKOUT2SOURCE_SEL0			(0)
#define CLKOUT2SOURCE_SEL1			(0x1)
#define CLKOUT2SOURCE_SEL2			(0x2)
#define CLKOUT2SOURCE_SEL4			(0x3)
#define CLKOUT2SOURCE_SEL5			(0x4)

struct regs_t {
	phys_addr_t base;
	unsigned long size;
	void __iomem *vbase;
};

static struct regs_t CM_DEVICE_regs = {
	.base 	= 0x44E00700,
	.size 	= 0x10,
	.vbase 	= NULL,
};

static void pinmux_config (void)
{
	/* do it in dtbs */
}

static void clkout2_setup (void)
{
	void __iomem *vbase = CM_DEVICE_regs.vbase;
	u32 value;

	if (vbase == NULL) {
		vbase = ioremap_nocache (CM_DEVICE_regs.base, CM_DEVICE_regs.size);
		if (vbase == NULL) {
			printk (KERN_INFO "%s: Error: ioremap 0x%08X fail\n", DEV_NAME, CM_DEVICE_regs.base);
			return;
		}
		CM_DEVICE_regs.vbase = vbase;
	}

	value = CLKOUT2SOURCE_SEL4 | CLKOUT2DIV_DIV7 | CLKOUT2EN;

	writel (value, vbase + CM_CLKOUT_CTRL);

	if (CM_DEVICE_regs.vbase) {
		iounmap (CM_DEVICE_regs.vbase);
		CM_DEVICE_regs.vbase = NULL;
	}
}

static void clkout2_disable (void)
{
	void __iomem *vbase = CM_DEVICE_regs.vbase;

	if (vbase == NULL) {
		vbase = ioremap_nocache (CM_DEVICE_regs.base, CM_DEVICE_regs.size);
		if (vbase == NULL) {
			printk (KERN_INFO "%s: Error: ioremap 0x%08X fail\n", DEV_NAME, CM_DEVICE_regs.base);
			return;
		}
		CM_DEVICE_regs.vbase = vbase;
	}

	writel (0, vbase + CM_CLKOUT_CTRL);

	if (CM_DEVICE_regs.vbase) {
		iounmap (CM_DEVICE_regs.vbase);
		CM_DEVICE_regs.vbase = NULL;
	}
}

static ssize_t enable_show (struct device *dev, struct device_attribute *attr, char *buf)
{
	u32 value;
	void __iomem *vbase = CM_DEVICE_regs.vbase;

	if (vbase == NULL) {
		vbase = ioremap_nocache (CM_DEVICE_regs.base, CM_DEVICE_regs.size);
		if (vbase == NULL) {
			printk (KERN_INFO "%s: Error: ioremap 0x%08X fail\n", DEV_NAME, CM_DEVICE_regs.base);
			return -ENOMEM;
		}
		CM_DEVICE_regs.vbase = vbase;
	}

	value = readl (vbase + CM_CLKOUT_CTRL);

	if (CM_DEVICE_regs.vbase) {
		iounmap (CM_DEVICE_regs.vbase);
		CM_DEVICE_regs.vbase = NULL;
	}

	return sprintf (buf, "%d\n", (value & CLKOUT2EN ? 1 : 0));
}

static ssize_t enable_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	if (sscanf(buf, "%d", &value) != 1)
		return -EINVAL;

	if (value) {
		pinmux_config ();
		clkout2_setup ();
	}
	else {
		clkout2_disable ();
	}

	return size;
}

static DEVICE_ATTR (enable, S_IRUGO | S_IWUSR, enable_show, enable_store);

/*
 */
static const struct file_operations clkout2_fops = {
	.owner		= THIS_MODULE,
	.open 		= NULL,
	.write		= NULL,
	.read		= NULL,
	.release 	= NULL,
};

static struct miscdevice clkout2_miscdev = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = DEV_NAME,
	.fops   = &clkout2_fops,
};

static int __init clkout2_init (void)
{
	int ret;

	ret = misc_register (&clkout2_miscdev);
	if (ret) {
		printk (KERN_ERR "%s: cannot register miscdev on minor=%d (%d)\n", DEV_NAME, MISC_DYNAMIC_MINOR, ret);
		goto err;
	}

	ret = device_create_file (clkout2_miscdev.this_device, &dev_attr_enable);
	if (ret < 0) {
		printk (KERN_ERR "%s: cannot register sysfs on minor=%d [%d]\n", DEV_NAME, MISC_DYNAMIC_MINOR, ret);
		goto err1;
	}

	clkout2_setup ();

	printk (KERN_INFO "%s: driver loaded success\n", DEV_NAME);

	return 0;

err1:
	misc_deregister (&clkout2_miscdev);
err:
	return -EBUSY;
}

static void __exit clkout2_exit (void)
{
	clkout2_disable();

	device_remove_file (clkout2_miscdev.this_device, &dev_attr_enable);
	misc_deregister (&clkout2_miscdev);

	printk (KERN_INFO "%s: driver unload\n", DEV_NAME);
}

module_init (clkout2_init);
module_exit (clkout2_exit);

MODULE_AUTHOR("Jeffery Cheng");
MODULE_DESCRIPTION("CLKOUT2 Driver for AM335X");
MODULE_LICENSE("GPL");
