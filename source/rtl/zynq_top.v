module zynq_top (
    input       clk50m_in,    // Clock
    input       rstn_in,
    output      bit_o,
    output      clk_o,
    output      bit_valid_o,
    //AD9826
    output      adc_clk_o,
    output      cdsclk2_o,
    input [7:0] adc_data_in,
    output      adc_oeb, //active low

    output      sclk_o,
    inout       sdata_io,
    output      sload_o,
    //G11620
    output      g11620_reset_o,
    output      g11620_clk_o,
    output      g11620_cf_sel2_o,
    input       g11620_start_in, // from g11620

    output[1:0] gpio_led_0_tri_o
/*
    inout [14:0]    DDR_addr,
    inout [2:0]     DDR_ba,
    inout           DDR_cas_n,
    inout           DDR_ck_n,
    inout           DDR_ck_p,
    inout           DDR_cke,
    inout           DDR_cs_n,
    inout [3:0]     DDR_dm,
    inout [31:0]    DDR_dq,
    inout [3:0]     DDR_dqs_n,
    inout [3:0]     DDR_dqs_p,
    inout DDR_odt,
    inout DDR_ras_n,
    inout DDR_reset_n,
    inout DDR_we_n,

    inout FIXED_IO_ddr_vrn,
    inout FIXED_IO_ddr_vrp,
    inout [53:0]FIXED_IO_mio,
    inout FIXED_IO_ps_clk,
    inout FIXED_IO_ps_porb,
    inout FIXED_IO_ps_srstb
*/
);

wire        locked;
wire        clk_500m;
wire        clk_62m5;
wire        clk_200m;
wire        clk_37m5;
wire        adcclk;
wire        cdsclk2;
wire        rst_n_adc;
wire        srst_n_adc;

wire        srst_n;
wire        srst_n_300m;
wire        srst_n_37m5;
wire        srst_n_200m;

wire [0:0]  data_gen_vio;
wire [31:0] data_gen;
wire        data_gen_valid;
wire [7:0]  data_gen_num;

wire [0:0]  data_source_vio;
// -------------------------- ADC signals -------------------------
wire        clk_25m;
wire        srst_n_25m;
wire        clk_5m;
wire        srst_n_5m;
wire        cfg_start;
wire        sdata_o;
wire        sdata_i;
wire        tri_en;
wire        cfg_done;
wire        sclk;
wire        srst_n_sclk;
wire        adc_done;

wire        adc9826_cfg_ram_rd;
wire [7:0]  adc9826_cfg_ram_addr;
wire [31:0] adc9826_cfg_ram_din;
wire        adc9826_cfg_start;
// -------------------------- G11620 signals --------------------
wire        g11620_start;
wire        g11620_soft_reset;
wire        g11620_done;
wire        g11620_cfg_ram_rd;
wire [7:0]  g11620_cfg_ram_addr;
wire [31:0] g11620_cfg_ram_din;
// -------------------------- Serializer signals --------------------
reg         start, start_r;
wire [0:0]  start_vio;
wire [7:0]  length;

// -------------------------- RAM signals ---------------------------
wire [8:0]      ad9826_ram_addr;
wire [31:0]     ad_9826_ram_din, ad9826_ram_dout;
reg [8:0]       data_gen_ram_addr;

wire            ram_clk, ram_rst;
wire             ram_wr;
wire [8:0]       ram_addr;
wire [31:0]      ram_din;
/*
// RAM signals
wire            ram_clk;
wire            ram_rst;
(*mark_debug = "true"*)reg [12:0]      ram_addr;
(*mark_debug = "true"*)wire [31:0]      ram_din;
wire [31:0]      ram_dout;
wire [3:0]      ram_byte_ena;
(*mark_debug = "true"*)wire            ram_wr_ena;
wire [12:0]     ram_addr_vio;
*/
  clk_wiz clk_wiz_inst
   (
    // Clock out ports
    .clk_200m(clk_200m),     // output clk_200m
    .clk_300m(clk_300m),     // output clk_300m
    .clk_37m5(clk_37m5),
    // Status and control signals
    .reset(~rstn_in), // input reset
    .locked(locked),       // output locked
   // Clock in ports
    .clk_in1(clk50m_in));

clk_gen clk_gen_inst (
    .clk    (clk_200m),
    .rst_n  (srst_n_200m),
    .adcclk (adcclk),
    .cdsclk2(cdsclk2),
    .sclk     (sclk),
    .reset_n_o(rst_n_adc)
    );
