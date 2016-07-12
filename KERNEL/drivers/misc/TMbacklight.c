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
#include <linux/of.h>

#include <asm/io.h>

#define DEV_NAME		"TMbacklight"

#define CM_PER_L4LS_CLKSTCTRL   (0x0)
#define CM_PER_L3S_CLKSTCTRL   (0x4)
#define CM_PER_L4FW_CLKSTCTRL   (0x8)
#define CM_PER_L3_CLKSTCTRL   (0xc)
#define CM_PER_OCPWP_L3_CLKSTCTRL   (0x12c)

#define CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK   (0x00000008u)
#define CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK   (0x00000010u)
#define CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L4_GCLK   (0x00000020u)
#define CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_L4LS_GCLK   (0x00000100u)
#define CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_TIMER4_GCLK   (0x00010000u)
#define CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK   (0x00000010u)

#define CM_DPLL_CLKSEL_TIMER7_CLK   (0x4)
#define CM_DPLL_CLKSEL_TIMER2_CLK   (0x8)
#define CM_DPLL_CLKSEL_TIMER3_CLK   (0xc)
#define CM_DPLL_CLKSEL_TIMER4_CLK   (0x10)
#define CM_DPLL_CLKSEL_TIMER5_CLK   (0x18)
#define CM_DPLL_CLKSEL_TIMER6_CLK   (0x1c)

#define CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL   (0x00000003u)
#define CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL_SHIFT   (0x00000000u)
#define CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL_SEL1   (0x0u)
#define CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL_SEL2   (0x1u)
#define CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL_SEL3   (0x2u)
#define CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL_SEL4   (0x3u)

#define CM_PER_TIMER5_CLKCTRL   (0xec)

#define CM_PER_TIMER5_CLKCTRL_MODULEMODE   (0x00000003u)
#define CM_PER_TIMER5_CLKCTRL_MODULEMODE_ENABLE   (0x2u)

#define CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE   (0x2u)
#define CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP   (0x2u)
#define CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP   (0x2u)
#define CM_PER_L4LS_CLKCTRL   (0x60)
#define CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE   (0x2u)
#define CM_PER_L3_INSTR_CLKCTRL   (0xdc)
#define CM_PER_L4LS_CLKCTRL_MODULEMODE   (0x00000003u)
#define CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL   (0x00000003u)
#define CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL   (0x00000003u)
#define CM_PER_L3_CLKCTRL_MODULEMODE   (0x00000003u)
#define CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE   (0x2u)
#define CM_PER_L3_CLKCTRL   (0xe0)
#define CM_PER_L3S_CLKSTCTRL_CLKTRCTRL   (0x00000003u)
#define CM_PER_L3_CLKSTCTRL_CLKTRCTRL   (0x00000003u)
#define CM_PER_L3_INSTR_CLKCTRL_MODULEMODE   (0x00000003u)
#define CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP   (0x2u)
#define CM_PER_L3_CLKSTCTRL   (0xc)
#define CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP   (0x2u)

#define DMTIMER_WRITE_POST_TCLR				0x00000001u
#define DMTIMER_TWPS   (0x48)
#define DMTIMER_TSICR   (0x54)
#define DMTIMER_TSICR_POSTED   (0x00000004u)
#define DMTIMER_TCLR_TRG   (0x00000C00u)
#define DMTIMER_TCLR_SCPWM   (0x00000080u)
#define DMTIMER_TCLR_TRG_SHIFT   (0x0000000Au)
#define DMTIMER_TCLR_TRG_OVERFLOWANDMATCH   (0x2u)
#define DMTIMER_TCLR_PT_TOGGLE   (0x1u)
#define DMTIMER_TCLR_PT_SHIFT   (0x0000000Cu)

#define DMTIMER_TCLR   (0x38)
#define DMTIMER_TCRR   (0x3C)
#define DMTIMER_TLDR   (0x40)
#define DMTIMER_TMAR   (0x4C)
#define DMTIMER_TCLR_AR   (0x00000002u)
#define DMTIMER_TCLR_CE   (0x00000040u)
#define DMTIMER_TCLR_ST   (0x00000001u)


