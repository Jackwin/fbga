`timescale 1ns/1ps

module reset_bridge (
    input clk,    // Clock
    input arst_n,  // Asynchronous reset active low

    output reg srst_n


);
reg [2:0] q;
reg [2:0] buffer;

always @(posedge clk or negedge arst_n) begin
    if(~arst_n) begin
        q <= 'h0;
    end 
    else begin
         q[2] <= q[1];
         q[1] <= q[0];
         q[0] <= 1'b1;
    end
end

always @(posedge clk) begin
    srst_n <= buffer[2];
    buffer[2] <= buffer[1];
    buffer[1] <= buffer[0];
    buffer[0] <= q[2];
end // always @(posedge clk)


endmodule