assign cdsclk2_o = cdsclk2;

vio_data_gen vio_data_gen_inst (
    .clk       (clk_37m5),
    .probe_out0(data_gen_vio),
    .probe_out1(data_gen_num)
    );

vio_data_gen vio_serializer_inst (
    .clk       (clk_37m5),
    .probe_out0(start_vio),
    .probe_out1(length)
    );

vio_data_gen vio_mux_inst (
    .clk       (clk_37m5),
    .probe_out0(data_source_vio),
    .probe_out1()
    );

reset_bridge reset_bridge_200m_inst (
    .clk   (clk_200m),
    .arst_n(locked),
    .srst_n(srst_n_200m)
    );

reset_bridge reset_bridge_300m_inst (
    .clk   (clk_300m),
    .arst_n(locked),
    .srst_n(srst_n_300m)
    );

reset_bridge reset_bridge_sclk_inst (
    .clk   (sclk),
    .arst_n(rst_n_adc),
    .srst_n(srst_n_sclk)
    );

reset_bridge reset_bridge_adc_inst (
    .clk   (adcclk),
    .arst_n(rst_n_adc),
    .srst_n(srst_n_adc)
    );


reset_bridge reset_bridge_37m5_inst (
    .clk   (clk_37m5),
    .arst_n(locked),
    .srst_n(srst_n_37m5)
    );

always @(posedge clk_37m5) begin
    if (~srst_n_37m5) begin
        start_r <= 1'b0;
        start <= 1'b0;
    end // if (~srst_n_300m)
    else begin
        start_r <= start_vio[0];
        start <= ~start_r & start_vio[0];
    end // else
end // always @(posedge clk_300m)

data_gen data_gen_inst (
    .clk             (clk_37m5),
    .rst_n           (srst_n_37m5),
    .data_gen_in     (data_gen_vio[0]),
    .data_gen_num_in (data_gen_num),
    .data_gen_o      (data_gen),
    .data_gen_valid_o(data_gen_valid)
    );
/*
serializer serializer_inst (
    .wr_clk       (clk_200m),
    .wr_rst       (~srst_n),
    .data_in      (data_gen[15:0]),
    .data_valid_in(data_gen_valid),
    .rd_clk       (clk_300m),
    .rd_rst       (~srst_n_300m),
    .start        (start),
    .length_in    (length),
    .clk_o        (clk_o),
    .bit_o        (bit_o),
    .bit_valid_o  (bit_valid_o)
    );
*/
oserdes oserdes_inst (
    .wr_clk       (clk_37m5),
    .wr_rst       (~srst_n_37m5),
    .data_in      (data_gen[7:0]),
    .data_valid_in(data_gen_valid),
    .rd_clk       (clk_300m),
    .rd_rst       (~srst_n_300m),
    .start        (start),
    .length_in    (length),
    .bit_o        (bit_o),
    .clk_o        (clk_o),
    .valid_o      (bit_valid_o)
    );


// --------------- g11620 -----------------------------------
assign g11620_cf_sel2_o = 1'b0;
wire [0:0]      g11620_integ_start_vio;
reg             g11620_integ_start_p, g11620_integ_start_r;
wire [9:0]      g11620_integ_time;
wire [0:0]      g11620_set_integ_time;

wire [31:0]     reg_wr_data;
wire [3:0]      reg_addr;
wire [0:0]      reg_wr;
wire [0:0]      reg_rd;
wire [31:0]     reg_rd_data;
wire [0:0]      reg_rd_valid;

assign clk_5m = adcclk;
assign srst_n_5m = srst_n_adc;


vio_reg vio_reg_inst(
    .clk       (clk_5m),
    .probe_in0 (reg_rd_data),
    .probe_in1 (reg_rd_valid),
    .probe_out0(reg_wr_data),
    .probe_out1(reg_addr),
    .probe_out2(reg_wr),
    .probe_out3(reg_rd)
    );



