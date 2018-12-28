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
#include <poll.h>

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
#define BUFFER_BYTESIZE     128  // Length of the buffers for DMA transfer
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
//--------------------------------------------------------------------
#define PL_PARAM_RAM_ADDR 0x40000000
#define INTEG_TIME_ADDR_OFFSET 1

static int DMAStart(void * mapped_dev_base, unsigned long char_cnt);

int main() {

    int valuefd, exportfd, directionfd;
    exportfd = open("/sys/class/gpio/export", O_WRONLY);
    if (exportfd < 0) {
        printf("Cannot open GPIO to export it\n");
        exit(1);
    }

    valuefd =  write(exportfd, "873", 32); // The width should be the GPIO_WIDTH export 873 -- LEDi
    if (valuefd < 0) {
        printf("Write gpio873 Error.\n");
//  exit(1);
    }
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

// --------------------------------- gpio_in------------------------------------
    int g11620_done_fd;
    // Set GPIO input
    exportfd = open("/sys/class/gpio/export", O_WRONLY);
    if (exportfd < 0) {
        printf("Cannot open GPIO to export it\n");
        exit(1);
    }
    valuefd = write(exportfd, "905", 32);

    write(exportfd, "rising", strlen("rising") + 1);
    close(exportfd);
    printf("GPIO905 exported successfully\n");
    // Set the direction
    directionfd = open("/sys/class/gpio/gpio905/direction", O_RDONLY);
    if (directionfd < 0) {
        printf("Cannot open GPIO direction 905\n");
        exit(1);
    }
    write(directionfd, "in", 1);
    close(directionfd);


// set edge
    valuefd = open("/sys/class/gpio/gpio905/edge", O_WRONLY);
    if (valuefd < 0) {
        printf("Cannot open GPIO value\n");
        exit(1);
    }
    write(valuefd, "rising", strlen("rising") + 1);
    close(valuefd);
// -------------------------------------------------------------
    int memfd;
    void *mapped_base, *mapped_dev_base;
    off_t dev_base = CDMA_BASE_ADDRESS;

    int memfd_ddr;
    void *mapped_ddr_base, *mapped_dev_ddr_base;
    off_t dev_ddr_base = DDR_BASE_ADDRESS;

    int memfd_2;
    void *mapped_base_2, *mapped_dev_base_2;
    off_t dev_base_2 = DDR_BASE_WRITE_ADDRESS;

    unsigned int TimeOut = 5;
    unsigned int ResetMask;
    unsigned int RegValue;
    unsigned int SrcArray[BUFFER_BYTESIZE ];
    unsigned int DestArray[BUFFER_BYTESIZE ];
    unsigned int Index;

// ---------------- Map the DDR to user space-----------------------------------
    memfd_ddr = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd_ddr == -1) {
        printf("Can't open /dev/mem.\n");
        exit(0);
    }
    printf("/dev/mem opened.\n");
    // Map one page of memory into user space such that the device is in that page, but it may not
    // be at the start of the page.
    mapped_ddr_base = mmap(0, DDR_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd_ddr, dev_ddr_base & ~DDR_MAP_MASK);
    if (mapped_ddr_base == (void *) - 1) {
        printf("Can't map the DDR memory to user space.\n");
        exit(0);
    }
    printf("Memory mapped at address %p.\n", mapped_ddr_base);
    // get the address of the device in user space which will be an offset from the base
    // that was mapped as memory is mapped at the start of a page
    mapped_dev_ddr_base = mapped_ddr_base + (dev_ddr_base & DDR_MAP_MASK);

    if (munmap(mapped_ddr_base, DDR_MAP_SIZE) == -1) {
        printf("Can't unmap memory from user space.\n");
        exit(0);
    }
    close(memfd_ddr);
// ------------------ Map the AXI CDMA Register memory to the User layer--------------------
    memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd == -1) {
        printf("Can't open /dev/mem.\n");
        exit(0);
    }
    printf("/dev/mem opened.\n");

    // Map one page of memory into user space such that the device is in that page, but it may not
    // be at the start of the page.
    mapped_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, dev_base & ~MAP_MASK);
    if (mapped_base == (void *) - 1) {
        printf("Can't map the memory to user space.\n");
        exit(0);
    }
    // get the address of the device in user space which will be an offset from the base
    // that was mapped as memory is mapped at the start of a page
    mapped_dev_base = mapped_base + (dev_base & MAP_MASK);
    //Reset CDMA
    do {
        ResetMask = (unsigned long )XAXICDMA_CR_RESET_MASK;
        *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_CR_OFFSET)) = (unsigned long)ResetMask;
        /* If the reset bit is still high, then reset is not done   */
        ResetMask = *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_CR_OFFSET));
        if (!(ResetMask & XAXICDMA_CR_RESET_MASK)) {
            break;
        }
        TimeOut -= 1;
    } while (TimeOut);
    //enable Interrupt
    RegValue = *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_CR_OFFSET));
    RegValue = (unsigned long)(RegValue | XAXICDMA_XR_IRQ_ALL_MASK );
    *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_CR_OFFSET)) = (unsigned long)RegValue;
    // Checking for the Bus Idle
    RegValue = *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_SR_OFFSET));
    if (!(RegValue & XAXICDMA_SR_IDLE_MASK)) {
        printf("BUS IS BUSY Error Condition \n\r");
        return 1;
    }
    // Check the DMA Mode and switch it to simple mode
    RegValue = *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_CR_OFFSET));
    if ((RegValue & XAXICDMA_CR_SGMODE_MASK)) {
        RegValue = (unsigned long)(RegValue & (~XAXICDMA_CR_SGMODE_MASK));
        printf("Reading \n \r");
        *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_CR_OFFSET)) = (unsigned long)RegValue;
    }
    /*
        //Set the Source Address
        *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_SRCADDR_OFFSET)) = (unsigned long)PL_BRAM_SRC_ADDRESS;
        //Set the Destination Address
        *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_DSTADDR_OFFSET)) = (unsigned long)DDR_BASE_ADDRESS;
        RegValue = (unsigned long)(BUFFER_BYTESIZE);
        // write Byte to Transfer
        *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_BTT_OFFSET)) = (unsigned long)RegValue;

    //--------------------------Wait for the DMA transfer Status ---------------------------
        do {
            RegValue = *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_SR_OFFSET));
        } while (!(RegValue & XAXICDMA_XR_IRQ_ALL_MASK));

        if ((RegValue & XAXICDMA_XR_IRQ_IOC_MASK)) {
            printf("Transfer Completed \n\r ");
        }
        if ((RegValue & XAXICDMA_XR_IRQ_DELAY_MASK)) {
            printf("IRQ Delay Interrupt\n\r ");
        }
        if ((RegValue & XAXICDMA_XR_IRQ_ERROR_MASK)) {
            printf(" Transfer Error Interrupt\n\r ");
        }
    */
