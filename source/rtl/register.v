`timescale 1ns/1ps

module register (
    input           clk,    // Clock
    input           rst_n,  // Asynchronous reset active low

    input [31:0]    data_in,
    input [3:0]     addr_in,
    input           wr_in,

    input           rd_in,
    output [31:0]   data_out,
    output reg      data_valid_out
);

wire    [31:0]      ram_dout;
ram # (
    .RAM_DEPTH(16),
    .RAM_DATA_WIDTH(32)
    )reg_ram(
    .clk (clk),
    .din(data_in),
    .wr_ena(wr_in),
    .addr(addr_in),
    .dout(ram_dout)
    );

always @(posedge clk) begin
    data_valid_out <= rd_in;
end // always @(posedge clk)

assign data_out = ram_dout;

endmodule


module ram # (
    parameter RAM_DEPTH = 1024,
    parameter RAM_DATA_WIDTH = 256
    )(

    input                       clk,
    input [RAM_DATA_WIDTH-1:0]  din,
    input                       wr_ena,
    input [RAM_ADDR_WIDTH-1:0]  addr,
    output [RAM_DATA_WIDTH-1:0] dout
    );

localparam RAM_ADDR_WIDTH = $clog2(RAM_DEPTH);
reg [RAM_DATA_WIDTH-1:0] ram[0:RAM_DEPTH-1];

always @(posedge clk) begin
    if (wr_ena) begin
        ram[addr] <= din;
    end // if (wr_ena)
end

assign dout = ram[addr];
endmodule