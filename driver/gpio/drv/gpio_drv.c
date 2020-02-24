#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/pagemap.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/of.h>
#include <linux/uaccess.h>

#include <linux/irq.h>
#include <linux/interrupt.h>

#include "gpio_drv.h"

static int fbga_drv_open(struct inode * inode, struct file *filp)
{
    printk("device is open!\n");
    return 0;
}

static int fbga_drv_release(struct inode *inode, struct file *filp)
{
    printk("device is release!\n");
    return 0;
}

static ssize_t fbga_drv_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    unsigned int val;
    int ret;

    if(p >= GPIO_SIZE)
	return count? -EINVAL:0;
    if(count > GPIO_SIZE - p)
	count = GPIO_SIZE - p;
    
    val = ioread32(fbga_drv_base);
    if(copy_to_user(buf, &val, count)) 
    {
	return -EINVAL;
    }
    else
    {
	ret = count;
	printk(KERN_INFO "read %d bytes from %d\n", count, p);
    }

    return ret;
}

static ssize_t fbga_drv_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    //unsigned long p = *ppos;
    unsigned int count = size;
    int val, ret;

    ret = copy_from_user(&val, buf, count);//copy data from APP buffer to val

    iowrite32(val, fbga_drv_base);

    printk("fbga_drv: write 0x%x to 0x%x. \n", val, (unsigned int)fbga_drv_base);
    return 0;
}

static loff_t fbga_drv_llseek(struct file *filp, loff_t offset, int orig)
{
    loff_t ret=0;
    switch(orig)
    {
	case SEEK_SET:
	    if(offset < 0)
	    {
		ret = -EINVAL;
		break;
	    }
	    if((unsigned int) offset > GPIO_SIZE)
	    {
		ret = -EINVAL;
		break;
	    }
	    filp->f_op = offset;
	    ret = (loff_t)(filp->f_op);
	    break;
	case SEEK_CUR:
	    if((filp->f_op + offset)>GPIO_SIZE)
	    {
		ret = -EINVAL;
		break;
	    }
	    if((filp->f_op+offset)<0)
	    {
		ret = -EINVAL;
		break;
	    }
	    filp->f_op += offset;
	    ret = (loff_t)(filp->f_op);
	    break;
	default:
	    ret = -EINVAL;
	    break;
    }
    return ret;
}

static const struct file_operations fbga_drv_fops =
{
    .owner = THIS_MODULE,
    .llseek = fbga_drv_llseek,
    .read = fbga_drv_read,
    .write = fbga_drv_write,
    .open = fbga_drv_open,
    .release = fbga_drv_release,
};

static struct miscdevice fbga_drv_dev=
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "fbga_drv_dev",
    .fops = &fbga_drv_fops,
};


static int fbga_drv_init(void)
{
    int ret;

    gpio_drv_base = ioremap(GPIO_BASS_ADD, GPIO_SIZE); //change phy add to vir add
    

    printk("GPIO: Access address to device is:0x%x\n", (unsigned int)gpio_drv_base);
    ret = misc_register(&fbga_drv_dev);
    if(ret)
    {
	printk("fbga_drv:[ERROR] Misc device register faifbga_drv.\n");
	return ret;
    }

    printk("Module init complete!\n");
    return 0;
}

static void fbga_drv_exit(void)
{
    printk("Module exit!\n");
    iounmap(gpio_drv_base);
    misc_deregister(&fbga_drv_dev);
}

module_init(fbga_drv_init);
module_exit(fbga_drv_exit);

MODULE_AUTHOR("CHENGL");
MODULE_DESCRIPTION("FBGA_DRV");
MODULE_LICENSE("Dual BSD/GPL");
