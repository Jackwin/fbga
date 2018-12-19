#ifndef GPIO_INTR_H
#define GPIO_INTR_H

#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_printf.h"
#include "xgpio.h"

#define INT_TYPE_RISING_EDGE    0x03
#define INT_TYPE_HIGHLEVEL      0x01
#define INT_TYPE_MASK           0x03
#define INT_CFG0_OFFSET         0x00000C00
#define PL_INT_ID               XPS_FPGA1_INT_ID


 void IntcTypeSetup(XScuGic *InstancePtr, u32 intId, u32 intType);
 int GpioIntrInit(XScuGic *intc, XGpio *GpioInstancePtr);
 int PLIntrInit(XScuGic intc);
 void PLIntrHandler(void *param);
 void GpioIntrHandler(void *param);



#endif
