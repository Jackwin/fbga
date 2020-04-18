module zynq_top (
    input           clk50m_in,    // Clock
    input           rstn_in,
    output          bit_o,
    output          clk_o,
    output          bit_valid_o,
    //AD9826
    output          adc_clk_o,
    output          cdsclk2_o,
    input [7:0]     adc_data_in,
    output          adc_oeb, //active low

    output          sclk_o,
    inout           sdata_io,
    output          sload_o,
    //G11620
    output          g11620_reset_o,
    output          g11620_clk_o,
    output          g11620_cf_sel2_o,

    input           g11620_start_in, // from g11620
    output [2:0]    led,
    output[1:0]     gpio_led_0_tri_o

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

wire [0:0]  data_source_vio;
//--------------------------- LED --------------------------------
reg [25:0]  counter;
// -------------------------- ADC signals -------------------------
wire        clk_25m;
wire        srst_n_25m;
wire        clk_5m;
wire        srst_n_5m;
//wire        cfg_start;
wire        sdata_o;
wire        sdata_i;
wire        tri_en;
wire        adc_cfg_done;
wire        sclk;
wire        srst_n_sclk;
wire        adc_done;

wire        adc9826_cfg_ram_rd;
wire [7:0]  adc9826_cfg_ram_addr;
wire [31:0] adc9826_cfg_ram_din;
wire        adc9826_cfg_start;
// AD9826 config signals
/*
wire [0:0]  cfg_wr_vio;
wire [8:0]  cfg_data;
wire [2:0]  cfg_addr;
wire [0:0]  cfg_rd_valid_vio;
wire [8:0]  cfg_rd_data;
wire [0:0]  cfg_rd_vio;
*/
// -------------------------- G11620 signals --------------------
wire        g11620_start;
wire        g11620_soft_reset;
wire        g11620_done;
wire        g11620_cfg_ram_rd;
wire [7:0]  g11620_cfg_ram_addr;
wire [31:0] g11620_cfg_ram_din;
// -------------------------- Serializer signals --------------------
reg         ser_start, ser_start_r;
wire [0:0]  ser_start_vio;
wire [7:0]  ser_length;

wire [0:0]  ser_data_gen_vio;
wire [31:0] ser_data_gen;
wire        ser_data_gen_valid;
wire [7:0]  ser_data_gen_num;

// -------------------------- RAM signals ---------------------------
wire [31:0]  ad9826_ram_addr;
wire [31:0] ad_9826_ram_din, ad9826_ram_dout;
reg [8:0]   data_gen_ram_addr;

// ------------------------- PL Data RAM signals --------------------
// pldata_ram stores the ADC data from AD9826
wire        pldata_ram_clk, pldata_ram_rst;
reg         pldata_ram_wr;
reg [31:0]   pldata_ram_addr;
reg [31:0]  pldata_ram_din;

wire [0:0]  pldata_gen_vio;
wire [31:0] pldata_gen;
wire        pldata_gen_valid;
wire [7:0]  pldata_gen_num;
reg [9:0]   pldata_gen_addr;
// ------------------------- PL Data RAM signals --------------------
// plparam_ram stores the parameters from PS to config the G11620
wire            plparam_ram_clk;
wire            plparam_ram_rst;
wire [3:0]      plparam_ram_wr;
wire [31:0]     plparam_ram_din;
wire [31:0]     plparam_ram_dout;
wire [31:0]     plparam_ram_addr;

  clk_wiz clk_wiz_inst
   (
    .clk_200m(clk_200m),
    .clk_300m(clk_300m),
    .clk_37m5(clk_37m5),
    .reset(~rstn_in),
    .locked(locked),
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
// ------------------------- Reset bridge ------------------------
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

vio_reg vio_reg_inst (
  .clk(clk50m_in),              // input wire clk
  .probe_in0(locked)  // input wire [31 : 0] probe_in0
);

//-------------------------- LED ------------------------------------

wire        adc9826_cfg_autostart;
reg [15:0]  powerup_cnt;
reg         led_ena;

always @(posedge sclk) begin
    if (~srst_n_sclk) begin
        powerup_cnt <= 'h0;
    end else begin
        if (powerup_cnt == 16'hffff) begin
            powerup_cnt <= powerup_cnt;
        end else begin
            powerup_cnt <= powerup_cnt + 1'd1;
        end
    end
end
// Generate auto configuration signal for adc9826
assign adc9826_cfg_autostart = (powerup_cnt == 16'hfffe);


// After adc9826 configuration done, the leds start to shrink
always @(posedge clk_37m5) begin
    if (~srst_n_37m5) begin
        led_ena <= 1'b0;
    end else begin
        if (adc_cfg_done) begin
            led_ena <= 1'b1;
        end
    end
end

always @(posedge clk_37m5) begin
    if (~srst_n_37m5) begin
        counter <= 'h0;
    end
    else if (led_ena) begin
        counter <= counter + 1'b1;
    end // else
end
assign led = {3{counter[25]}};


// ------------------------- Serilalizer ---- ------------------------

vio_data_gen vio_data_gen_inst (
    .clk       (clk_37m5),
    .probe_out0(ser_data_gen_vio),
    .probe_out1(ser_data_gen_num)
    );
vio_data_gen vio_serializer_inst (
    .clk       (clk_37m5),
    .probe_out0(ser_start_vio),
    .probe_out1(ser_length)
    );

always @(posedge clk_37m5) begin
    if (~srst_n_37m5) begin
        ser_start_r <= 1'b0;
        ser_start <= 1'b0;
    end // if (~srst_n_300m)
    else begin
        ser_start_r <= ser_start_vio[0];
        ser_start <= ~ser_start_r & ser_start_vio[0];
    end // else
end // always @(posedge clk_300m)
data_gen ser_data_gen_inst (
    .clk             (clk_37m5),
    .rst_n           (srst_n_37m5),
    .data_gen_in     (ser_data_gen_vio[0]),
    .data_gen_num_in (ser_data_gen_num),
    .data_gen_o      (ser_data_gen),
    .data_gen_valid_o(ser_data_gen_valid)
    );

oserdes oserdes_inst (
    .wr_clk       (clk_37m5),
    .wr_rst       (~srst_n_37m5),
    .data_in      (ser_data_gen[7:0]),
    .data_valid_in(ser_data_gen_valid),
    .rd_clk       (clk_300m),
    .rd_rst       (~srst_n_300m),
    .start        (ser_start),
    .length_in    (ser_length),
    .bit_o        (bit_o),
    .clk_o        (),
    .valid_o      (bit_valid_o)
    );

// --------------- g11620 -----------------------------------
assign g11620_cf_sel2_o = 1'b1;
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

g11620 g11620_inst(
    .clk            (clk_5m),
    .rst_n          (srst_n_5m),

    .g11620_clk     (g11620_clk_o),
    .reset_o        (g11620_reset_o),
    .ad_sp          (g11620_start_in),

    .start_in       (g11620_start),
    .soft_reset_in  (g11620_soft_reset),
    .done_o         (g11620_done),
    .cfg_ram_rd_o   (g11620_cfg_ram_rd),
    .cfg_ram_addr_o (g11620_cfg_ram_addr),
    .cfg_ram_din    (g11620_cfg_ram_din)

    );
/*
vio_reg vio_reg_inst(
    .clk       (clk_5m),
    .probe_in0 (reg_rd_data),
    .probe_in1 (reg_rd_valid),
    .probe_out0(reg_wr_data),
    .probe_out1(reg_addr),
    .probe_out2(reg_wr),
    .probe_out3(reg_rd)
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
*/
// --------------- ad9826 logics -----------------------------
wire ad9826_start;
vio_data_gen ad9826_vio_inst (
    .clk       (clk_5m),
    .probe_out0(ad9826_start),
    .probe_out1()
    );

adc9826 adc9826_inst (
    .clk        (clk_5m),
    .rst_n      (srst_n_5m),
    .adc_clk_o  (adc_clk_o),
    .adc_data_in(adc_data_in),
    .adc_oeb    (adc_oeb),
    .cfg_done_in (adc_cfg_done),
    .adc_start_in(g11620_start_in | ad9826_start),
    .adc_done_out(adc_done),
    .ram_addr_o  (ad9826_ram_addr),
    .ram_data_o  (ad9826_ram_dout),
    .ram_wr_o    (ad9826_ram_wr)
    );


/*
vio_cfg_9826 vio_cfg_9826_inst (
  .clk(sclk),                // input wire clk
  .probe_in0(cfg_rd_valid_vio),    // input wire [0 : 0] probe_in0
  .probe_in1(cfg_rd_data),    // input wire [8 : 0] probe_in1
  .probe_out0(cfg_data),  // output wire [8 : 0] probe_out0
  .probe_out1(cfg_wr_vio),  // output wire [0 : 0] probe_out1
  .probe_out2(cfg_addr),  // output wire [2 : 0] probe_out2
  .probe_out3(cfg_rd_vio)  // output wire [0 : 0] probe_out3
);



vio_data_gen ad9826_cfg_vio_inst (
    .clk       (sclk),
    .probe_out0(cfg_start),
    .probe_out1()
    );
*/

reg [1:0]       cs;
reg             cfg_wr;
reg [2:0]       cfg_addr;
reg [8:0]       cfg_data;
reg             cfg_start;

localparam IDLE_s = 2'd0,
            S1_s = 2'd1,
            S2_s = 2'd2,
            S3_s = 2'd3;

ila_autocfg ila_autocfg_inst (
    .clk(sclk), // input wire clk     
    .probe0(cs), // input wire [1:0]  probe0  
    .probe1(cfg_wr), // input wire [0:0]  probe1 
    .probe2(cfg_addr), // input wire [2:0]  probe2 
    .probe3(cfg_data), // input wire [8:0]  probe3 
    .probe4(cfg_start), // input wire [0:0]  probe4 
    .probe5(adc_cfg_done), // input wire [0:0]  probe5 
    .probe6(adc9826_cfg_autostart), // input wire [0:0]  probe6 
    .probe7(powerup_cnt) // input wire [15:0]  probe7
);

always @(posedge sclk) begin
    if (~srst_n_sclk) begin
        cs = IDLE_s;
    end 
    else begin
        case(cs)
            IDLE_s: if (adc9826_cfg_autostart) cs <= S1_s;
            S1_s: cs <= S2_s;
            S2_s: cs <= S3_s;
            S3_s: cs <= IDLE_s;
            default: cs <= IDLE_s;
        endcase
    end
end

always @(*) begin
    cfg_start = 1'b0;
    cfg_addr = 'h0;
    cfg_wr = 'h0;
    cfg_data = 'h0;
    case(cs)
        IDLE_s: begin
            cfg_addr = 'h0;
            cfg_wr = 'h0;
            cfg_data = 'h0;
        end
        S1_s: begin
            cfg_addr = 'h0;
            cfg_data = 'h0c8;
            cfg_wr = 1'b1;
        end
        S2_s: begin
            cfg_addr = 'h1;
            cfg_data = 'h0c0;
            cfg_wr = 1'b1;
        end
        S3_s: begin
            cfg_start = 1'b1;
        end
        default: begin
            cfg_start = 1'b0;
            cfg_addr = 'h0;
            cfg_wr = 'h0;
            cfg_data = 'h0;
        end
    endcase
end


adc9826_cfg adc9826_cfg_inst (
    .clk         (sclk),
    .rst_n       (srst_n_sclk),
    .cfg_done_o  (adc_cfg_done),

    .cfg_data_in  (cfg_data),
    .cfg_wr_in    (cfg_wr),
    .cfg_addr_in  (cfg_addr),
    .cfg_rd_in    (cfg_rd_vio),
    .cfg_rd_data_o(),
    .cfg_rd_valid_o(),
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

// --------------- pldata and plparam RAM -----------------------------

vio_data_gen vio_mux_inst (
    .clk       (clk_5m),
    .probe_out0(data_source_vio),
    .probe_out1()
    );


vio_data_gen vio_pldata_gen_inst (
    .clk       (clk_5m),
    .probe_out0(pldata_gen_vio),
    .probe_out1(pldata_gen_num)
    );

data_gen pldata_gen_inst (
    .clk             (clk_5m),
    .rst_n           (srst_n_5m),
    .data_gen_in     (pldata_gen_vio[0]),
    .data_gen_num_in (pldata_gen_num),
    .data_gen_o      (pldata_gen),
    .data_gen_valid_o(pldata_gen_valid)
    );
/*
ila_data_gen ila_data_gen_inst (
    .clk(clk_5m), // input wire clk
    .probe0(pldata_ram_wr), // input wire [0:0]  probe0
    .probe1(pldata_ram_addr), // input wire [9:0]  probe1
    .probe2(pldata_ram_din) // input wire [31:0]  probe2
);
*/


always @(posedge clk_5m) begin
    if (~srst_n_5m) begin
        pldata_gen_addr <= 'h0;
    end // if (~srst_n_37m5)
    else begin
        if (pldata_gen_valid) pldata_gen_addr <= pldata_gen_addr + 3'd4;
    end // else
end // always @(posedge clk_37m5)


assign pldata_ram_clk = clk_5m;
assign pldata_ram_rst = ~srst_n_5m;
//assign pldata_ram_din = ad9826_ram_dout;
//assign pldata_ram_wr = ad9826_ram_wr;
//assign pldata_ram_addr = ad9826_ram_addr;

always@(*) begin
    pldata_ram_din = {ad9826_ram_dout[23:16], ad9826_ram_dout[31:24], ad9826_ram_dout[7:0], ad9826_ram_dout[15:8]};
    pldata_ram_wr = ad9826_ram_wr;
    pldata_ram_addr = ad9826_ram_addr;
    case(data_source_vio[0])
        1'b0: begin
            // Alig with the big-edian on PS
            pldata_ram_din = {ad9826_ram_dout[23:16], ad9826_ram_dout[31:24], ad9826_ram_dout[7:0], ad9826_ram_dout[15:8]};
            pldata_ram_wr = ad9826_ram_wr;
            pldata_ram_addr = ad9826_ram_addr;
        end // 1'b0:
        1'b1: begin
            pldata_ram_din = pldata_gen;
            pldata_ram_wr = pldata_gen_valid;
            pldata_ram_addr = {22'h0, pldata_gen_addr};
        end // 1'b1:
    endcase // data_source_vio[0]
end // always@(*)

assign plparam_ram_clk = clk_5m;
assign plparam_ram_rst = ~srst_n_5m;
assign plparam_ram_din = 'h0;
assign plparam_ram_wr = 4'b0;
//assign adc9826_cfg_ram_din = plparam_ram_dout;
assign g11620_cfg_ram_din = plparam_ram_dout;
assign plparam_ram_addr[31:8] = 'h0;
assign plparam_ram_addr[7:0] = g11620_cfg_ram_addr;


zynq_sys_wrapper zynq_system_wrapper_inst (
    .ps2pl_ctrl_tri_o    ({g11620_start, g11620_soft_reset, adc9826_cfg_start, gpio_led_0_tri_o}),
    .pldata_ram_addr  (pldata_ram_addr),
    .pldata_ram_clk   (pldata_ram_clk),
    .pldata_ram_din   (pldata_ram_din),
    .pldata_ram_dout  (),
    .pldata_ram_en    (1'b1),
    .pldata_ram_rst   (pldata_ram_rst),
    .pldata_ram_we    ({4{pldata_ram_wr}}),

    .plparam_ram_addr(plparam_ram_addr),
    .plparam_ram_clk (plparam_ram_clk),
    .plparam_ram_din (plparam_ram_din),
    .plparam_ram_en  (1'b1),
    .plparam_ram_rst (plparam_ram_rst),
    .plparam_ram_we  (plparam_ram_wr),
    .plparam_ram_dout(plparam_ram_dout),

    // The direct interrupt input
    .intr_in          (pldata_gen_vio),
   // .intr_in          (g11620_done),
    // The GPIO interrupt input
    .GPIO_in_tri_i    (g11620_done | pldata_gen_vio)


    );

endmodule