#define TIMER_OVERFLOW_MASK				(0xFFFFFFFFUL)
#define TIMER_INITIAL_COUNT             (TIMER_OVERFLOW_MASK - 0x5DC0 + 1)
#define TIMER_RLD_COUNT                 (TIMER_INITIAL_COUNT)
#define TIMER_DUTY_COUNT                (((TIMER_OVERFLOW_MASK - TIMER_RLD_COUNT)>>1) + TIMER_INITIAL_COUNT)

#define CONTROL_CONF_GPMC_BEN0_CLE         (0x89C)

#define DMTimerWritePostedStatusGet(baseAdd)	readl(baseAdd + DMTIMER_TWPS)
#define DMTimerWaitForWrite(reg, baseAdd)   \
            if(readl(baseAdd + DMTIMER_TSICR) & DMTIMER_TSICR_POSTED)\
            while((reg & DMTimerWritePostedStatusGet(baseAdd)));

enum channel_t {
	AM335X_TIMER4 = 4,
	AM335X_TIMER5,
	AM335X_TIMER6,
	AM335X_TIMER7,
	AM335X_TIMER_INVALID,
};

static struct timer_dev {
	enum channel_t channel;
	u32 frequency;
	int brightness;
} timer_dev;

struct regs_t {
	phys_addr_t base;
	unsigned long size;
	void __iomem *vbase;
};

static struct regs_t SOC_CM_DPLL_REGS = {
	.base	= 0x44E00500,
	.size 	= 0x100,
	.vbase	= 0,
};
static struct regs_t SOC_CM_PER_REGS = {
	.base	= 0x44E00000,
	.size 	= 0x150,
	.vbase	= 0,
};
static struct regs_t SOC_DMTIMER_5_REGS = {
	.base	= 0x48046000,
	.size 	= 0x5C,
	.vbase	= 0,
};
static struct regs_t SOC_CONTROL_REGS = {
	.base	= 0x44E10000,
	.size 	= 0xF00,
	.vbase	= 0,
};

static int regs_map (struct regs_t *regs)
{
	if (regs->vbase) {
		printk (KERN_WARNING "%s: regs 0x%08X already mapped?\n", DEV_NAME, regs->base);
	}
	regs->vbase = ioremap_nocache (regs->base, regs->size);
	if (regs->vbase == NULL) {
		printk (KERN_ERR "%s: Error: regs 0x%08X mapped fail\n", DEV_NAME, regs->base);
		return -EBUSY;
	}

	return 0;
}

static void __exit regs_unmap (struct regs_t *regs)
{
	if (regs->vbase) {
		iounmap (regs->vbase);
		regs->vbase = NULL;
	}
}

static void __init pinmux_config (void)
{
	/*
	 * do it in kernel dts
	 */
	writel (0x00000012, SOC_CONTROL_REGS.vbase + CONTROL_CONF_GPMC_BEN0_CLE);
}