g11620 g11620_inst(
    .clk       (clk_5m),
    .rst_n     (srst_n_5m),

    .start_in              (g11620_integ_start_p),
    .integ_time_in      (g11620_integ_time),
    .set_integ_time    (g11620_set_integ_time[0]),
    .g11620_clk(g11620_clk_o),
    .reset_o   (g11620_reset_o),
    .ad_sp     (g11620_start_in),
/*
    .start_in      (g11620_start),
    .soft_reset_in (g11620_soft_reset),
    .done_o        (g11620_done),
    .cfg_ram_rd_o  (g11620_cfg_ram_rd),
    .cfg_ram_addr_o(g11620_cfg_ram_addr),
    .cfg_ram_din   (g11620_cfg_ram_din),
*/
    .data_in   (reg_wr_data),
    .wr_in     (reg_wr[0]),
    .addr_in   (reg_addr),
    .rd_in     (reg_rd[0]),
    .rd_data_o (reg_rd_data),
    .rd_valid_o(reg_rd_valid[0])
    );

vio_g11620 vio_g11620_inst (
  .clk(clk_5m),                // input wire clk
  .probe_out0(g11620_integ_start_vio),  // output wire [0 : 0] probe_out0
  .probe_out1(g11620_set_integ_time),  // output wire [0 : 0] probe_out1
  .probe_out2(g11620_integ_time)  // output wire [9 : 0] probe_out2
);

always @(posedge clk_5m) begin
    g11620_integ_start_r <= g11620_integ_start_vio[0];
    g11620_integ_start_p <= ~g11620_integ_start_r & g11620_integ_start_vio[0];
end // always @(posedge clk_5m)

// --------------- ad9826 logics -----------------------------

adc9826 adc9826_inst (
    .clk        (clk_5m),
    .rst_n      (srst_n_5m),
    .adc_clk_o  (adc_clk_o),
    .adc_data_in(adc_data_in),
    .adc_oeb    (adc_oeb),
    .cfg_done_in(cfg_done),
    .adc_start_in(g11620_start_in),
    .adc_done_out(adc_done),
    .ram_addr_o  (ad9826_ram_addr),
    .ram_data_o  (ad9826_ram_dout),
    .ram_wr_o    (ad9826_ram_wr)
    );


wire [0:0]  cfg_wr_vio;
wire [8:0]  cfg_data;
wire [2:0]  cfg_addr;
wire [0:0]  cfg_rd_valid_vio;
wire [8:0]  cfg_rd_data;
wire [0:0]  cfg_rd_vio;

vio_data_gen ad9826_cfg_vio_inst (
    .clk       (sclk),
    .probe_out0(cfg_start),
    .probe_out1()
    );

vio_cfg_9826 vio_cfg_9826_inst (
  .clk(sclk),                // input wire clk
  .probe_in0(cfg_rd_valid_vio),    // input wire [0 : 0] probe_in0
  .probe_in1(cfg_rd_data),    // input wire [8 : 0] probe_in1
  .probe_out0(cfg_data),  // output wire [8 : 0] probe_out0
  .probe_out1(cfg_wr_vio),  // output wire [0 : 0] probe_out1
  .probe_out2(cfg_addr),  // output wire [2 : 0] probe_out2
  .probe_out3(cfg_rd_vio)  // output wire [0 : 0] probe_out3
);


adc9826_cfg adc9826_cfg_inst (
    .clk         (sclk),
    .rst_n       (srst_n_sclk),
    .cfg_done_o  (cfg_done),

    .cfg_data_in  (cfg_data),
    .cfg_wr_in    (cfg_wr_vio[0]),
    .cfg_addr_in  (cfg_addr),
    .cfg_rd_in    (cfg_rd_vio[0]),
    .cfg_rd_data_o(cfg_rd_data),
    .cfg_rd_valid_o(cfg_rd_valid_vio[0]),
/*
    .cfg_ram_rd_o  (adc9826_cfg_ram_rd),
    .cfg_ram_addr_o(adc9826_cfg_ram_addr),
    .cfg_ram_din   (adc9826_cfg_ram_din),
*/
    .cfg_start_in(cfg_start | adc9826_cfg_start),
    .sclk        (sclk_o),
    .sload_o     (sload_o),
    .sdata_i     (sdata_i),
    .sdata_o     (sdata_o),
    .tri_en      (tri_en)
    );
assign sdata_io = ~tri_en ? sdata_o : 1'bz;
assign sdata_i = sdata_io;

// -------------- Write BRAM logics --------------------------

