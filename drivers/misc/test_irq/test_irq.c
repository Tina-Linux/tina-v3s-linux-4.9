#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/sunxi-gpio.h>
#include <linux/etherdevice.h>
#include <linux/crypto.h>
#include <linux/pm_wakeirq.h>
#include "test_irq.h"

static struct sunxi_test_irq_platdata *test_irq;

static int test_irq_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct sunxi_test_irq_platdata *data;
	struct gpio_config config;
	u32 val;
	int ret = 0;
	int irq_num = 0;
	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	data->pdev = pdev;
	test_irq = data;

	data->test_used = -1;
	if (!of_property_read_u32(np, "test_used", &val)) {
		switch (val) {
		case 1:
			data->test_used = val;
			break;
		default:
			printk("test_used no use (%u)\n", val);
			return -EINVAL;
		}
	}
	printk("test_used (%u)\n", val);


	data->test_irq_gpio = of_get_named_gpio_flags(np, "test_irq_gpio",
			0, (enum of_gpio_flags *)&config);
	if (!gpio_is_valid(data->test_irq_gpio)) 
	{
		printk("get gpio test_irq_gpio failed\n");
	} 
	else 
	{
			printk("test_irq_gpio gpio=%d  mul-sel=%d  pull=%d  drv_level=%d  data=%d\n",
					config.gpio,
					config.mul_sel,
					config.pull,
					config.drv_level,
					config.data);

			ret = devm_gpio_request(dev, data->test_irq_gpio,"irq_gpio");
			if (ret < 0) 
			{
				printk("can't request irq_gpio %d\n",
					data->test_irq_gpio);
				return ret;
			}

			ret = gpio_direction_input(data->test_irq_gpio);
			if (ret < 0) 
			{
				printk("can't request input direction irq_gpio %d\n",
					data->test_irq_gpio);
				return ret;
			}

			ret = device_init_wakeup(dev, 1);
			if (ret < 0) 
			{
				printk("device init wakeup failed\n");
				return ret;
			}
			irq_num = gpio_to_irq(data->test_irq_gpio);
			printk(">>>>>>>>>>>> irq_num = %d\n",irq_num);
			ret = dev_pm_set_wake_irq(dev, irq_num);
			if (ret < 0) 
			{
				printk("can't enable wakeup src for irq_gpio %d\n",
					data->test_irq_gpio);
				return ret;
			}
	}

	printk("ready to do wake up irq\n");
	ret = enable_irq_wake(irq_num);
	printk("ready to do wake up irq-----\n");
	if (ret)
		printk(">>>>>>>>>>> wake up irq failed !!!!! \n");
	else
		printk(">>>>>>>>>>> wake up irq ok !!!!! \n");
	
	return 0;
}

static int test_irq_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id test_irq_ids[] = {
	{ .compatible = "allwinner,test-irq" },
	{ /* Sentinel */ }
};

static struct platform_driver test_irq_driver = {
	.probe		= test_irq_probe,
	.remove	= test_irq_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "test-irq",
		.of_match_table	= test_irq_ids,
	},
};

module_platform_driver(test_irq_driver);

MODULE_DESCRIPTION("test irq driver");
MODULE_LICENSE("GPL");