//-------------- Un-map the AXI CDMA memory from the User layer.-------------------------
    /*
        if (munmap(mapped_base, MAP_SIZE) == -1) {
            printf("Can't unmap memory from user space.\n");
            exit(0);
        }

        close(memfd);
    */
// -------------------- Set PL_Param RAM ----------------------------
    int fd, n;
    unsigned char *map_base;
    unsigned char *buffer;
    unsigned char buf_size = 4;

    buffer = (char *)malloc(buf_size * sizeof(char));
    if (buffer == NULL) {
        printf("Fail to malloc buffer.\n");
        return 0;
    }
    memset(buffer, 0 , buf_size);

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        printf("Error_open: \n");
    }
    map_base = mmap(NULL, buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PL_PARAM_RAM_ADDR);
    if (map_base == NULL) {
        printf("Error2: \n");
        exit(1);
    } else {
        printf("sucessful: \n");
    }


    int *integ_time_ptr = ((int *)map_base) + INTEG_TIME_ADDR_OFFSET;
    *integ_time_ptr = 0xfff0; // set the integration time

    if (munmap(map_base, buf_size) == -1) {
        printf("Can't unmap memory from user space.\n");
        exit(0);
    }

    close(fd);
//---------------------------- main ----------------------------------------

    g11620_done_fd = open("/sys/class/gpio/gpio905/value", O_RDONLY | O_NONBLOCK );
    if (g11620_done_fd < 0) {
        printf("Cannot open g11620_done_fd value\n");
        exit(1);
    }

    struct pollfd fds[1];
    memset((void*)fds, 0, sizeof(fds));

    char rd_data;
    unsigned long char_cnt;
    int status;
    char_cnt = BUFFER_BYTESIZE;

    while (1) {
        fds[0].fd = g11620_done_fd;
        fds[0].events = POLLPRI;
        read(g11620_done_fd, &rd_data, sizeof(rd_data)); // Initial read to prevent false edge
        if (poll(fds, 1, POLL_TIMEOUT) == -1) {
            perror(" G11620_done_fd poll failed!\n");
            return -1;
        }

        if (fds[0].revents & POLLPRI) {
            if (lseek(g11620_done_fd, 0, SEEK_SET) == -1) {
                perror("lseek failed!\n");
                return -1;
            }
            int len;
            if ((len = read(g11620_done_fd, &rd_data, sizeof(rd_data))) == -1) {
                perror("read failed!\n");
                return -1;
            }


            printf("Capture the edge.\n");
            if (rd_data == 0x31) {
                status = DMAStart(mapped_dev_base, char_cnt);
            }
        }
        fflush(stdout);
    }
}


static int DMAStart(void * mapped_dev_base, unsigned long char_cnt) {
    unsigned long reg_value;
    //Set the Source Address
    *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_SRCADDR_OFFSET)) = (unsigned long)PL_BRAM_SRC_ADDRESS;
    //Set the Destination Address
    *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_DSTADDR_OFFSET)) = (unsigned long)DDR_BASE_ADDRESS;
    //RegValue = (unsigned long)(BUFFER_BYTESIZE);
    // write Byte to Transfer
    *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_BTT_OFFSET)) = (unsigned long)char_cnt;

    //--------------------------Wait for the DMA transfer Status ---------------------------
    do {
        reg_value = *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_SR_OFFSET));
    } while (!(reg_value & XAXICDMA_XR_IRQ_ALL_MASK));

    if ((reg_value & XAXICDMA_XR_IRQ_IOC_MASK)) {
        printf("Transfer Completed \n\r ");
        return 1;
    }
    if ((reg_value & XAXICDMA_XR_IRQ_DELAY_MASK)) {
        printf("IRQ Delay Interrupt\n\r ");
        return 2;
    }
    if ((reg_value & XAXICDMA_XR_IRQ_ERROR_MASK)) {
        printf(" Transfer Error Interrupt\n\r ");
        return 0;
    }

}
