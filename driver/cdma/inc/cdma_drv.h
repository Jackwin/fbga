#ifndef _CDMA_H
#define _CDMA_H

#define DEVICE_NAME "cdma_drv"
#define CDMA_REG_SIZE 1024*64
#define CDMA_BASS_ADD 0x7e200000
#define DMA_IRQ_NUM 62

#define WR_OP 0
#define RD_OP 1

static void __iomem *cdma_drv_base; //cdma_drv base address
static unsigned char *kmalloc_area;

#endif
