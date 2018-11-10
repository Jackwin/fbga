`timescale 1ns/1ps
`include "param.h"
/*
ADC9826 works at SHA 2-channel mode, of which one is to capture the data from G11620, and the other is to
capture the voltage of thermistor resistor to acquire the temperature.
 Notes:
 1. The output data latency is 3 ADCCLK cycles.
 2. At the falling edge of CDSCLK2, each channel is sampled.
 3. R channel is G11620 input, G channel is thermistor input
 4. Config Reg( 9'b0-1100-1100): 4V input(D7 = 1), internal voltage reference(D6 = 1), 2CH or 1 CH(D5= 0) Enable SHA (D4 = 0)
    DC bias level (D3 = 1, SHA dianle the clamp function) D2 set power-donw. D0 set the 16-bit output.
 5. MUX CFG(9'b0-1100-0000): D8 = 0 D5-D0 = 0, D7 = 1 sample sequence is R-G-B, D6 = 1 select CH-R .
    At the beginning, only sample the SH-R
 6. RED PGA CFG: 9'b0  x1.0
 7. RED OFFSET CFG: 9'b0-0000-0000 =>  +0 mV


*/
module adc9826_cfg
(
    input           clk,
    input           rst_n,
    output          cfg_done_o,
    input           cfg_start_in,
    output          sclk,

     // Register interface
    input [8:0]     cfg_data_in,
    input           cfg_wr_in,
    input [2:0]     cfg_addr_in,
    input           cfg_rd_in,
    output reg [8:0]cfg_rd_data_o,
    output reg      cfg_rd_valid_o,

/*
    // control RAM
    output wire       cfg_ram_rd_o,
    output wire [7:0] cfg_ram_addr_o,
    input [31:0]     cfg_ram_din,
*/

    //inout           sdata_io,
    input           sdata_i,
    output          sdata_o,
    output          tri_en,
    output          sload_o

);

localparam IDLE_CFG = 4'd0,
            REG_CFG = 4'd1,
            MUX_CFG = 4'd2,
            RED_PGA_CFG = 4'd3,
            RED_OFFSET_CFG = 4'd4,
            REG_RD = 4'd5,
            MUX_RD = 4'd6,
            RED_PGA_RD = 4'd7,
            RED_OFFSET_RD = 4'd8;

localparam REG_DATA = 9'b011001000; // 0C8
localparam MUX_DATA = 9'b011000000; //0c0
localparam RED_PGA_DATA = 9'b0; //0
localparam RED_OFFSET_DATA = 9'b000000000;
localparam REG_ADDR = 3'd0;
localparam MUX_ADDR = 3'd1;
localparam RED_PGA_ADDR = 3'd2;
localparam RED_OFFSET_ADDR = 3'b101;

//configuration signals
wire        ready;
reg         rd_cmd;
wire [8:0]  rd_data;
wire        rd_data_valid;

reg         wr_cmd;
reg [8:0]   wr_data;
reg [7:0]   addr;
wire        done;

reg         cfg_start;
reg [3:0]   cfg_cs, cfg_ns;
reg         cfg_done;

reg [8:0]   ram[0:7];
wire [8:0]   ram_din;
wire         ram_wr;
wire [2:0]   ram_addr;
wire [8:0]  ram_dout;

// -------------------   Configure adc9826 -----------------------------
assign sclk = clk;
always @(posedge clk) begin
    cfg_start <= cfg_start_in;
end // always @(posedge clk)

always @(posedge clk) begin
    if (~rst_n) begin
        cfg_cs <= IDLE_CFG;
    end // if (~rst_n)
       else begin
        cfg_cs <= cfg_ns;
    end // else
end // always @(posedge clk)

always @(*) begin
    cfg_ns = cfg_cs;
    case(cfg_cs)
        IDLE_CFG:begin
            if (~cfg_start & cfg_start_in & ready) begin
                cfg_ns = REG_CFG;
            end // if (cfg_start)
        end // IDLE_CFG:
        REG_CFG: begin
            if (done) cfg_ns = MUX_CFG;
        end // REG_CFG:
        MUX_CFG: begin
            if (done) cfg_ns = RED_PGA_CFG;
        end // MUX_CFG:
        RED_PGA_CFG: begin
            if (done) cfg_ns = RED_OFFSET_CFG;
        end // RED_PGA_CFG:
        RED_OFFSET_CFG: begin
            if (done) cfg_ns = REG_RD;
        end // RED_OFFSET_CFG:
        REG_RD: begin
            if (rd_data_valid && rd_data == ram[`AD9826_REG_ADDR]) begin
           // if (rd_data == cfg_ram_din[8:0]) begin
                cfg_ns = MUX_RD;
            end // if (rd_data_valid && rd_data == REG_DATA)
        end // REG_RD:
        MUX_RD: begin
            if (rd_data_valid && rd_data == ram[`AD9826_MUX_ADDR]) begin
             //if (rd_data == cfg_ram_din[8:0]) begin
                cfg_ns = RED_PGA_RD;
            end
        end // MUX_RD:
        RED_PGA_RD: begin
            if (rd_data_valid && rd_data == ram[`AD9826_RED_PGA_ADDR]) begin
            //if (rd_data == cfg_ram_din[8:0]) begin
                cfg_ns = RED_OFFSET_RD;
            end
        end // MUX_RD:
        RED_OFFSET_RD: begin
            if (rd_data_valid && rd_data == ram[`AD9826_RED_OFFSET_ADDR]) begin
            //if (rd_data == cfg_ram_din[8:0]) begin
                cfg_ns = IDLE_CFG;
            end
        end
        default: begin
            cfg_ns = IDLE_CFG;
        end // default:
    endcase // cfg_cs
