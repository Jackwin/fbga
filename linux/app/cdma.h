#ifndef _CDMA_H
#define _CDMA_H

#define CDMA_BASE_ADDRESS     0x7e200000
#define PL_BRAM_SRC_ADDRESS  0xC4000000
#define DDR_BASE_ADDRESS    0x10000000

#define XGPIO_CHAN_OFFSET  8


#define XAXICDMA_CR_OFFSET      0x00000000  /**< Control register */
#define XAXICDMA_SR_OFFSET      0x00000004  /**< Status register */
#define XAXICDMA_CDESC_OFFSET   0x00000008  /**< Current descriptor pointer */
#define XAXICDMA_TDESC_OFFSET   0x00000010  /**< Tail descriptor pointer */
#define XAXICDMA_SRCADDR_OFFSET 0x00000018  /**< Source address register */
#define XAXICDMA_DSTADDR_OFFSET 0x00000020  /**< Destination address register */
#define XAXICDMA_BTT_OFFSET     0x00000028  /**< Bytes to transfer */

/** @name Bitmasks of XAXICDMA_CR_OFFSET register
 * @{
 */
#define XAXICDMA_CR_RESET_MASK  0x00000004 /**< Reset DMA engine */
#define XAXICDMA_CR_SGMODE_MASK 0x00000008 /**< Scatter gather mode */

/** @name Bitmask for interrupts
 * These masks are shared by XAXICDMA_CR_OFFSET register and
 * XAXICDMA_SR_OFFSET register
 * @{
 */
#define XAXICDMA_XR_IRQ_IOC_MASK    0x00001000 /**< Completion interrupt */
#define XAXICDMA_XR_IRQ_DELAY_MASK  0x00002000 /**< Delay interrupt */
#define XAXICDMA_XR_IRQ_ERROR_MASK  0x00004000 /**< Error interrupt */
#define XAXICDMA_XR_IRQ_ALL_MASK    0x00007000 /**< All interrupts */
#define XAXICDMA_XR_IRQ_SIMPLE_ALL_MASK 0x00005000 /**< All interrupts for
                                                        simple only mode */
/*@}*/

/** @name Bitmasks of XAXICDMA_SR_OFFSET register
 * This register reports status of a DMA channel, including
 */
#define XAXICDMA_CR_RESET_MASK  0x00000004 /**< Reset DMA engine */
#define XAXICDMA_CR_SGMODE_MASK 0x00000008 /**< Scatter gather mode */

/** @name Bitmask for interrupts
 * These masks are shared by XAXICDMA_CR_OFFSET register and
 * XAXICDMA_SR_OFFSET register
 * @{
 */
#define XAXICDMA_XR_IRQ_IOC_MASK    0x00001000 /**< Completion interrupt */
#define XAXICDMA_XR_IRQ_DELAY_MASK  0x00002000 /**< Delay interrupt */
#define XAXICDMA_XR_IRQ_ERROR_MASK  0x00004000 /**< Error interrupt */
#define XAXICDMA_XR_IRQ_ALL_MASK    0x00007000 /**< All interrupts */
#define XAXICDMA_XR_IRQ_SIMPLE_ALL_MASK 0x00005000 /**< All interrupts for*/



/** @name Bitmasks of XAXICDMA_SR_OFFSET register
 * This register reports status of a DMA channel, including
 * idle state, errors, and interrupts
 * @{
 */
#define XAXICDMA_SR_IDLE_MASK         0x00000002  /**< DMA channel idle */
#define XAXICDMA_SR_SGINCLD_MASK      0x00000008  /**< Hybrid build */
#define XAXICDMA_SR_ERR_INTERNAL_MASK 0x00000010  /**< Datamover internal err */
#define XAXICDMA_SR_ERR_SLAVE_MASK    0x00000020  /**< Datamover slave err */
#define XAXICDMA_SR_ERR_DECODE_MASK   0x00000040  /**< Datamover decode err */
#define XAXICDMA_SR_ERR_SG_INT_MASK   0x00000100  /**< SG internal err */
#define XAXICDMA_SR_ERR_SG_SLV_MASK   0x00000200  /**< SG slave err */
#define XAXICDMA_SR_ERR_SG_DEC_MASK   0x00000400  /**< SG decode err */
#define XAXICDMA_SR_ERR_ALL_MASK      0x00000770  /**< All errors */
/*@}*/

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define DDR_MAP_SIZE 0x10000000

#define DDR_MAP_MASK (DDR_MAP_SIZE - 1)

#define DDR_WRITE_OFFSET 0x10000000
#define BUFFER_BYTESIZE (256 * 4)  // Length of the buffers for DMA transfer. In the unit of byte
//#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
//--------------------------------------------------------------------
#define PL_PARAM_RAM_ADDR 0x40000000
#define INTEG_TIME_ADDR_OFFSET 1
int* MapDDR2UserSpace(void);
int UnmapDDR2UserSpace(int *mapped_ddr_base);
int* SetCDMA(void);
int DMAStart(void * mapped_dev_base, unsigned long char_cnt);

int SetPLParamRAM(unsigned char addr_off, unsigned int integ_time);
#endif




