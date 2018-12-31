
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

#include "cdma.h"

int *MapDDR2UserSpace(void) {
    int memfd_ddr;
    void *mapped_ddr_base, *mapped_dev_ddr_base;
    off_t dev_ddr_base = DDR_BASE_ADDRESS;
    memfd_ddr = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd_ddr == -1) {
        printf("Can't open /dev/mem.\n");
        exit(0);
    }
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
    // mapped_dev_ddr_base = mapped_ddr_base + (dev_ddr_base & DDR_MAP_MASK);

    close(memfd_ddr);
    return mapped_ddr_base;
}

int UnmapDDR2UserSpace(int *mapped_ddr_base) {

    if (munmap(mapped_ddr_base, DDR_MAP_SIZE) == -1) {
        printf("Can't unmap memory from user space.\n");
        exit(0);
    }
    return 0;
}
int* SetCDMA(void) {

    int memfd;
    void *mapped_base, *mapped_dev_base;
    off_t dev_base = CDMA_BASE_ADDRESS;

    int memfd_ddr;
    void *mapped_ddr_base, *mapped_dev_ddr_base;
    off_t dev_ddr_base = DDR_BASE_ADDRESS;

    unsigned int TimeOut = 5;
    unsigned int ResetMask;
    unsigned int RegValue;
    unsigned int SrcArray[BUFFER_BYTESIZE ];
    unsigned int DestArray[BUFFER_BYTESIZE ];
    unsigned int Index;
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
        // If the reset bit is still high, then reset is not done
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
        // return 1;
    }
    // Check the DMA Mode and switch it to simple mode
    RegValue = *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_CR_OFFSET));
    if ((RegValue & XAXICDMA_CR_SGMODE_MASK)) {
        RegValue = (unsigned long)(RegValue & (~XAXICDMA_CR_SGMODE_MASK));
        printf("Reading \n \r");
        *((volatile unsigned long *) (mapped_dev_base + XAXICDMA_CR_OFFSET)) = (unsigned long)RegValue;
    }
    close(memfd);
    return mapped_dev_base;
}


int DMAStart(void * mapped_dev_base, unsigned long char_cnt) {
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

int SetPLParamRAM(unsigned char addr_off, unsigned int integ_time) {

    int fd, n;
    unsigned char *map_base;
    unsigned char *buffer;
    unsigned char buf_size = 32;

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

    int *integ_time_ptr = ((int *)map_base) + addr_off;
    *integ_time_ptr = integ_time; // set the integration time

    if (munmap(map_base, buf_size) == -1) {
        printf("Can't unmap memory from user space.\n");
        exit(0);
    }
    free(buffer);
    close(fd);
}
