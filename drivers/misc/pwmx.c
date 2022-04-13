#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>
//#include <mach/hardware.h>
#include <linux/gpio.h>
//#include <mach/sys_config.h>
//#include <asm/system.h>
#include <linux/slab.h>

#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/pwm.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#define MAX           100
#define LOW_BRIGHT    0
#define HIGHT_BRIGHT  1
#define MIDIUM_BRIGHT  2 
#define ON_OFF        3
#define ADJEST_BRGHT  4
#define ENABLE    0
#define DISABLE   1

struct pwm_info{
        s32                     dev;
        u32                     channel;
        u32                     polarity;
        u32                     period_ns;
        u32                     duty_ns;
        u32                     enable;
    };

struct pwm_info *buzzer_pwm_info;
struct timer_list timer;
struct timer_list switch_timer;
static int   time_delay  =  0;
static int   switch_delay  = 0;
#define NUMBER_OF_ARRAY(array) (sizeof(array) / sizeof(array[0]))

static int buzzer_sys_pwm_request(u32 pwm_id)
{
    int ret = 0;
    struct pwm_device *pwm_dev;

    pwm_dev = pwm_request(pwm_id, "buzzer");

    if(NULL == pwm_dev || IS_ERR(pwm_dev)) {
        printk("buzzer_sys_pwm_request pwm %d fail!\n", pwm_id);
    } else {
        printk("buzzer_sys_pwm_request pwm %d success!\n", pwm_id);
    }
    ret = (int)pwm_dev;
    return ret;
}

static int buzzer_sys_pwm_config(int p_handler, int duty_ns, int period_ns)
{
    int ret = 0;
    struct pwm_device *pwm_dev;

    pwm_dev = (struct pwm_device *)p_handler;
    if(NULL == pwm_dev || IS_ERR(pwm_dev)) {
        printk("buzzer_sys_pwm_Config, handle is NULL!\n");
        ret = -1;
    } else {
        ret = pwm_config(pwm_dev, duty_ns, period_ns);
        printk("buzzer_sys_pwm_Config pwm %d, <%d / %d> \n", pwm_dev->pwm, duty_ns, period_ns);
    }
    return ret;
}
int buzzer_sys_pwm_set_polarity(int p_handler, int polarity)
{
    int ret = 0;
    struct pwm_device *pwm_dev;

    pwm_dev = (struct pwm_device *)p_handler;
    if(NULL == pwm_dev || IS_ERR(pwm_dev)) {
        printk("buzzer_sys_pwm_Set_Polarity, handle is NULL!\n");
        ret = -1;
    } else {
        ret = pwm_set_polarity(pwm_dev, polarity);
        printk("buzzer_sys_pwm_Set_Polarity pwm %d, active %s\n", pwm_dev->pwm, (polarity==0)? "high":"low");
    }
    return ret;
}

void set_pwm(int freq)
{
    unsigned duty_ns, period_ns ;
    //freq *= 1000;
    period_ns =  1000000000 / freq ;
    duty_ns =period_ns/2;
    printk("set_pwm  <duty_ns=%d  period_ns=%d> \n", duty_ns, period_ns);
    if (freq == 0) {
        pwm_config(buzzer_pwm_info->dev, 0, period_ns);
        pwm_disable(buzzer_pwm_info->dev);
    } else {
        pwm_config(buzzer_pwm_info->dev, duty_ns, period_ns);
        pwm_enable(buzzer_pwm_info->dev);
    }
    return ;
}


static ssize_t pwm_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t size)
{
    unsigned int data = 0;
    char *after;
    data = simple_strtoul(buf, &after, 10);
    set_pwm(data);

    return size;
}

static ssize_t pwm_show(struct device *dev,struct device_attribute *attr, char *buf)
{
    return 0;
}

static DEVICE_ATTR(pwmx, S_IRUGO|S_IWUSR|S_IWGRP,
        pwm_show, pwm_store);        
static struct attribute *pwm_ctl_attributes[] = {
    &dev_attr_pwmx.attr,
    NULL
};

static struct attribute_group pwm_attribute_group = {
    .name = "pwmx_dev",
    .attrs =pwm_ctl_attributes,
};
static struct miscdevice misc_pwm={
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mis_pwm",
};

static int __init pwm_init(void)
{
    int err;
    printk("zhb+++++++pwm_init\n");
    buzzer_pwm_info = (struct pwm_info *)kmalloc(sizeof(struct pwm_info),GFP_KERNEL | __GFP_ZERO);
    buzzer_pwm_info->dev = buzzer_sys_pwm_request(0);
    buzzer_sys_pwm_set_polarity(buzzer_pwm_info->dev,0);
    err = misc_register(&misc_pwm);
    if(err < 0)
    {
        pr_err("%s pwm mis register driver as misc device error\n", __FUNCTION__);
        goto exit;
    }

    err=sysfs_create_group(&misc_pwm.this_device->kobj,&pwm_attribute_group);
    if(err < 0){
        pr_err("%s sysfs_create_group error\n",__FUNCTION__);
        goto exit;
    }
    set_pwm(10000);//10khz
    exit:
        return err;
}

static void __exit pwm_exit(void)
{
    sysfs_remove_group(&misc_pwm.this_device->kobj,&pwm_attribute_group);
    misc_deregister(&misc_pwm);
}

module_init(pwm_init);
module_exit(pwm_exit);
MODULE_LICENSE("GPL");