end

always @* begin
    wr_data = 'h0;
    addr = 'h0;
    wr_cmd = 1'b0;
    rd_cmd = 1'b0;
    cfg_done <= 1'b0;
    case(cfg_cs)
        IDLE_CFG: begin
            wr_data = 'h0;
            addr = 'h0;
           // ram_addr = 'h0;
            wr_cmd = 1'b0;
            rd_cmd = 1'b0;
        end // IDLE_CFG:
        REG_CFG: begin
            wr_cmd = ready;
            wr_data = ram[`AD9826_REG_ADDR];
            addr = `AD9826_REG_ADDR;
        end // REG_CFG:
        MUX_CFG: begin
            wr_cmd = ready;
            wr_data = ram[`AD9826_MUX_ADDR];
            addr = `AD9826_MUX_ADDR;
        end // MUX_CFG:
        RED_PGA_CFG: begin
            wr_cmd = ready;
            wr_data = ram[`AD9826_RED_PGA_ADDR];
            addr = `AD9826_RED_PGA_ADDR;
        end // RED_PGA_CFG:
        RED_OFFSET_CFG: begin
            wr_cmd = ready;
            wr_data = ram[`AD9826_RED_OFFSET_ADDR];
            addr = `AD9826_RED_OFFSET_ADDR;
        end // RED_OFFSET_CFG:
        REG_RD: begin
            rd_cmd = ready;
            addr = `AD9826_REG_ADDR;
        end // REG_RD:
        MUX_RD: begin
            rd_cmd = ready;
            addr = `AD9826_MUX_ADDR;
        end // MUX_RD:
        RED_PGA_RD: begin
            rd_cmd = ready;
            addr = `AD9826_RED_PGA_ADDR;
        end // RED_PGA_RD:
        RED_OFFSET_RD: begin
            rd_cmd = ready ;
            addr = `AD9826_RED_OFFSET_ADDR;
            cfg_done <= 1'b1;
        end // RED_OFFSET_RD:
        default: begin
            wr_cmd = 'h0;
            rd_cmd = 1'b0;
        end // default:
    endcase // cfg_cs
end
assign cfg_done_o = cfg_done;

serial_interface # (
    .DEBUG(0)
    )serial_interface_inst (
    .clk            (clk),
    .rst_n          (rst_n),

    .wr_in          (wr_cmd),
    .wr_data_in     (wr_data),
    .addr_in        (addr[2:0]),

    .rd_in          (rd_cmd),
    .rd_data_o      (rd_data),
    .rd_data_valid_o(rd_data_valid),
    .ready_o        (ready),
    .done_o         (done),

    //.sdata_io       (sdata_io),
    .sload_o        (sload_o),
    .sdata_i        (sdata_i),
    .sdata_o        (sdata_o),
    .tri_en         (tri_en)

    );

assign ram_din = cfg_data_in;
assign ram_wr = cfg_wr_in;
assign ram_addr = cfg_addr_in;
always @(posedge clk) begin
    if (ram_wr) ram[ram_addr] <= ram_din;
end

assign ram_dout = ram[ram_addr];
//assign cfg_ram_addr_o = addr;
//assign ram_dout = cfg_ram_din;
//assign wr_data = cfg_ram_din[8:0];
//assign cfg_ram_rd_o = (cfg_cs != IDLE_CFG);

always @(posedge clk) begin
    cfg_rd_data_o <= ram_dout;
    cfg_rd_valid_o <= cfg_rd_in;
end // always @(posedge clk)
/*
wire [0:0]      cfg_start_vio;
wire [0:0]      cfg_rd_vio;
wire [0:0]      sload_vio;
wire [0:0]      sdata_i_vio;
wire [0:0]      sdata_o_vio;
wire [0:0]      wr_cmd_vio;
wire [0:0]      rd_cmd_vio;
wire [0:0]      ready_vio;
wire [0:0]      tri_en_vio;
assign cfg_start_vio[0] = cfg_start;
assign cfg_rd_vio[0] = cfg_ram_rd_o;
assign sload_vio[0] = sload_o;
assign sdata_i_vio[0] = sdata_i;
assign sdata_o_vio[0] = sdata_o;
assign wr_cmd_vio[0] = wr_cmd;
assign rd_cmd_vio[0] = rd_cmd;
assign ready_vio[0] = ready;
assign tri_en_vio[0] = tri_en;

ila_ad9826 ila_ad9826_cfg_inst (
    .clk(clk), // input wire clk
    .probe0(cfg_cs), // input wire [3:0]  probe0
    .probe1(cfg_start_vio), // input wire [0:0]  probe1
    .probe2(rd_data), // input wire [8:0]  probe2
    .probe3(wr_cmd_vio), // input wire [0:0]  probe3
    .probe4(cfg_rd_vio), // input wire [0:0]  probe4
    .probe5(addr[2:0]), // input wire [2:0]  probe5
    .probe6(ready_vio), // input wire [0:0]  probe6
    .probe7(sload_vio), // input wire [0:0]  probe7
    .probe8(sdata_i_vio), // input wire [0:0]  probe8
    .probe9(sdata_o_vio), // input wire [0:0]  probe9
    .probe10(tri_en_vio)

);
*/
endmodule
