#include "timer.h"

void TimerInit(XScuGic intc, XScuTimer timer, u32 timer_load_value, u32 device_id) {

	 XScuTimer_Config *timer_config_ptr;
     xil_printf("------------Timee initialization starts-------------\n");

     timer_config_ptr = XScuTimer_LookupConfig(device_id);
     XScuTimer_CfgInitialize(&timer, timer_config_ptr,timer_config_ptr->BaseAddr);
     XScuTimer_SelfTest(&timer);

     XScuTimer_LoadTimer(&timer, timer_load_value);
     XScuTimer_EnableAutoReload(&timer);
     XScuTimer_Start(&timer);
     //set up the interrupts
     SetupTimerInterruptSystem(&intc,&timer,TIMER_IRPT_INTR);

}

void SetupTimerInterruptSystem(XScuGic *GicInstancePtr, XScuTimer *TimerInstancePtr, u16 TimerIntrId)

{
    XScuGic_Config *IntcConfig; //GIC config
    Xil_ExceptionInit();
    //initialise the GIC

    IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

    XScuGic_CfgInitialize(GicInstancePtr, IntcConfig,

                    IntcConfig->CpuBaseAddress);

    //connect to the hardware

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,

                (Xil_ExceptionHandler)XScuGic_InterruptHandler,

                GicInstancePtr);

    //set up the timer interrupt

    XScuGic_Connect(GicInstancePtr, TimerIntrId,

                    (Xil_ExceptionHandler)TimerIntrHandler,

                    (void *)TimerInstancePtr);

    //enable the interrupt for the Timer at GIC

    XScuGic_Enable(GicInstancePtr, TimerIntrId);

    //enable interrupt on the timer

    XScuTimer_EnableInterrupt(TimerInstancePtr);

    // Enable interrupts in the Processor.

    Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);

    }

void TimerIntrHandler(void *CallBackRef) {

    static int sec = 0;
    XScuTimer *TimerInstancePtr = (XScuTimer *) CallBackRef;
    XScuTimer_ClearInterruptStatus(TimerInstancePtr);
    sec++;
    xil_printf(" %d Second\n\r",sec);

}
