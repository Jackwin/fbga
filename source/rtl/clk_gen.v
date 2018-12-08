`timescale 1ns/1ps

module clk_gen (
    input       clk,  // input clock is 200MHz
    input       rst_n,

    output reg  adcclk, // output is 5 MHz
    output      reset_n_o,
    output reg  sclk,
    output reg  cdsclk2
);
reg [19:0]  reset_cnt;
reg         reset_n;
reg [5:0]   clk_cnt;
reg [6:0]   clk_cnt2;
wire        adcclk_i;
wire        cdsclk2_i;
wire        sclk_i;

assign reset_n_o = reset_n;
always @(posedge clk) begin
    if (~rst_n) begin
        reset_n <= 1'b0;
    end // if (~rst_n)
    else begin
        reset_cnt <= reset_cnt + 1'd1;
        if (reset_cnt == 20'hfffff) begin
            reset_n <= 1'b1;
            reset_cnt <= reset_cnt;
        end // if (reset_cnt == 20'hfffff)
    end // else
end // always @(posedge clk)


always @(posedge clk) begin
    if (~rst_n) begin
        clk_cnt <= 'h0;
        clk_cnt2 <= 'h0;
    end // if (~rst_n)
    else begin
        clk_cnt <= clk_cnt + 1'd1;
        clk_cnt2 <= clk_cnt2 + 1'd1;
        if (clk_cnt == 6'd39) begin
            clk_cnt <= 'h0;
        end // if (clk_cnt == 5'd19)

        if (clk_cnt2 == 7'd119) begin
            clk_cnt2 <= 'h0;
        end // if (clk_cnt2 == 6'd59)
    end // else
end // always @(posedge clk)
assign adcclk_i = (clk_cnt < 6'd20) ? 1'b0 : 1'b1;
assign cdsclk2_i = (clk_cnt > 6'd0 && clk_cnt < 6'd9) ? 1'b1 : 1'b0;
assign sclk_i = (clk_cnt2 < 7'd60) ? 1'b0 : 1'b1;
always @(posedge clk) begin
    adcclk <= adcclk_i;
    cdsclk2 <= cdsclk2_i;
    sclk <= sclk_i;
end // always @(posedge clk)

//-------------------------------------------
wire [0:0]      adcclk_ila;
wire [0:0]      cdsclk2_ila;
wire [0:0]      reset_n_ila;
wire [0:0]      sclk_ila;
assign adcclk_ila[0] = adcclk_i;
assign cdsclk2_ila[0] = cdsclk2_i;
assign sclk_ila[0] = reset_n;
ila_clk_gen ila_clk_gen_inst (
    .clk(clk), // input wire clk
    .probe0(clk_cnt), // input wire [5:0]  probe0
    .probe1(adcclk_ila), // input wire [0:0]  probe1
    .probe2(cdsclk2_ila), // input wire [0:0]  probe2
    .probe3(sclk_ila) // input wire [0:0]  probe3
);


endmodule