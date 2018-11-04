`timescale 1ns/1ps
/*
ADC 9826 serial interface to configure the internal registers
*/

module serial_interface # (
    parameter DEBUG = 1
    ) (
    input       clk,
    input       rst_n,

    input       wr_in,
    input [8:0] wr_data_in,
    input [2:0] addr_in,
    input       rd_in,
    output [8:0]rd_data_o,
    output      rd_data_valid_o,

    output      ready_o,
    output reg  done_o,

    //inout       sdata_io,
    input       sdata_i,
    output      sdata_o,
    output reg  tri_en,
    output      sload_o

);


reg [15:0]  shift_reg;
reg [3:0]   cnt;
reg [8:0]   rd_data_reg;
reg         rd_data_valid;
reg         ready;
reg [2:0]   addr_reg;

//reg         tri_en;
reg         sbit_o;
wire        sbit_i;

integer i;
localparam  IDLE = 3'b0,
            WR_CMD = 3'd1,
            RD_CMD = 3'd2,
            RD_DATA = 3'd3,
            END = 3'd4;
reg [2:0]   state;

reg         sbit, sload;

always @(posedge clk) begin
    if (~rst_n) begin
        state <= IDLE;
        sload <= 1'b1;
        sbit_o <= 1'b0;
        tri_en <= 1'b1;
        done_o <= 1'b0;
    end // if (~rst_n)
    else begin
        case(state)
            IDLE: begin
                sload <= 1'b1;
                sbit_o <= 1'b0;
                tri_en <= 1'b1;
                done_o <= 1'b0;
                cnt <= 'h0;
                rd_data_valid <= 1'b0;
                if (wr_in) begin
                    state <= WR_CMD;
                    shift_reg <= {1'b0, addr_in, 3'b0, wr_data_in};
                end
                else if (rd_in) begin
                    shift_reg <= {1'b1, addr_in, 3'b0, 9'h0};
                    state <= RD_CMD;
                end // else if (rd_in)
            end // IDLE:
            WR_CMD: begin
                sload <= 1'b0;
                tri_en <= 1'b0;
                sbit_o <= shift_reg[15];
                for (i = 15; i > 0; i = i - 1) begin
                    shift_reg[i] <= shift_reg[i - 1];
                end // for (int i = 14; i >= 0; i--)
                shift_reg[0] <= 1'b0;
                cnt <= cnt + 1'd1;
                if (cnt == 4'hf) begin
                    state <= IDLE;
                    done_o <= 1'b1;
                end
            end // WR_CMD:
            RD_CMD: begin
                sload <= 1'b0;
                tri_en <= 1'b0;
                sbit_o <= shift_reg[15];
                for (i = 15; i > 0; i = i - 1) begin
                    shift_reg[i] <= shift_reg[i - 1];
                end // for (int i = 14; i >= 0; i--)
                shift_reg[0] <= 1'b0;
                cnt <= cnt + 1'd1;
                if (cnt == 4'h7) begin
                    state <= RD_DATA;
                    done_o <= 1'b1;
                    cnt <= 'h0;
                end // if (cnt == 4'h6)
            end // RD_CMD:
            RD_DATA: begin
                sload <= 1'b0;
                cnt <= cnt + 1'd1;
                tri_en <= 1'b1;

                if (cnt > 4'd7) begin
                    sload <= 1'b1;
                end // if (ctn == 4'd9)
                if (cnt == 4'd9) begin
                    state <= END;
                    rd_data_valid <= 1'b1;
                    done_o <= 1'b1;
                    cnt <= 'h0;
                end
                else begin
                    rd_data_reg[8:0] <= {rd_data_reg[7:0], sbit_i};
                end
            end // RD_DATA:
            END: begin  // Just wait one cycle delay for the ready signal in adc9826
               // cnt <= cnt + 1'd1;
                //if (cnt == 'd2) state <= IDLE;
                state <= IDLE;
            end

            default: state <= IDLE;
        endcase // state
    end // else
end // always @(posedge clk)

//assign sdata_io = (!tri_en) ? sbit_o : 1'bz;
//assign sbit_i = sdata_io;

assign sdata_o = sbit_o;
assign sbit_i = sdata_i;
assign ready_o = (state == IDLE);
assign sload_o = sload;
assign rd_data_o = rd_data_reg;
assign rd_data_valid_o = rd_data_valid;
generate
    if (DEBUG == 1) begin
        wire [0:0] rd_data_valid_ila;
        assign rd_data_valid_ila[0] = rd_data_valid;
        ila_serial ila_serial_inst (
            .clk(clk), // input wire clk
            .probe0(state), // input wire [2:0]  probe0
            .probe1(cnt), // input wire [3:0]  probe1
            .probe2(rd_data_reg), // input wire [8:0]  probe2
            .probe3(rd_data_valid_ila) // input wire [0:0]  probe3
        );
    end
endgenerate


endmodule
