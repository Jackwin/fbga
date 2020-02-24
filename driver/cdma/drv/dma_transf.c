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
#include <linux/irqreturn.h>
#include <linux/interrupt.h>

#include "dma_transf.h"
#include "cdma_drv.h"


static irqreturn_t dma_irq_handle(int irq, void *dev_id)
{
    return IRQ_HANDLED;
}

int reset_cdma(void)
{
    unsigned long ResetMask;
    unsigned int RegValue;
    unsigned int TimeOut = 5;

    do {
        ResetMask = (unsigned long )XAXICDMA_CR_RESET_MASK;
        iowrite32(ResetMask, cdma_drv_base + XAXICDMA_CR_OFFSET);
        // If the reset bit is still high, then reset is not done
        ResetMask = ioread32(cdma_drv_base + XAXICDMA_CR_OFFSET);
        if (!(ResetMask & XAXICDMA_CR_RESET_MASK)) {
            break;
        }
        TimeOut -= 1;
    } while (TimeOut);

    // Checking for the Bus Idle
    RegValue = ioread32(cdma_drv_base + XAXICDMA_SR_OFFSET);
    if (!(RegValue & XAXICDMA_SR_IDLE_MASK)) {
        printk("BUS IS BUSY Error Condition \n\r");
	return 3;
    }

    // Check the DMA Mode and switch it to simple mode
    RegValue = ioread32(cdma_drv_base + XAXICDMA_CR_OFFSET);
    if ((RegValue & XAXICDMA_CR_SGMODE_MASK)) {
        RegValue = (unsigned long)(RegValue & (~XAXICDMA_CR_SGMODE_MASK));
        printk("Reading \n \r");
        iowrite32(RegValue, cdma_drv_base + XAXICDMA_CR_OFFSET);
    }

    return 0;
}


int DMAStart(unsigned int char_cnt, unsigned int direction) {
    unsigned int reg_value;
    unsigned long host_phy_addr;
    unsigned long dest_addr, src_addr;
    host_phy_addr = virt_to_phys((void *)kmalloc_area); 

    if(direction == WR_OP)
    {
	dest_addr = PL_BRAM_SRC_ADDRESS;
	src_addr = host_phy_addr;
    }
    else
    {
	dest_addr = host_phy_addr;
	src_addr = PL_BRAM_SRC_ADDRESS;
    }

    //Set the Source Address
    iowrite32(src_addr, cdma_drv_base + XAXICDMA_SRCADDR_OFFSET);
    //Set the Destination Address
    iowrite32(dest_addr, cdma_drv_base + XAXICDMA_DSTADDR_OFFSET);
    
    //RegValue = (unsigned long)(BUFFER_BYTESIZE);
    // write Byte to Transfer
    iowrite32(char_cnt, cdma_drv_base + XAXICDMA_BTT_OFFSET);

    //--------------------------Wait for the DMA transfer Status ---------------------------
    do {
        reg_value = ioread32(cdma_drv_base + XAXICDMA_SR_OFFSET);
    } while (!(reg_value & XAXICDMA_XR_IRQ_ALL_MASK));

    if ((reg_value & XAXICDMA_XR_IRQ_IOC_MASK)) {
        printk("Transfer Completed \n\r ");
        return 1;
    }
    if ((reg_value & XAXICDMA_XR_IRQ_DELAY_MASK)) {
        printk("IRQ Delay Interrupt\n\r ");
        return 2;
    }
    if ((reg_value & XAXICDMA_XR_IRQ_ERROR_MASK)) {
        printk(" Transfer Error Interrupt\n\r ");
        return 0;
    }

}

