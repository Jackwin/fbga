#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xgpio.h"
#include "sleep.h"
#include "xaxicdma.h"
#include "xil_cache.h"
#include "xscutimer.h"
#include "xil_exception.h"
#include "timer.h"
#include "dma_intr.h"
#include "gpio_intr.h"
#include "cdma.h"
#include "pl_param.h"
#include "main.h"
#include "g11620.h"

XGpio intr_gpio;
/* Instance For GPIO */
XGpio  ps_ctrl_gpio;
XAxiCdma_Config *axi_cdma_cfg;
XScuGic intc;
int main() {

//    XScuTimer timer;
  //  XAxiCdma axi_cdma;
    int status;

  //  u8 ps_ctrl_gpio_val = 0;

    init_platform();

    // Initialize GPIO
    XGpio_Initialize(&ps_ctrl_gpio, XPAR_PS2PL_GPIO_DEVICE_ID); //initialize GPIO IP
    XGpio_SetDataDirection(&ps_ctrl_gpio, 1, 0x0);            	//set GPIO as output
    // Configure the AD9826. Move the Ad9826 configuration to PL
    //*(pl_param_buffer + AD9826_REG_ADDR) = 0xc8;
    //*(pl_param_buffer + AD9826_MUX_ADDR) = 0xfff0; // 8f0
    // ps_ctrl_gpio_val = ps_ctrl_gpio_val | AD9826_CFG_START;
    // XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val);  //Rising edge of AD9826_CFG_START triggers the ad9826 configuration
    // ps_ctrl_gpio_val = ps_ctrl_gpio_val & ~AD9826_CFG_START;
    // XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val);

     // Configure the G11620 space
/*
     u32 *pl_param_buffer = ( u32*)PL_PARAM_RAM_ADDR;
     *(pl_param_buffer + G11620_INTEG_R_ADDR) = 0xfff0;
     ps_ctrl_gpio_val = ps_ctrl_gpio_val | G11620_START;
     XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val); //assert
     ps_ctrl_gpio_val = ps_ctrl_gpio_val & ~G11620_START;
     XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val); // de-assert

*/
    // GPIO interrupt and wil be used as the indicator of G11620 operation done.

    status = XGpio_Initialize(&intr_gpio, XPAR_AXI_GPIO_0_DEVICE_ID); //initialize GPIO IP
    if(status != XST_SUCCESS) return XST_FAILURE;
    XGpio_SetDataDirection(&intr_gpio, 1, 0xFFFFFFFF);
    GpioIntrInit(&intc, &intr_gpio);


    // Direct PL interrupt. Will trig the DMA operation moving data from PL ram to DDR3 on PS side
    // PL intr cannot work well with G11620ConsecInteg function. It's weird!!
  // PLIntrInit(intc);

    u16 integ_time = 10;
    u16 consec_times = 1000;
   G11620ConsecInteg(integ_time, consec_times);

    while(1){

        	for (u32 Ledwidth = 0x0; Ledwidth < 2; Ledwidth++) {
        		XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, 1 << Ledwidth);
        		usleep(1000 * 1000); //sleep 1s
        		XGpio_DiscreteClear(&ps_ctrl_gpio, 1, 1 << Ledwidth);
        	}

    }


    return 0;

}
