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

#include "cdma_drv.h"
#include "dma_transf.h"


int cdma_drv_open(struct inode * inode, struct file *filp)
{
    printk("device is open!\n");
    return 0;
}

int cdma_drv_release(struct inode *inode, struct file *filp)
{
    printk("device is release!\n");
    return 0;
}

static ssize_t cdma_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    unsigned int ret = 0;

    struct cdma_drv_priv_data *devp = filp->private_data;
    
    DMAStart(16, RD_OP);

    if(copy_to_user(buf, (void*)(kmalloc_area), count))
    {
	ret = -EINVAL;
    }
    
    return ret;
}

static ssize_t cdma_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int val, ret;

    ret = copy_from_user(&kmalloc_area, buf, count);//copy data from APP buffer to val

    DMAStart(16, WR_OP);

    printk("cdma_drv dma write to PL\n");
    return 0;
}

//static int cdma_drv_map(struct file *filp, unsigned char *buffer, struct vm_area_struct *vma)
//{
//    unsigned long page;
//    unsigned char i;
//    unsigned long start; = (unsigned long)vma->vm_start;
//    unsigned long size;
//    void *kaddr;
//
//    start = (unsigned long)vma->vm_start;
//    size = (unsigned long)(vma->vm_end - vma->vm_start);
//    vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
//    vma->vm_private_date = filp->private_data;
//    kaddr = filp->private_data;
//
//    if(size > CDMA_REG_SIZE)  //MAX mmap size cannot supass CDMA_REG_SIZE
//	size = CDMA_REG_SIZE;
//
//    page = virt_to_phys(kaddr); //Get APP's PHY addr
//
//    if(remap_pfn_range(vma, start, page>>PAGE_SHIFT, size, PAGE_SHARED)) //Remap APP's VIR addr to OS's consective PHY page
//    {
//	printk("Error remap_pfn!\n");
//	return -1;
//    }
//    return 0;
//}


static loff_t cdma_drv_llseek(struct file *filp, loff_t offset, int orig)
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
	    if((unsigned int) offset > CDMA_REG_SIZE)
	    {
		ret = -EINVAL;
		break;
	    }
	    filp->f_op = (unsigned int)offset;
	    ret = filp->f_op;
	    break;
	case SEEK_CUR:
	    if((filp->f_pos+offset)>CDMA_REG_SIZE)
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
	    ret = filp->f_op;
	    break;
	default:
	    ret = -EINVAL;
	    break;
    }
    return ret;
}

static const struct file_operations cdma_drv_fops=
{
    .owner = THIS_MODULE,
    .open = cdma_drv_open,
    .release = cdma_drv_release,
    .write = cdma_drv_write,
    .llseek = cdma_drv_llseek,
};

static struct miscdevice cdma_drv_dev=
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &cdma_drv_fops,
};


static int cdma_drv_init(void)
{
    int ret;
    int result;
    
    cdma_drv_base = ioremap(CDMA_BASS_ADD, CDMA_SIZE); //change phy add to vir add
    
    kmalloc_area = kmalloc(BUFFER_BYTESIZE, __GFP_DMA);
    if(!kmalloc_area) 
    {
	return -EINVAL;
	goto error_handle;
    }

    memset(kmalloc_area, 0, BUFFER_BYTESIZE); //set 0 for kmalloc memory

    SetPageReserved(virt_to_page(kmalloc_area);

    reset_cdma();
    
    set_irq_type(DMA_IRQ_NUM, IRQ_TYPE_EDGE_RISING);
    ret = request_irq(DMA_IRQ_NUM, dma_irq_handle, SA_INTERRUPT, DEVICE_NAME, NULL);
    if(ret)
    {
    	printk(" Can not apply IRQ for DMA\n");
	return ret;
	goto error_handle;
    }

    printk("CDMA: Access address to device is:0x%x\n", (unsigned int)cdma_drv_base);
    ret = misc_register(&cdma_drv_dev);
    if(ret)
    {
	printk("cdma_drv:[ERROR] Misc device register faicdma_drv.\n");
	return ret;
    }

    printk("Module init complete!\n");
    return 0;

error_handle:
    misc_deregister(&cdma_drv_dev);
}

static void cdma_drv_exit(void)
{
    printk("Module exit!\n");
    iounmap(cdma_drv_base);
    kfree(kmalloc_area);
    free_irq(DMA_IRQ_NUM);
    misc_deregister(&cdma_drv_dev);
}

module_init(cdma_drv_init);
module_exit(cdma_drv_exit);

MODULE_AUTHOR("CHENGL");
MODULE_DESCRIPTION("CDMA_DRV");
MODULE_LICENSE("Dual BSD/GPL");