static void timer5_moduleclk_config (void)
{
	u32 val;
	//[1]
	writel (CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP, SOC_CM_PER_REGS.vbase + CM_PER_L3S_CLKSTCTRL);

	while ((readl(SOC_CM_PER_REGS.vbase + CM_PER_L3S_CLKSTCTRL) &
	        CM_PER_L3S_CLKSTCTRL_CLKTRCTRL) != CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
	//[2]
	writel (CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP, SOC_CM_PER_REGS.vbase + CM_PER_L3_CLKSTCTRL);

	while ((readl(SOC_CM_PER_REGS.vbase + CM_PER_L3_CLKSTCTRL) &
	        CM_PER_L3_CLKSTCTRL_CLKTRCTRL) != CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
	//[3]
	writel (CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE, SOC_CM_PER_REGS.vbase + CM_PER_L3_INSTR_CLKCTRL);

	while ((readl(SOC_CM_PER_REGS.vbase + CM_PER_L3_INSTR_CLKCTRL) &
	        CM_PER_L3_INSTR_CLKCTRL_MODULEMODE) != CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE);
	//[4]
	writel (CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE, SOC_CM_PER_REGS.vbase + CM_PER_L3_CLKCTRL);

	while ((readl(SOC_CM_PER_REGS.vbase + CM_PER_L3_CLKCTRL) &
	        CM_PER_L3_CLKCTRL_MODULEMODE) != CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE);
	//[5]
	writel (CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP, SOC_CM_PER_REGS.vbase + CM_PER_OCPWP_L3_CLKSTCTRL);

	while ((readl(SOC_CM_PER_REGS.vbase + CM_PER_OCPWP_L3_CLKSTCTRL) &
	        CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL) != CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
	//[6]
	writel (CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP, SOC_CM_PER_REGS.vbase + CM_PER_L4LS_CLKSTCTRL);

	while ((readl(SOC_CM_PER_REGS.vbase + CM_PER_L4LS_CLKSTCTRL) &
	        CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL) != CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
	//[7]
	writel (CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE, SOC_CM_PER_REGS.vbase + CM_PER_L4LS_CLKCTRL);

	while ((readl(SOC_CM_PER_REGS.vbase + CM_PER_L4LS_CLKCTRL) &
	        CM_PER_L4LS_CLKCTRL_MODULEMODE) != CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE);

	/*------------------------*/
	val = readl (SOC_CM_DPLL_REGS.vbase + CM_DPLL_CLKSEL_TIMER5_CLK);
	val &= ~(CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL);
	writel (val, SOC_CM_DPLL_REGS.vbase + CM_DPLL_CLKSEL_TIMER5_CLK);

	val = readl (SOC_CM_DPLL_REGS.vbase + CM_DPLL_CLKSEL_TIMER5_CLK);
	val |= (CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL_SEL2);
	writel (val, SOC_CM_DPLL_REGS.vbase + CM_DPLL_CLKSEL_TIMER5_CLK);

	while ((readl (SOC_CM_DPLL_REGS.vbase + CM_DPLL_CLKSEL_TIMER5_CLK) &
	        CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL) != CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL_SEL2);

	val = readl (SOC_CM_PER_REGS.vbase + CM_PER_TIMER5_CLKCTRL);
	val |= CM_PER_TIMER5_CLKCTRL_MODULEMODE_ENABLE;
	writel (val, SOC_CM_PER_REGS.vbase + CM_PER_TIMER5_CLKCTRL);

	while ((readl (SOC_CM_PER_REGS.vbase + CM_PER_TIMER5_CLKCTRL) &
	        CM_PER_TIMER5_CLKCTRL_MODULEMODE) != CM_PER_TIMER5_CLKCTRL_MODULEMODE_ENABLE);

	while (!(readl (SOC_CM_PER_REGS.vbase + CM_PER_L3S_CLKSTCTRL) & CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK));
	while (!(readl (SOC_CM_PER_REGS.vbase + CM_PER_L3_CLKSTCTRL) & CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK));
	while (!(readl (SOC_CM_PER_REGS.vbase + CM_PER_OCPWP_L3_CLKSTCTRL) &
	         (CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK | CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L4_GCLK)));
	while (!(readl (SOC_CM_PER_REGS.vbase + CM_PER_L4LS_CLKSTCTRL) &
	         (CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_L4LS_GCLK | CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_TIMER4_GCLK)));
}

static void set_timer_counter (void __iomem *base, u32 counter)
{
	writel (counter, base + DMTIMER_TCRR);
}

static void set_timer_reload (void __iomem *base, u32 reload)
{
	writel (reload, base + DMTIMER_TLDR);
}

static void set_timer_compare (void __iomem *base, u32 cmpval)
{
	writel (cmpval, base + DMTIMER_TMAR);
}

static void timer_mode_config (void __iomem *base, u32 mode)
{
	u32 val;

	DMTimerWaitForWrite (DMTIMER_WRITE_POST_TCLR, base);

	val = readl (base + DMTIMER_TCLR);
	val &= ~(DMTIMER_TCLR_AR | DMTIMER_TCLR_CE | DMTIMER_TCLR_TRG);
	writel (val, base + DMTIMER_TCLR);

	DMTimerWaitForWrite (DMTIMER_WRITE_POST_TCLR, base);

	val = readl (base + DMTIMER_TCLR);
	val |= mode;
	writel (val, base + DMTIMER_TCLR);

	DMTimerWaitForWrite (DMTIMER_WRITE_POST_TCLR, base);
}

static void timer_setup (struct timer_dev *dev)
{
	u32 val;
	void __iomem *base = NULL;
	int duty = dev->brightness;
	u32 freq = dev->frequency;
	u32 CLK_M_OSC = 24000000UL;
	u32 reload_count;
	u32 TLDR;

	switch (dev->channel) {
	case AM335X_TIMER4:
		break;
	case AM335X_TIMER5:
		base = SOC_DMTIMER_5_REGS.vbase;
		break;
	default:
		break;
	}

	if (base == NULL) {
		printk (KERN_ERR "%s: Error: TIMER%d regs cannot access\n", DEV_NAME, dev->channel);
		return;
	}

	if (duty < 0)
		duty = 0;
	else if (duty >= 99)
		duty = 99;

	TLDR = CLK_M_OSC / freq;
	reload_count = TIMER_OVERFLOW_MASK - TLDR + 1;
	val = (TIMER_OVERFLOW_MASK - reload_count + 1) / 100;
	if (val == 0) {
		printk (KERN_ERR "%s: Error: frequency out of range\n", DEV_NAME);
		return;
	}
	val = reload_count + val * duty;

	set_timer_counter (base, reload_count);
	set_timer_reload (base, reload_count);
	set_timer_compare (base, val);

	val = DMTIMER_TCLR_AR | DMTIMER_TCLR_CE | DMTIMER_TCLR_SCPWM |
	      (DMTIMER_TCLR_TRG_OVERFLOWANDMATCH << DMTIMER_TCLR_TRG_SHIFT) |
	      (DMTIMER_TCLR_PT_TOGGLE << DMTIMER_TCLR_PT_SHIFT);
	timer_mode_config (base, val);
}

static void timer_enable (struct timer_dev *dev)
{
	u32 val;
	void __iomem *base = NULL;

	switch (dev->channel) {
	case AM335X_TIMER4:

		break;
	case AM335X_TIMER5:
		base = SOC_DMTIMER_5_REGS.vbase;
		break;
	default:
		break;
	}

	if (base == NULL) {
		printk (KERN_ERR "%s: Error: TIMER%d regs cannot access\n", DEV_NAME, dev->channel);
		return;
	}

	val = readl (base + DMTIMER_TCLR);
	val |= DMTIMER_TCLR_ST;
	writel (val, base + DMTIMER_TCLR);
}

static void timer_disable (struct timer_dev *dev)
{
	u32 val;
	void __iomem *base = NULL;

	switch (dev->channel) {
	case AM335X_TIMER4:

		break;
	case AM335X_TIMER5:
		base = SOC_DMTIMER_5_REGS.vbase;
		break;
	default:
		break;
	}

	if (base == NULL) {
		printk (KERN_ERR "%s: Error: TIMER%d regs cannot access\n", DEV_NAME, dev->channel);
		return;
	}

	val = readl (base + DMTIMER_TCLR);
	val &= ~DMTIMER_TCLR_ST;
	writel (val, base + DMTIMER_TCLR);
}

static int timer_open (struct inode *inode, struct file *file)
{
	return 0;
}

static int timer_close (struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t brightness_show (struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf (buf, "%d\n", timer_dev.brightness);
}

static ssize_t brightness_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	if (sscanf(buf, "%d", &value) != 1)
		return -EINVAL;

	timer_dev.brightness = value;

	timer_setup (&timer_dev);

	return size;
}

static DEVICE_ATTR (brightness, S_IRUGO | S_IWUSR, brightness_show, brightness_store);

static ssize_t frequency_show (struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf (buf, "%d\n", timer_dev.frequency);
}

static ssize_t frequency_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	u32  value;

	if (sscanf(buf, "%d", &value) != 1)
		return -EINVAL;

	timer_dev.frequency = value;

	timer_setup (&timer_dev);

	return size;
}

static DEVICE_ATTR (frequency, S_IRUGO | S_IWUSR, frequency_show, frequency_store);

/*
 */
static const struct file_operations timer_fops = {
	.owner		= THIS_MODULE,
	.open 		= timer_open,
	.write		= NULL,
	.read		= NULL,
	.release 	= timer_close,
};

static struct miscdevice timer_miscdev = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = DEV_NAME,
	.fops   = &timer_fops,
};

static int TMbacklight_probe(struct platform_device *pdev)
{
	int ret;
#ifdef CONFIG_OF
	struct device *dev = &pdev->dev;
	struct device_node *np;
	u32 value;
#endif

	timer_dev.channel = AM335X_TIMER5;
	timer_dev.frequency = 1000;
	timer_dev.brightness = 50;

#ifdef CONFIG_OF
	np = dev->of_node;

	ret = of_property_read_u32(np, "bl,default_frequency", &value);
	if (!ret)
		timer_dev.frequency = value;

	ret = of_property_read_u32(np, "bl,default_duty", &value);
		timer_dev.brightness = value;
#endif

	if (regs_map (&SOC_CM_DPLL_REGS))
		goto err;

	if (regs_map (&SOC_CM_PER_REGS))
		goto err;

	if (regs_map (&SOC_DMTIMER_5_REGS))
		goto err;

	if (regs_map (&SOC_CONTROL_REGS))
		goto err;

	ret = misc_register (&timer_miscdev);
	if (ret) {
		dev_err(&pdev->dev, "cannot register miscdev on minor=%d (%d)\n", MISC_DYNAMIC_MINOR, ret);
		goto err;
	}

	ret = device_create_file (timer_miscdev.this_device, &dev_attr_brightness);
	if (ret < 0) {
		dev_err(&pdev->dev, "cannot register sysfs on minor=%d [%d]\n", MISC_DYNAMIC_MINOR, ret);
		goto err1;
	}

	ret = device_create_file (timer_miscdev.this_device, &dev_attr_frequency);
	if (ret < 0) {
		dev_err(&pdev->dev, "cannot register sysfs on minor=%d [%d]\n", MISC_DYNAMIC_MINOR, ret);
		goto err1;
	}

	pinmux_config();

	if (timer_dev.channel == AM335X_TIMER5)
		timer5_moduleclk_config();

	timer_setup (&timer_dev);

	timer_enable (&timer_dev);

	dev_info(&pdev->dev, "driver loaded success\n");

	return 0;

err1:
	misc_deregister (&timer_miscdev);
err:
	regs_unmap (&SOC_CM_DPLL_REGS);
	regs_unmap (&SOC_CM_PER_REGS);
	regs_unmap (&SOC_DMTIMER_5_REGS);
	regs_unmap (&SOC_CONTROL_REGS);
	return -EBUSY;
}

static int TMbacklight_remove(struct platform_device *pdev)
{
	timer_disable (&timer_dev);

	regs_unmap (&SOC_CM_DPLL_REGS);
	regs_unmap (&SOC_CM_PER_REGS);
	regs_unmap (&SOC_DMTIMER_5_REGS);
	regs_unmap (&SOC_CONTROL_REGS);

	device_remove_file (timer_miscdev.this_device, &dev_attr_brightness);
	device_remove_file (timer_miscdev.this_device, &dev_attr_frequency);
	misc_deregister (&timer_miscdev);

	return 0;
}

static const struct of_device_id of_TMbacklight_match[] = {
	{ .compatible = "TMbacklight", },
	{},
};
MODULE_DEVICE_TABLE(of, of_TMbacklight_match);

static struct platform_driver TMbacklight_driver = {
    .probe      = TMbacklight_probe,
    .remove     = TMbacklight_remove,
    .driver     = {
        .name   = "TMbacklight",
        .of_match_table = of_TMbacklight_match,
    },
};

module_platform_driver(TMbacklight_driver);

MODULE_AUTHOR("Jeffery Cheng");
MODULE_DESCRIPTION("Backlight Timer Driver for AM335x");
MODULE_LICENSE("GPL");
