#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/sched.h>
#include <linux/delay.h>

typedef unsigned char uchar;

#define DEV_NAME				"lcmDisplay"

#define DISP_TOTAL_LENGTH		32

static const struct of_device_id of_gpio_lcm_match[] = {
	{ .compatible = "lcm_wc1602", },
	{},
};
MODULE_DEVICE_TABLE(of, of_gpio_lcm_match);

struct gpio_lcm_priv {
	int vo;
	int rs;
	int rw;
	int en;
	int db0;
	int db1;
	int db2;
	int db3;
	int db4;
	int db5;
	int db6;
	int db7;
	unsigned char mem[DISP_TOTAL_LENGTH];
};

struct gpio_lcm_priv *lcm_priv = NULL;
static spinlock_t lock;

static uchar PATTERN[] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//00H CGRAM
	0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,//01H CGRAM
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//02H CGRAM
	0x0A,0x15,0x0A,0x15,0x0A,0x15,0x0A,0x15,//03H CGRAM
	0x0E,0x11,0x11,0x11,0x11,0x15,0x12,0x0D,//04H CGRAM
	0x1f,0x11,0x11,0x11,0x11,0x11,0x11,0x1f,//05H CGRAM
};

static void inline vo_set(int value)
{
	gpio_set_value(lcm_priv->vo, value);
}

static void inline rs_set(int value)
{
	gpio_set_value(lcm_priv->rs, value);
}

static void inline rw_set(int value)
{
	gpio_set_value(lcm_priv->rw, value);
}

static void inline en_set(int value)
{
	gpio_set_value(lcm_priv->en, value);
}

static void inline db0_set(int value)
{
	gpio_set_value(lcm_priv->db0, value);
}

static void inline db1_set(int value)
{
	gpio_set_value(lcm_priv->db1, value);
}

static void inline db2_set(int value)
{
	gpio_set_value(lcm_priv->db2, value);
}

static void inline db3_set(int value)
{
	gpio_set_value(lcm_priv->db3, value);
}

static void inline db4_set(int value)
{
	gpio_set_value(lcm_priv->db4, value);
}

static void inline db5_set(int value)
{
	gpio_set_value(lcm_priv->db5, value);
}

static void inline db6_set(int value)
{
	gpio_set_value(lcm_priv->db6, value);
}

static void inline db7_set(int value)
{
	gpio_set_value(lcm_priv->db7, value);
}

static int inline db0_read(void)
{
	return gpio_get_value(lcm_priv->db0);
}

static int inline db1_read(void)
{
	return gpio_get_value(lcm_priv->db1);
}

static int inline db2_read(void)
{
	return gpio_get_value(lcm_priv->db2);
}

static int inline db3_read(void)
{
	return gpio_get_value(lcm_priv->db3);
}

static int inline db4_read(void)
{
	return gpio_get_value(lcm_priv->db4);
}

static int inline db5_read(void)
{
	return gpio_get_value(lcm_priv->db5);
}

static int inline db6_read(void)
{
	return gpio_get_value(lcm_priv->db6);
}

static int inline db7_read(void)
{
	return gpio_get_value(lcm_priv->db7);
}



static void db_output(int value)
{
	db0_set(value & (1<<0));
	db1_set(value & (1<<1));
	db2_set(value & (1<<2));
	db3_set(value & (1<<3));
	db4_set(value & (1<<4));
	db5_set(value & (1<<5));
	db6_set(value & (1<<6));
	db7_set(value & (1<<7));
}

__attribute__ ((unused))
static int db_input(void)
{
	char d0 = db0_read();
	char d1 = db1_read();
	char d2 = db2_read();
	char d3 = db3_read();
	char d4 = db4_read();
	char d5 = db5_read();
	char d6 = db6_read();
	char d7 = db7_read();

	return (d0 | (d1<<1) | (d2<<2) | (d3<<3) | (d4<<4) | (d5<<5) | (d6<<6) | (d7<<7));
}

static void read_busy(void)
{
	/* nop */
}

static void write_i8(uchar cmd)
{
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);
	rs_set(0);
	rw_set(0);
	udelay(100);
	en_set(1);
	db_output(cmd);
	udelay(100);
	en_set(0);
	udelay(10);
	rs_set(1);
	rw_set(1);
	read_busy();
	spin_unlock_irqrestore(&lock, flags);
}

static void write_d8(uchar data)
{
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);
	rs_set(1);
	rw_set(0);
	udelay(100);
	en_set(1);
	db_output(data);
	udelay(100);
	en_set(0);
	udelay(10);
	rs_set(1);
	rw_set(1);
	read_busy();
	spin_unlock_irqrestore(&lock, flags);
}