/*
assign ram_clk = clk_200m;
assign ram_rst = ~srst_n;
assign ram_din = data_gen;
assign ram_wr_ena = data_gen_valid;

always @(posedge clk_200m) begin
    if (ram_rst) begin
        ram_addr <= 'h0;
    end // if (ram_rst)
    else begin
        if (data_gen_valid) ram_addr <= ram_addr + 1'b1;
        else ram_addr <= ram_addr_vio;
    end // else
end // always @(posedge clk_200m)

wire [0:0]      ila_ram_locked;
wire [0:0]      ila_ram_data_gen_valid;
assign ila_ram_locked[0] = locked;
assign ila_ram_data_gen_valid[0] = data_gen_valid;
ila_0 ila_ram (
    .clk(clk_200m), // input wire clk
    .probe0(ila_ram_locked), // input wire [0:0]  probe0
    .probe1(ram_din), // input wire [31:0]  probe1
    .probe2(ram_addr), // input wire [12:0]  probe2
    .probe3(ila_ram_data_gen_valid) // input wire [0:0]  probe3
);
*/

//------------------------------------------------------------

always @(posedge clk_37m5) begin
    if (~srst_n_37m5) begin
        data_gen_ram_addr <= 'h0;
    end // if (~srst_n_37m5)
    else begin
        if (data_gen_valid) data_gen_ram_addr <= data_gen_ram_addr + 1'd1;
    end // else
end // always @(posedge clk_37m5)

//assign ram_clk = clk_37m5;
//assign ram_rst = ~srst_n_37m5;
assign ram_clk = clk_5m;
assign ram_rst = ~srst_n_5m;
assign ram_din = ad9826_ram_dout;
assign ram_wr = ad9826_ram_wr;
assign ram_addr = ad9826_ram_addr;
/*
always@(*) begin
    ram_din = ad9826_ram_dout;
    ram_wr = ad9826_ram_wr;
    ram_addr = ad9826_ram_addr;
    case(data_source_vio[0])
        1'b0: begin
            ram_din = ad9826_ram_dout;
            ram_wr = ad9826_ram_wr;
            ram_addr = ad9826_ram_addr;
        end // 1'b0:
        1'b1: begin
            ram_din = data_gen;
            ram_wr = data_gen_valid;
            ram_addr = data_gen_ram_addr;
        end // 1'b1:
    endcase // data_source_vio[0]
end // always@(*)

*/
wire            plparam_ram_clk;
wire            plparam_ram_rst;
wire            plparam_ram_wr;
wire [31:0]     plparam_ram_din;
wire [31:0]     plparam_ram_dout;
reg [31:0]     plparam_ram_addr;

assign plparam_ram_clk = clk_5m;
assign plparam_ram_rst = ~srst_n_5m;
assign plparam_ram_din = 'h0;
assign plparam_ram_wr = 1'b0;
assign adc9826_cfg_ram_din = plparam_ram_dout;
assign g11620_cfg_ram_din = plparam_ram_dout;
always@(*) begin
    case({adc9826_cfg_ram_rd, g11620_cfg_ram_rd})
        2'b00: plparam_ram_addr = 'h0;
        2'b01: plparam_ram_addr = adc9826_cfg_ram_addr;
        2'b10: plparam_ram_addr = g11620_cfg_ram_addr;
        2'b11: plparam_ram_addr = g11620_cfg_ram_addr;
        default: plparam_ram_addr = 'h0;
    endcase
end // always@(*)


zynq_sys_wrapper zynq_system_wrapper_inst (
    .ps2pl_ctrl_tri_o    ({g11620_start, g11620_soft_reset, adc9826_cfg_start, gpio_led_0_tri_o}),
    .pldata_ram_addr  ({23'd0,ram_addr}),
    .pldata_ram_clk   (ram_clk),
    .pldata_ram_din   (ram_din),
    .pldata_ram_dout  (),
    .pldata_ram_en    (1'b1),
    .pldata_ram_rst   (ram_rst),
    .pldata_ram_we    (ram_wr),
    .plparam_ram_addr(plparam_ram_addr),
    .plparam_ram_clk (plparam_ram_clk),
    .plparam_ram_din (plparam_ram_din),
    .plparam_ram_en  (1'b1),
    .plparam_ram_rst (plparam_ram_rst),
    .plparam_ram_we  (plparam_ram_wr),
    .plparam_ram_dout(plparam_ram_dout),

    .intr_in          (adc_done),
    .GPIO_in_tri_i    (g11620_done)


    );

endmodule