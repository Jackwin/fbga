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

#define TIMER_LOAD_VALUE    0xFFFFFFFF
#define CDMA_RAM_ADDR 0xC2000000
#define PLDATA_RAM_ADDR 0xc4000000

#define PL_PARAM_RAM_ADDR 0x40000000
#define GP_RAM_ADDR 0x42000000
#define PS_DDR_ADDR 0x10000000

#define TOTAL_NUM 64
#define BUF_LEN (TOTAL_NUM * 4) // Byte

/* Instance For GPIO */
XGpio GpioOutput, ps_ctrl_gpio;
XAxiCdma_Config *axi_cdma_cfg;
XScuGic intc;
int main() {

    XScuTimer timer;
    XAxiCdma axi_cdma;
    int status;
    u32 *pl_param_buffer = ( u32*)PL_PARAM_RAM_ADDR;
    u8 ps_ctrl_gpio_val = 0;

    init_platform();

    // Initialize GPIO
    XGpio_Initialize(&ps_ctrl_gpio, XPAR_PS2PL_GPIO_DEVICE_ID); //initialize GPIO IP
    XGpio_SetDataDirection(&ps_ctrl_gpio, 1, 0x0);            	//set GPIO as output
    // Configure the AD9826
    *(pl_param_buffer + AD9826_REG_ADDR) = 0xc8;
    *(pl_param_buffer + AD9826_MUX_ADDR) = 0xc0;
     ps_ctrl_gpio_val = ps_ctrl_gpio_val | AD9826_CFG_START;
     XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val);  //Rising edge of AD9826_CFG_START triggers the ad9826 configuration
     ps_ctrl_gpio_val = ps_ctrl_gpio_val & ~AD9826_CFG_START;
     XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val);

     // Configure the G11620 space
     *(pl_param_buffer + G11620_INTEG_R_ADDR) = 0x100;
     ps_ctrl_gpio_val = ps_ctrl_gpio_val | G11620_START;
     XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val); //assert
     ps_ctrl_gpio_val = ps_ctrl_gpio_val & ~G11620_START;
     XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val); // de-assert


   // XScuTimer_Config *timer_config_ptr;
   // u32 timer_load_value = 0x13D92D3F;
   // u32 timer_device_id = XPAR_XSCUTIMER_0_DEVICE_ID;
    //TimerInit(intc, timer, timer_load_value, timer_device_id);

    // PL interrupt
/*
    status = XGpio_Initialize(&Gpio1Output, XPAR_AXI_GPIO_0_DEVICE_ID); //initialize GPIO IP
    if(status != XST_SUCCESS) return XST_FAILURE;
    XGpio_SetDataDirection(&Gpio1Output, 1, 0xFFFFFFFF);
    GpioIntrInit(&intc, &Gpio1Output);
*/
    //PLIntrInit(intc);


   // XAxiCdma_SetupIntr(&intc, &axi_cdma,
    //		XPAR_AXICDMA_0_DEVICE_ID, XPAR_FABRIC_AXICDMA_0_VEC_ID);
     //cdma_test();


    while(1){
    	/*
    	for (u32 Ledwidth = 0x0; Ledwidth < 2; Ledwidth++) {
    		XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, 1 << Ledwidth);
    		usleep(1000 * 1000); //sleep 1s
    		XGpio_DiscreteClear(&ps_ctrl_gpio, 1, 1 << Ledwidth);
    	}
    	*/
    }
    return 0;

}