static void save_font_8bit(void)
{
	int i;
	uchar *p = PATTERN;

	write_i8(0x40);

	for(i = 0; i < sizeof(PATTERN); i++) {
		write_d8(*p++);
	}
}

__attribute__ ((unused))
static void show_cgram(uchar c)
{
	int i;

	write_i8(0x80);
	for(i = 0; i < 16; i++) {
		write_d8(c);
	}

	write_i8(0xC0);
	for(i = 0; i < 16; i++) {
		write_d8(c);
	}
}

__attribute__ ((unused))
static void show_cgrom_twoLine(uchar c1, uchar c2)
{
	int i;

	write_i8(0x80);
	for(i = 0; i < 16; i++) {
		write_d8(c1++);
	}

	write_i8(0xC0);
	for(i = 0; i < 16; i++) {
		write_d8(c2++);
	}
}

static void lcm_show(int offset, uchar c)
{
	int off = 0;

	if (offset >= 0 && offset < 16)
		off = 0x80 + offset;
	else if (offset >= 16 && offset < 32)
		off = 0xC0 + offset - 16;
	else
		return;

	write_i8(off);
	write_d8(c);
}

static void lcm_show_mem(uchar *buf, int size)
{
	int i;

	if (size <= 16) {
		write_i8(0x80);
		for (i = 0; i < size; i++)
			write_d8(buf[i]);
	} else if (size <= 32) {
		write_i8(0x80);
		for (i = 0; i < 16; i++)
			write_d8(buf[i]);

		write_i8(0xC0);
		for (i = 0; i < size - 16; i++)
			write_d8(buf[16 + i]);
	}
}

static void lcm_init(void)
{
	write_i8(0x38);
	mdelay(5);
	write_i8(0x38);
	mdelay(1);
	write_i8(0x38);
	mdelay(1);
	write_i8(0x38);
	mdelay(1);

	write_i8(0x0C);
	write_i8(0x01);
	write_i8(0x06);
	msleep(5);

	save_font_8bit();
}

static struct gpio_lcm_priv *gpio_lcm_create(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct gpio_lcm_priv *priv;
	struct device_node *np;
	enum of_gpio_flags flags;

	priv = devm_kzalloc(dev, sizeof(struct gpio_lcm_priv), GFP_KERNEL);
	if (!priv)
		return ERR_PTR(-ENOMEM);

	np = dev->of_node;
	priv->vo = of_get_named_gpio_flags(np, "vo", 0, &flags);
	priv->rs = of_get_named_gpio_flags(np, "rs", 0, &flags);
	priv->rw = of_get_named_gpio_flags(np, "rw", 0, &flags);
	priv->en = of_get_named_gpio_flags(np, "en", 0, &flags);
	priv->db0 = of_get_named_gpio_flags(np, "db0", 0, &flags);
	priv->db1 = of_get_named_gpio_flags(np, "db1", 0, &flags);
	priv->db2 = of_get_named_gpio_flags(np, "db2", 0, &flags);
	priv->db3 = of_get_named_gpio_flags(np, "db3", 0, &flags);
	priv->db4 = of_get_named_gpio_flags(np, "db4", 0, &flags);
	priv->db5 = of_get_named_gpio_flags(np, "db5", 0, &flags);
	priv->db6 = of_get_named_gpio_flags(np, "db6", 0, &flags);
	priv->db7 = of_get_named_gpio_flags(np, "db7", 0, &flags);

	devm_gpio_request_one(dev, priv->vo, 0, "vo");
	devm_gpio_request_one(dev, priv->rs, 0, "rs");
	devm_gpio_request_one(dev, priv->rw, 0, "rw");
	devm_gpio_request_one(dev, priv->en, 0, "en");
	devm_gpio_request_one(dev, priv->db0, 0, "db0");
	devm_gpio_request_one(dev, priv->db1, 0, "db1");
	devm_gpio_request_one(dev, priv->db2, 0, "db2");
	devm_gpio_request_one(dev, priv->db3, 0, "db3");
	devm_gpio_request_one(dev, priv->db4, 0, "db4");
	devm_gpio_request_one(dev, priv->db5, 0, "db5");
	devm_gpio_request_one(dev, priv->db6, 0, "db6");
	devm_gpio_request_one(dev, priv->db7, 0, "db7");

	gpio_direction_output(priv->vo, 0);
	gpio_direction_output(priv->rs, 0);
	gpio_direction_output(priv->rw, 0);
	gpio_direction_output(priv->en, 0);
	gpio_direction_output(priv->db0, 0);
	gpio_direction_output(priv->db1, 0);
	gpio_direction_output(priv->db2, 0);
	gpio_direction_output(priv->db3, 0);
	gpio_direction_output(priv->db4, 0);
	gpio_direction_output(priv->db5, 0);
	gpio_direction_output(priv->db6, 0);
	gpio_direction_output(priv->db7, 0);

	return priv;
}

