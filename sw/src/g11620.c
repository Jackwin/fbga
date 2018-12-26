/*
 * g11620.c
 *
 *  Created on: Dec 20, 2018
 *      Author: chunjiew
 */
#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xgpio.h"
#include "sleep.h"
#include "main.h"
#include "pl_param.h"

extern XGpio ps_ctrl_gpio;
extern u16 dma_done;
// The times is in the unit of us. Integration time ranges from 3 us to 63535 us
u16 G11620SetIntegTimes(u16 integ_time) {

    if (integ_time < 3 || integ_time > 65535) {
        printf("The integration time is not valid\n");
        return 0;
    }
    u32 clk_cycle_cnt = integ_time / 0.2; // the clock cycle is 200 ns in the PL

    volatile u32 *pl_param_buffer = ( u32*)PL_PARAM_RAM_ADDR;
    *(pl_param_buffer + G11620_INTEG_R_ADDR) = clk_cycle_cnt;

    if (*(pl_param_buffer + G11620_INTEG_R_ADDR) == clk_cycle_cnt)
        return 1;
    else
        return 0;

}

void G11620Start(void) {

    u8 ps_ctrl_gpio_val = 0;
    ps_ctrl_gpio_val = ps_ctrl_gpio_val | G11620_START;
    XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val); //assert
    ps_ctrl_gpio_val = ps_ctrl_gpio_val & ~G11620_START;
    XGpio_DiscreteWrite(&ps_ctrl_gpio, 1, ps_ctrl_gpio_val); // de-assert
}

// Consecutive Integration
u16 G11620ConsecInteg(u16 integ_time, u16 consec_times) {
    u16 cnt = consec_times;
    u16 status;
    status =  G11620SetIntegTimes(integ_time);
    if (status == 0) xil_printf("Fail to set integ time.\n");

    while(cnt > 0) {
        G11620Start();
        while(dma_done == 0);
        dma_done = 0;
        cnt--;
    }

    xil_printf("All is done.!!!!\n");

    return 1;

}
