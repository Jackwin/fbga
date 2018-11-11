//Copyright 1986-2017 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2017.4 (win64) Build 2086221 Fri Dec 15 20:55:39 MST 2017
//Date        : Sat Nov  3 10:07:45 2018
//Host        : chunjiew-MOBL running 64-bit major release  (build 9200)
//Command     : generate_target zynq_sys_wrapper.bd
//Design      : zynq_sys_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module zynq_sys_wrapper
   (DDR_addr,
    DDR_ba,
    DDR_cas_n,
    DDR_ck_n,
    DDR_ck_p,
    DDR_cke,
    DDR_cs_n,
    DDR_dm,
    DDR_dq,
    DDR_dqs_n,
    DDR_dqs_p,
    DDR_odt,
    DDR_ras_n,
    DDR_reset_n,
    DDR_we_n,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ddr_vrp,
    FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,
    GPIO_in_tri_i,
    intr_in,
    pldata_ram_addr,
    pldata_ram_clk,
    pldata_ram_din,
    pldata_ram_dout,
    pldata_ram_en,
    pldata_ram_rst,
    pldata_ram_we,
    plparam_ram_addr,
    plparam_ram_clk,
    plparam_ram_din,
    plparam_ram_dout,
    plparam_ram_en,
    plparam_ram_rst,
    plparam_ram_we,
    ps2pl_ctrl_tri_o);
  inout [14:0]DDR_addr;
  inout [2:0]DDR_ba;
  inout DDR_cas_n;
  inout DDR_ck_n;
  inout DDR_ck_p;
  inout DDR_cke;
  inout DDR_cs_n;
  inout [3:0]DDR_dm;
  inout [31:0]DDR_dq;
  inout [3:0]DDR_dqs_n;
  inout [3:0]DDR_dqs_p;
  inout DDR_odt;
  inout DDR_ras_n;
  inout DDR_reset_n;
  inout DDR_we_n;
  inout FIXED_IO_ddr_vrn;
  inout FIXED_IO_ddr_vrp;
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_clk;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ps_srstb;
  input [0:0]GPIO_in_tri_i;
  input [0:0]intr_in;
  input [31:0]pldata_ram_addr;
  input pldata_ram_clk;
  input [31:0]pldata_ram_din;
  output [31:0]pldata_ram_dout;
  input pldata_ram_en;
  input pldata_ram_rst;
  input [3:0]pldata_ram_we;
  input [31:0]plparam_ram_addr;
  input plparam_ram_clk;
  input [31:0]plparam_ram_din;
  output [31:0]plparam_ram_dout;
  input plparam_ram_en;
  input plparam_ram_rst;
  input [3:0]plparam_ram_we;
  output [31:0]ps2pl_ctrl_tri_o;

  wire [14:0]DDR_addr;
  wire [2:0]DDR_ba;
  wire DDR_cas_n;
  wire DDR_ck_n;
  wire DDR_ck_p;
  wire DDR_cke;
  wire DDR_cs_n;
  wire [3:0]DDR_dm;
  wire [31:0]DDR_dq;
  wire [3:0]DDR_dqs_n;
  wire [3:0]DDR_dqs_p;
  wire DDR_odt;
  wire DDR_ras_n;
  wire DDR_reset_n;
  wire DDR_we_n;
  wire FIXED_IO_ddr_vrn;
  wire FIXED_IO_ddr_vrp;
  wire [53:0]FIXED_IO_mio;
  wire FIXED_IO_ps_clk;
  wire FIXED_IO_ps_porb;
  wire FIXED_IO_ps_srstb;
  wire [0:0]GPIO_in_tri_i;
  wire [0:0]intr_in;
  wire [31:0]pldata_ram_addr;
  wire pldata_ram_clk;
  wire [31:0]pldata_ram_din;
  wire [31:0]pldata_ram_dout;
  wire pldata_ram_en;
  wire pldata_ram_rst;
  wire [3:0]pldata_ram_we;
  wire [31:0]plparam_ram_addr;
  wire plparam_ram_clk;
  wire [31:0]plparam_ram_din;
  wire [31:0]plparam_ram_dout;
  wire plparam_ram_en;
  wire plparam_ram_rst;
  wire [3:0]plparam_ram_we;
  wire [31:0]ps2pl_ctrl_tri_o;

  zynq_sys zynq_sys_i
       (.DDR_addr(DDR_addr),
        .DDR_ba(DDR_ba),
        .DDR_cas_n(DDR_cas_n),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cke(DDR_cke),
        .DDR_cs_n(DDR_cs_n),
        .DDR_dm(DDR_dm),
        .DDR_dq(DDR_dq),
        .DDR_dqs_n(DDR_dqs_n),
        .DDR_dqs_p(DDR_dqs_p),
        .DDR_odt(DDR_odt),
        .DDR_ras_n(DDR_ras_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_we_n(DDR_we_n),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .GPIO_in_tri_i(GPIO_in_tri_i),
        .intr_in(intr_in),
        .pldata_ram_addr(pldata_ram_addr),
        .pldata_ram_clk(pldata_ram_clk),
        .pldata_ram_din(pldata_ram_din),
        .pldata_ram_dout(pldata_ram_dout),
        .pldata_ram_en(pldata_ram_en),
        .pldata_ram_rst(pldata_ram_rst),
        .pldata_ram_we(pldata_ram_we),
        .plparam_ram_addr(plparam_ram_addr),
        .plparam_ram_clk(plparam_ram_clk),
        .plparam_ram_din(plparam_ram_din),
        .plparam_ram_dout(plparam_ram_dout),
        .plparam_ram_en(plparam_ram_en),
        .plparam_ram_rst(plparam_ram_rst),
        .plparam_ram_we(plparam_ram_we),
        .ps2pl_ctrl_tri_o(ps2pl_ctrl_tri_o));
endmodule
