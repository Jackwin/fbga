#include "gpio_intr.h"
#include "cdma.h"

XGpio Gpio1Output;
 void IntcTypeSetup(XScuGic *InstancePtr, u32 intId, u32 intType)

{
    int mask;
    intType &= INT_TYPE_MASK;
    mask = XScuGic_DistReadReg(InstancePtr, INT_CFG0_OFFSET + (intId/16)*4);
    xil_printf("Mask is %x\n\r", mask);
    mask &= ~(INT_TYPE_MASK << (intId%16)*2);
    mask |= intType << ((intId%16)*2);
    xil_printf("Mask is %x\n\r", mask);
    XScuGic_DistWriteReg(InstancePtr, INT_CFG0_OFFSET + (intId/16)*4, mask);

}
 int GpioIntrInit(XScuGic *intc, XGpio *GpioInstancePtr) {
    int status;
    XScuGic_Config *IntcConfig;
    IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
    status = XScuGic_CfgInitialize(intc, IntcConfig, IntcConfig->CpuBaseAddress);
    if(status != XST_SUCCESS) return XST_FAILURE;
	// Call to interrupt setup
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
	                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
	                                 intc);

	Xil_ExceptionEnable();

    status = XScuGic_Connect(intc, XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR, (Xil_ExceptionHandler)GpioIntrHandler,
                         (void *)GpioInstancePtr);
    if(status != XST_SUCCESS) return XST_FAILURE;
    IntcTypeSetup(intc, XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR, INT_TYPE_RISING_EDGE );
    XGpio_InterruptEnable(GpioInstancePtr, 1);
    XGpio_InterruptGlobalEnable(GpioInstancePtr);
    XScuGic_Enable(intc, XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR);
    return XST_SUCCESS;
}

 int PLIntrInit(XScuGic intc) {
    int status;
    XScuGic_Config *IntcConfig;
    IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
    status = XScuGic_CfgInitialize(&intc, IntcConfig, IntcConfig->CpuBaseAddress);
    if(status != XST_SUCCESS) return XST_FAILURE;
    // Call to interrupt setup
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                     &intc);

    Xil_ExceptionEnable();

    status = XScuGic_Connect(&intc, PL_INT_ID, (Xil_ExceptionHandler)PLIntrHandler,
                         (void *)1);
    if(status != XST_SUCCESS) return XST_FAILURE;
    IntcTypeSetup(&intc, PL_INT_ID, INT_TYPE_RISING_EDGE);
    XScuGic_Enable(&intc, PL_INT_ID);
    return XST_SUCCESS;
}


 void GpioIntrHandler(void *param)
{
    xil_printf(" GPIO interrupt.\n\r");
    XGpio_InterruptDisable(&Gpio1Output, XGPIO_IR_CH1_MASK);
    // Acknowledge GPIO interruptsL
   (void) XGpio_InterruptClear(&Gpio1Output, XGPIO_IR_CH1_MASK);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&Gpio1Output, XGPIO_IR_CH1_MASK);

}

 // PL interrupt function corresponding to the signal intr_in in zynq_sys_wrapper
 void PLIntrHandler(void *param)
{
    xil_printf("PL interrupt.\n\r");
    cdma_test();

}
