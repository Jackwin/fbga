#include "dma_intr.h"
volatile static int Done = 0;	/* Dma transfer is done */
volatile static int error = 0;	/* Dma Bus Error occurs */
#define PL_BRAM_ADDR 0xC2000000
#define PL_ADC_DATA_ADDR 0xc4000000
#define BRAM_GP0_ADDR 0x42000000
#define PS_DDR_ADDR 0x10000000
#define TOTAL_NUM 2
#define BUF_LEN (TOTAL_NUM * 4) // Byte
#define BUFFER_BYTESIZE		64
volatile static u8 SrcBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));
volatile static u8 DestBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));
/*
static int  SetupIntrSystem(XScuGic  *IntcInstancePtr, XAxiCdma *InstancePtr, u32 IntrId);
static void CallBack(void *CallBackRef, u32 IrqMask, int *IgnorePtr);
static void DisableIntrSystem(XScuGic *IntcInstancePtr, u32 IntrId);
*/
int XAxiCdma_SetupIntr(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
    u16 DeviceId, u32 IntrId) {
    XAxiCdma_Config *CfgPtr;
    int i ,k, data = 0;
    u8  *SrcPtr;
    u8  *DestPtr;
    u32 *rx_buffer = (u32 *) PS_DDR_ADDR;
    u32 *tx_buffer = (u32 *) PL_BRAM_ADDR;
   // u32 *tx_buffer = (u32 *)PL_ADC_DATA_ADDR;
    u32 *rd_ram    = (u32 *) BRAM_GP0_ADDR;
    data = 0;
       for (i = 0; i < TOTAL_NUM; i++) {
       	data = data + 1;
       	*(rd_ram + i) = data;
       }
   	SrcPtr = (u8 *)SrcBuffer;
   	DestPtr = (u8 *)DestBuffer;
   	for (u32 Index = 0; Index < BUFFER_BYTESIZE; Index++) {
   		SrcPtr[Index] = Index & 0xFF;
   		DestPtr[Index] = 0;
   	}


    int Status;
    int SubmitTries = 10;       /* Retry to submit */
    int Tries = NUMBER_OF_TRANSFERS;
    int Index;

    /* Initialize the XAxiCdma device.
     */
    CfgPtr = XAxiCdma_LookupConfig(DeviceId);
    if (!CfgPtr) {
        return XST_FAILURE;
    }

    Status = XAxiCdma_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /* Setup the interrupt system
     */
    Status = SetupIntrSystem(IntcInstancePtr, InstancePtr, IntrId);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /* Enable all (completion/error/delay) interrupts
     */
    XAxiCdma_IntrEnable(InstancePtr, XAXICDMA_XR_IRQ_ALL_MASK);

    for (Index = 0; Index < Tries; Index++) {
    	//Xil_DCacheFlush();
    	//Xil_DCacheFlushRange((UINTPTR)&SrcBuffer, BUFFER_BYTESIZE);
    	for (int retries = 10; retries > 0; retries--) {
    	    Status = XAxiCdma_SimpleTransfer(
    	    								 InstancePtr,
    	                                     (u32) tx_buffer, // BRAM
    	                                     (u32) rx_buffer, // DDR
											 BUFFER_BYTESIZE,
    										 (XAxiCdma_CallBackFn)CallBack,
											 (void *)InstancePtr);
    	    if (Status == XST_SUCCESS) {
    	    			break;
    	    		}
    	}
    	   // Xil_DCacheFlush();
    	    while(!Done && !error){}

    	  //  Xil_DCacheInvalidateRange((UINTPTR)&DestBuffer, BUFFER_BYTESIZE);

        if(Status != XST_SUCCESS) {
            DisableIntrSystem(IntcInstancePtr, IntrId);
            xil_printf("CDMA intr error.\n\r");
            return XST_FAILURE;
        }
    }

    /* Test finishes successfully, clean up and return
     */
    DisableIntrSystem(IntcInstancePtr, IntrId);

    return XST_SUCCESS;


}

static int SetupIntrSystem(XScuGic  *IntcInstancePtr, XAxiCdma *InstancePtr, u32 IntrId) {
    int Status;
	/*
     * Initialize the interrupt controller driver
     */
    XScuGic_Config *IntcConfig;

    /*
     * Initialize the interrupt controller driver so that it is ready to
     * use.
     */
    IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
    if (NULL == IntcConfig) {
    	return XST_FAILURE;
    }

    Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
    				IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS) {
    	return XST_FAILURE;
    }

    XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId, 0xA0, 0x3);

    /*
     * Connect the device driver handler that will be called when an
     * interrupt for the device occurs, the handler defined above performs
     * the specific interrupt processing for the device.
     */
    Status = XScuGic_Connect(IntcInstancePtr, IntrId,
    			//(Xil_InterruptHandler)XAxiCdma_IntrHandler,
    		(Xil_InterruptHandler)CDMAIntrHandler,
    			InstancePtr);
    if (Status != XST_SUCCESS) {
    	return Status;
    }

    /*
     * Enable the interrupt for the DMA device.
     */
    XScuGic_Enable(IntcInstancePtr, IntrId);


    Xil_ExceptionInit();

    /*
     * Connect the interrupt controller interrupt handler to the hardware
     * interrupt handling logic in the processor.
     */
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
            (Xil_ExceptionHandler)XScuGic_InterruptHandler, IntcInstancePtr);


    /*
     * Enable interrupts in the Processor.
     */
    Xil_ExceptionEnable();
    return XST_SUCCESS;

}
/* Callback is called by the interrupt handler
 * *
 */
static void CallBack (void *CallBackRef, u32 IrqMask, int *IgnorePtr) {
    if (IrqMask & XAXICDMA_XR_IRQ_ERROR_MASK) {
        error = TRUE;
    }

    if (IrqMask & XAXICDMA_XR_IRQ_IOC_MASK) {
        Done = TRUE;
    }


}


/*
*   Disables the interrupt for the XAxiCdma device
*/
static void DisableIntrSystem(XScuGic *IntcInstancePtr, u32 IntrId)
{

    /* Disconnect the interrupt
     */
    XScuGic_Disable(IntcInstancePtr, IntrId);
    XScuGic_Disconnect(IntcInstancePtr, IntrId);

}

void CDMAIntrHandler(void *CallBackRef) {

    xil_printf(" ------CDMA interrupt handler -------------------\n\r");
    u32 i, k;
    u32 *rx_buffer = (u32 *) PS_DDR_ADDR;
    for(i=0; i<256; i++)
	 //for(i=0; i<BUFF_LEN/4; i++)
	 {
		 k = *(rx_buffer + i);
		//\ k = DestBuffer[i];
		 xil_printf("The DMA read from address = %2d, the     read value = %8x-----------------------\n\r",i,k);
	 }

}

