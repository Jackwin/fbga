/*
 * pl_param.h
 *
 *  Created on: Oct 28, 2018
 *      Author: chunjiew
 */

#ifndef PL_PARAM_H_
#define PL_PARAM_H_

#define AD9826_REG_ADDR 0
#define AD9826_MUX_ADDR 1
#define AD9826_RED_PGA_ADDR  2
#define AD9826_GREEN_PGA_ADDR 3
#define AD9826_BLUE_PGA_ADDR 4
#define AD9826_RED_OFFSET_ADDR 5
#define AD9826_GREEN_OFFSET_ADDR 6
#define AD9826_BLUE_OFFSET_ADDR 7

#define G11620_CTRL_R_ADDR 0
#define G11620_INTEG_R_ADDR 1
#define G11620_STP_R_ADDR 2
#define G11620_PW_R_ADDR 3
#define G11620_SCAN_R_ADDR 4
#define G11620_SP_R_ADDR 5
#define G11620_EP_R_ADDR 6

#define AD9826_CFG_START 1 << 2
#define G11620_SOFT_START 1 << 3
#define G11620_START 1 << 4

#endif /* PL_PARAM_H_ */
