/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xgpio.h"
#include "sleep.h"
#include "xaxicdma.h"
#include "xil_cache.h"
#include "xscutimer.h"
#include "xil_exception.h"

#define TIMER_LOAD_VALUE    0xFFFFFFFF
#define PL_BRAM_ADDR 0xc2000000
#define PL_DATA_ADDR 0xc4000000
#define BRAM_GP0_ADDR 0x42000000
#define PS_DDR_ADDR 0x10000000 // not start 0x0
#define TOTAL_NUM 512
#define BUF_LEN (TOTAL_NUM * 4) // Byte


static XAxiCdma_Config *axi_cdma_cfg;
static XAxiCdma axi_cdma;
u16 dma_done;
/* Instance For GPIO */
//XGpio GpioOutput;

static XScuTimer Timer;
int hs_timer(void)
{
    int Status;
    XScuTimer_Config *ConfigPtr;
    XScuTimer *TimerInstancePtr=&Timer;
    /*
     * Initialize the Scu Private Timer so that it is ready to use.
     */
    ConfigPtr = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);

    /*
     * This is where the virtual address would be used, this example
     * uses physical address.
     */
    Status = XScuTimer_CfgInitialize(TimerInstancePtr, ConfigPtr, ConfigPtr->BaseAddr);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    else {
        xil_printf("XScuTimer_CfgInitialize OK\n\r");
        return XST_SUCCESS;
    }
}



int cdma_test(void)
{
    u32 i,k ,data = 0;
    int status;
    volatile int CntValue1,CntValue2;
    dma_done = 0;
    /*
    // BRAM_GP0_ADDR is the BRAM address used for the DMA test, which is accessable via GP(BRAM_GP0_ADDR)
    // BRAM_GP0_ADDR is used to initialize the PL_BRAM_ADDR contents
     u32 *rd_ram    = (u32 *) BRAM_GP0_ADDR;
      // Initialize the PL_BRAM_ADDR contents
    data = 0;
     for (i = 0; i < TOTAL_NUM; i++) {
        data = data + 1;
        *(rd_ram + i) = data;
    }
    */
    // PL_DATA_ADDR is the address for the ADC data in PL
    u32 *tx_buffer = (u32 *) PL_DATA_ADDR;
    // DDR address
    u32 *rx_buffer = (u32 *) PS_DDR_ADDR;
   // init_platform();

    status = hs_timer();
    if (status != XST_SUCCESS) {
        xil_printf("Fail to enable timer.\n");
        return 0;
    }

    // Set up the AXI CDMA
    printf("--Set up the AXI CDMA\n\r");
    axi_cdma_cfg = XAxiCdma_LookupConfig(XPAR_AXICDMA_0_DEVICE_ID);
    if (!axi_cdma_cfg) {
        printf("AXAxiCdma_LookupConfig failed\n\r");
    }

    status = XAxiCdma_CfgInitialize(&axi_cdma, axi_cdma_cfg, axi_cdma_cfg->BaseAddress);
    if (status == XST_SUCCESS ){
        printf("XAxiCdma_CfgInitialize succeed\n\r");
    }
    printf("--Disable Interrupt of AXI CDMA\n\r");
    XAxiCdma_IntrDisable(&axi_cdma, XAXICDMA_XR_IRQ_ALL_MASK);

    if (XAxiCdma_IsBusy(&axi_cdma)) {
        printf("AXI CDMA is busy...\n\r");
        while (XAxiCdma_IsBusy(&axi_cdma));
    }

    Xil_DCacheFlush();

    XScuTimer_LoadTimer(&Timer, TIMER_LOAD_VALUE);
    CntValue1 = XScuTimer_GetCounterValue(&Timer);
    XScuTimer_Start(&Timer);

    status = XAxiCdma_SimpleTransfer(
                                     &axi_cdma,
                                     (u32) tx_buffer, // BRAM
                                     (u32) rx_buffer, // DDR
                                     BUF_LEN,
                                     NULL,
                                     NULL);
    if(status != XST_SUCCESS) {
        xil_printf("DMA fails.\n\r");
    }
    Xil_DCacheFlush();

    // Wait until core isn't busy
    if (XAxiCdma_IsBusy(&axi_cdma)) {
        printf("AXI CDMA is busy...\n\r");
        while (XAxiCdma_IsBusy(&axi_cdma));
    }
    XScuTimer_Stop(&Timer);


    CntValue2 = XScuTimer_GetCounterValue(&Timer);
   // printf ("CntValue2 is %x.\n", CntValue2);
    int duration = (CntValue1-CntValue2)*3/1000;
    int rate = TOTAL_NUM * 4 / duration;
    printf("DMA from BRAM to DDR is %d MB/s\n\r", rate);  // Timer fre is 333.33MHz

     Xil_DCacheInvalidateRange((u32 )PS_DDR_ADDR, BUF_LEN);

     for(i=0; i<16; i++)
     {
         k = *(rx_buffer + i);
         xil_printf("The DMA read from address = %2d, the read value = %8x-----------------------\n\r",i,k);
     }
     dma_done = 1;
     return 1;
}
