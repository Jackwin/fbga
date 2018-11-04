`timescale 1ns/1ps

module data_gen (
    input           clk,
    input           rst_n,
    input           data_gen_in,
    input [7:0]     data_gen_num_in,

    output [31:0]   data_gen_o,
    output          data_gen_valid_o

);

localparam IDLE = 1'b0,
            DATA_GEN = 1'b1;

reg         data_gen_r, data_gen_p;
reg [7:0]   data_gen_num;
reg         state;
reg [31:0]  data_gen;
reg         data_gen_valid;
reg [7:0]   data_cnt;

always @(posedge clk) begin
    data_gen_r <= data_gen_in;
    data_gen_p <= ~data_gen_r & data_gen_in;
end // always @(posedge clk)

always @(posedge clk) begin
    if (~rst_n) begin
        data_gen_num <= 'h0;
    end // if (~rst_n)
    else if (data_gen_p) begin
        data_gen_num <= data_gen_num_in;
    end // else if (data_gen_p)
end // always @(posedge clk)

always @(posedge clk) begin
    if (~rst_n) begin
        state <= IDLE;
    end
    else begin
        case(state)
            IDLE: begin
                if (data_gen_p) state <= DATA_GEN;
                data_gen_valid <= 1'b0;
                data_gen <= 32'h00010203;
                data_cnt <= 'h0;
            end // IDLE:
            DATA_GEN: begin
                data_gen <= data_gen + 32'h04040404;
                data_gen_valid <= 1'b1;
                data_cnt <= data_cnt + 1'b1;
                if (data_cnt == data_gen_num) begin
                    state <= IDLE;
                end // if (data_cnt == data_gen_num)
            end // DATA_GEN:
            default: begin
                data_gen_valid <= 1'b0;
                state <= IDLE;
            end
        endcase // state
    end // else
end // always @(posedge clk)

assign data_gen_o = data_gen;
assign data_gen_valid_o = data_gen_valid;

endmodule