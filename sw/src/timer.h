#ifndef TIMER_H
#define TIMER_H

#include "xparameters.h"
#include "xscutimer.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_printf.h"


#define TIMER_DEVICE_ID     XPAR_XSCUTIMER_0_DEVICE_ID
#define INTC_DEVICE_ID      XPAR_SCUGIC_0_DEVICE_ID
#define TIMER_IRPT_INTR     XPAR_SCUTIMER_INTR

void TimerInit(XScuGic intc, XScuTimer timer, u32 timer_load_value, u32 device_id);
void SetupTimerInterruptSystem(XScuGic *GicInstancePtr, XScuTimer *TimerInstancePtr, u16 TimerIntrId);
void TimerIntrHandler(void *CallBackRef);



#endif
