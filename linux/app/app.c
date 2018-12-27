#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>

#define CDMA_BASE_ADDRESS     0x7e200000
#define GPIO_DATA_OFFSET     0
#define GPIO_DIRECTION_OFFSET     4
#define DDR_BASE_ADDRESS     0x20000000
#define PL_PARAM_
#define PL_BRAM_SRC_ADDRESS  0xC4000000
#define DDR_BASE_WRITE_ADDRESS    0x30000000

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
#define XAXICDMA_XR_IRQ_SIMPLE_ALL_MASK 0x00005000 /**< All interrupts for
                                                        simple only mode */
/*@}*/

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
#define BUFFER_BYTESIZE     4096  // Length of the buffers for DMA transfer

//--------------------------------------------------------------------
#define PL_PARAM_RAM_ADDR 0x40000000
#define INTEG_TIME_ADDR_OFFSET 1

int main() {

	int valuefd, exportfd, directionfd; 
	exportfd = open("/sys/class/gpio/export", O_WRONLY); 
	if (exportfd < 0) {
		 printf("Cannot open GPIO to export it\n"); 
		exit(1);
	 }
	 
	write(exportfd, "873", 32); // The width should be the GPIO_WIDTH export 873 -- LED
    	close(exportfd);
    	printf("GPIO exported successfully\n");
	
	// Set the direction	
	directionfd = open("/sys/class/gpio/gpio873/direction", O_RDWR); 
	if (directionfd < 0) { 
		printf("Cannot open GPIO direction it\n"); 
		exit(1);
	 }
	write(directionfd, "out", 32); 
	close(directionfd); 
	printf("GPIO direction set as output successfully\n");

	// Set the value
	valuefd = open("/sys/class/gpio/gpio873/value", O_RDWR); 
	if (valuefd < 0) { 
		printf("Cannot open GPIO value\n"); 
		exit(1); 
	}
	 printf("GPIO value opened, now toggling...\n");

// -------------------- Set PL_Param RAM ----------------------------
	int fd, n;
	unsigned char *map_base;
	unsigned char *buffer;
	unsigned char buf_size = 4;

	buffer = (char *)malloc(buf_size* sizeof(char));
	if (buffer == NULL) {
		printf("Fail to malloc buffer.\n");
		return 0;
	}	
	memset(buffer, 0 ,buf_size);

    	fd = open("/dev/mem", O_RDWR | O_SYNC); 
    	if(fd == -1) {
      		printf("Error_open: \n");
    	}
    	map_base = mmap(NULL, buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PL_PARAM_RAM_ADDR); 
    	if(map_base == NULL) {
     	 	printf("Error2: \n");
		exit(1);
    	} else {
     		 printf("sucessful: \n");
   	}
	int *integ_time_ptr = ((int *)map_base) + INTEG_TIME_ADDR_OFFSET;
	*integ_time_ptr = 0xfff0; // set the integration time


	while (1) { 
		write(valuefd,"1", 2); 
		sleep(1); 
		write(valuefd,"0", 2);
		 sleep(1);
	 }

}
