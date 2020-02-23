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


#define MAX_BUF 64
#define GPIO_SIZE 1024*1024
#define GPIO_BASS_ADD 0x41200000

static void __iomem *gpio_base; //gpio base address

int gpio_open(struct inode * inode, struct file *filp)
{
    printk("device is open!\n");
    return 0;
}

int gpio_release(struct inode *inode, struct file *filp)
{
    printk("device is release!\n");
    return 0;
}

static int gpio_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = **ppos;
    unsigned int count = size;
    struct gpio_priv_data *devp = filp->private_data;

    if(p >= GPIO_SIZE)
	return count? -EINVAL:0;
    if(count > GPIO_SIZE - p)
	count = GPIO_SIZE - p;
    if(copy_to_user(buf, (void*)(devp->mem+p), count))
    {
	ret = -EINVAL;
    }
    else
    {
	*ppos +=count;
	ret = count;
	printk(KERN_INFO "read %d bytes from %d\n", count, p);
    }
    return ret;
}

static int gpio_write(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int val, ret;

    ret = copy_from_user(&val, buf, count);//copy data from APP buffer to val

    iowrite32(val, gpio_base);

    printk("gpio: write 0x%x to 0x%x. \n", val, (unsigned int)gpio_base);
    return 0;
}

static int gpio_map(struct file *filp, unsigned char *buffer, struct vm_area_struct *vma)
{
    unsigned long page;
    unsigned char i;
    unsigned long start; = (unsigned long)vma->vm_start;
    unsigned long size;
    void *kaddr;

    start = (unsigned long)vma->vm_start;
    size = (unsigned long)(vma->vm_end - vma->vm_start);
    vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
    vma->vm_private_date = filp->private_data;
    kaddr = filp->private_data;

    if(size > GPIO_SIZE)  //MAX mmap size cannot supass GPIO_SIZE
	size = GPIO_SIZE;

    page = virt_to_phys(kaddr); //Get APP's PHY addr

    if(remap_pfn_range(vma, start, page>>PAGE_SHIFT, size, PAGE_SHARED)) //Remap APP's VIR addr to OS's consective PHY page
    {
	printk("Error remap_pfn!\n");
	return -1;
    }
    return 0;
}


static lofft_t gpio_llseek(struct file *filp, lofft_t offset, int orig)
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
	    filp->f_ops = (unsigned int)offset;
	    ret = filp->f_ops;
	    break;
	case SEEK_CUR:
	    if((filp->f_pos+offset)>GPIO_SIZE)
	    {
		ret = -EINVAL;
		break;
	    }
	    if((filp->f_ops+offset)<0)
	    {
		ret = -EINVAL;
		break;
	    }
	    filp->f_ops += offset;
	    ret = filp->f_ops;
	    break;
	default:
	    ret = -EINVAL;
	    break;
    }
    return ret;
}

static const struct file_operations gpio_fops=
{
    .owner = THIS_MODULE,
    .open = gpio_open,
    .release = gpio_release,
    .write = gpio_write,
    .mmap = gpio_map,
    .llseek = gpio_llseek,
};

static struct miscdevice gpio_dev=
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "gpio_dev",
    .fops = &gpio_fops,
};

static int gpio_init(void)
{
    int ret;
    gpio_base = ioremap(GPIO_BASS_ADD, GPIO_SIZE); //change phy add to vir add
    printk("GPIO: Access address to device is:0x%x\n", (unsigned int)gpio_base);
    ret = misc_register(&gpio_dev);
    if(ret)
    {
	printk("gpio:[ERROR] Misc device register faigpio.\n");
	return ret;
    }

    printk("Module init complete!\n");
    return 0;
}

static void gpio_exit(void)
{
    printk("Module exit!\n");
    iounmap(gpio_base);
    misc_deregister(&gpio_dev);
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_AUTHOR("CHENGL");
MODULE_DESCRIPTION("GPIODriver");
MODULE_LICENSE("Dual BSD/GPL");