static int lcm_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int lcm_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations lcm_fops = {
	.owner      = THIS_MODULE,
	.open       = lcm_open,
	.release    = lcm_close,
};

static struct miscdevice lcm_miscdev = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = DEV_NAME,
	.fops 	= &lcm_fops,
};

static ssize_t fbmem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct gpio_lcm_priv *priv = lcm_priv;

	if (priv == NULL) {
		printk(KERN_ERR "Error: %s hasn't been initialized\n", DEV_NAME);
		return -ENODEV;
	}

	return sprintf (buf, "%s", priv->mem);
}

static ssize_t fbmem_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct gpio_lcm_priv *priv = lcm_priv;

	int len;
	int i;

	if (priv == NULL) {
		printk(KERN_ERR "Error: %s hasn't been initialized\n", DEV_NAME);
		return -ENODEV;
	}

	len = size > DISP_TOTAL_LENGTH ? DISP_TOTAL_LENGTH : size;
	for (i = 0; i < len; i++) {
		if (priv->mem[i] != buf[i]) {
			lcm_show(i, buf[i]);
			priv->mem[i] = buf[i];
		}
	}

	return size;
}

static DEVICE_ATTR(fbmem, S_IRUGO | S_IWUSR, fbmem_show, fbmem_store);

static ssize_t clear_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* do nothing */
	return 0;
}

static ssize_t clear_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	struct gpio_lcm_priv *priv = lcm_priv;

	if (sscanf(buf, "%d", &value) != 1)
		return -EINVAL;

	if (value) {
		memset(priv->mem, ' ', DISP_TOTAL_LENGTH);
		lcm_show_mem(priv->mem, sizeof(priv->mem));
	}

	return size;
}

static DEVICE_ATTR(clear, S_IRUGO | S_IWUSR, clear_show, clear_store);

static int gpio_lcm_probe(struct platform_device *pdev)
{
	int ret;
	struct gpio_lcm_priv *priv;

#ifdef CONFIG_OF
	priv = gpio_lcm_create(pdev);
	if (IS_ERR(priv))
		return PTR_ERR(priv);
#endif
	platform_set_drvdata(pdev, priv);

	spin_lock_init(&lock);

	lcm_priv = priv;

	ret = misc_register(&lcm_miscdev);
	if (ret) {
		dev_err(&pdev->dev, "device %s miscdev register fail: %d\n", DEV_NAME, ret);
		goto err;
	}

	ret = device_create_file(lcm_miscdev.this_device, &dev_attr_fbmem);
	if (ret < 0) {
		dev_err(&pdev->dev, "device %s sysfs register fail: %d\n", DEV_NAME, ret);
		goto err1;
	}

	device_create_file(lcm_miscdev.this_device, &dev_attr_clear);

	lcm_init();

#if 0	/* just for debug */
	show_cgrom_twoLine(0x10, 0x20);
	msleep (1000);
	show_cgrom_twoLine(0x30, 0x40);
	msleep (1000);
	show_cgrom_twoLine(0x50, 0x60);
	msleep (1000);
	show_cgrom_twoLine(0x70, 0x80);
	msleep (1000);
	show_cgrom_twoLine(0x90, 0xa0);
	msleep (1000);

	{
		int i;
		for (i = 0; i < 16; i++) {
			write_i8(0x80 + i);
			write_d8('A' + i);
			msleep(1000);
		}
	}
#endif
	sprintf(priv->mem, "Beaglebone Black Display Module");
	lcm_show_mem(priv->mem, sizeof(priv->mem));

	return 0;
err1:
	misc_deregister(&lcm_miscdev);
err:
	return -ENODEV;
}

static int gpio_lcm_remove(struct platform_device *pdev)
{
	device_remove_file(lcm_miscdev.this_device, &dev_attr_clear);

	device_remove_file(lcm_miscdev.this_device, &dev_attr_fbmem);

	misc_deregister(&lcm_miscdev);

	return 0;
}

static struct platform_driver gpio_lcm_driver = {
	.probe		= gpio_lcm_probe,
	.remove		= gpio_lcm_remove,
	.driver		= {
		.name	= "lcm_wc1602",
		.of_match_table = of_gpio_lcm_match,
	},
};

module_platform_driver(gpio_lcm_driver);

MODULE_AUTHOR("Jeffery Cheng <>");
MODULE_DESCRIPTION("GPIO LCM WC1602 driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:lcm-wc1602");
