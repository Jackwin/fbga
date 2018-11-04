

#ifndef DMA_INTR_H

#define DMA_INTR_H

#include "xaxicdma.h"

#include "xparameters.h"

#include "xil_exception.h"

#include "xdebug.h"

#include "xscugic.h"
#include "xil_cache.h"
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif

/************************** Constant Definitions *****************************/

/*

* Device hardware build related constants.

*/

#define DMA_DEV_ID XPAR_AXIDMA_0_DEVICE_ID

#define MEM_BASE_ADDR 0x01000000

#define RX_INTR_ID XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR

#define TX_INTR_ID XPAR_FABRIC_AXI_DMA_0_MM2S_INTROUT_INTR

//#define INTC_DEVICE_ID XPAR_PS7_SCUGIC_0_DEVICE_ID

#define TX_BUFFER_BASE (MEM_BASE_ADDR + 0x00100000)

#define RX_BUFFER_BASE (MEM_BASE_ADDR + 0x00300000)

#define RX_BUFFER_HIGH (MEM_BASE_ADDR + 0x004FFFFF)

/* Timeout loop counter for reset

*/

#define RESET_TIMEOUT_COUNTER 10000

/* test start value

*/

#define TEST_START_VALUE 0xC

/*

* Buffer and Buffer Descriptor related constant definition

*/

#define MAX_PKT_LEN 256//4MB

/*

* transfer times

*/

#define NUMBER_OF_TRANSFERS 4

extern volatile int TxDone;

extern volatile int RxDone;

extern volatile int Error;

static int  SetupIntrSystem(XScuGic  *IntcInstancePtr, XAxiCdma *InstancePtr, u32 IntrId);

static void CallBack(void *CallBackRef, u32 IrqMask, int *IgnorePtr);

static void DisableIntrSystem(XScuGic *IntcInstancePtr, u32 IntrId);

int XAxiCdma_SetupIntr(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
    u16 DeviceId, u32 IntrId);

int  DMA_CheckData(int Length, u8 StartValue);
void CDMAIntrHandler(void *CallBackRef);

#endif
