/*
Func Description:
1. Serialize the input data
2. The delay from start to bit_o is 2 read clock cycle
Date: 2018-7-15
*/
`timescale 1ns/1ps
module serializer (
    input       wr_clk,    // Clock
    input       wr_rst,  // Asynchronous reset active low

    input [15:0]data_in,
    input       data_valid_in,

    // Serilizer interface
    input       rd_clk,
    input       rd_rst,
    input       start,
    input [7:0] length_in,

    output      clk_o,
    output      bit_o,
    output reg  bit_valid_o
);

localparam IDLE = 1'b0, SHIFT = 1'b1;
reg [3:0]    bit_index;
reg         state;
reg [15:0]  shift_reg;
reg [7:0]   ram_wr_addr;
reg [7:0]   ram_rd_addr;
wire [15:0] ram_dout;
(*mark_debug = "true"*)wire        load;
reg [7:0]   ram_rd_cnt;
reg [7:0]   length_in_reg;


assign clk_o = rd_clk;

always @(posedge wr_clk) begin
    if(wr_rst) begin
        ram_wr_addr <= 'h0;
    end // if(wr_rst_n)
    else if (data_valid_in) begin
        ram_wr_addr <= ram_wr_addr + 1'b1;
    end // else if (data_valid_in)
end // always @(posedge wr_clk)
/*
wire [0:0] data_valid_ila;
assign data_valid_ila[0] = data_valid_in;
ila_ram_wr ila_ram_wr_inst (
	.clk(wr_clk), // input wire clk
	.probe0(data_valid_ila), // input wire [0:0]  probe0
	.probe1(ram_wr_addr), // input wire [7:0]  probe1
	.probe2(data_in) // input wire [15:0]  probe2
);
*/
dram_16inx256 dram_16inx256_inst (
  .clka(wr_clk),    // input wire clka
  .ena(1'b1),      // input wire ena
  .wea(data_valid_in),      // input wire [0 : 0] wea
  .addra(ram_wr_addr),  // input wire [7 : 0] addra
  .dina(data_in),    // input wire [15 : 0] dina
  .clkb(rd_clk),    // input wire clkb
  .enb(1'b1),      // input wire enb
  .addrb(ram_rd_addr),  // input wire [7 : 0] addrb
  .doutb(ram_dout)  // output wire [15 : 0] doutb
);

always @(posedge rd_clk) begin
    if (rd_rst) begin
        state <= IDLE;
        bit_index <= 'h0;
        length_in_reg <= 'h0;
        bit_valid_o <= 1'b0;
    end // if (~rst_n)
    else begin
        case(state)
            IDLE: begin
                if (start)begin
                    state <= SHIFT;
                    length_in_reg <= length_in;
                end // if (start)
                bit_index <= 'h0;
                ram_rd_cnt <= 'h0;
                bit_valid_o <= 1'b0;
            end
            SHIFT: begin
                bit_valid_o <= 1'b1;
                if (&bit_index == 1'b1) begin
                    bit_index <= 'h0;
                    if (ram_rd_cnt == (length_in_reg - 1'd1))
                        state <= IDLE;
                    else
                        ram_rd_cnt <= ram_rd_cnt + 1'd1;
                end // if (fifo_empty && (&bit_index = 1'b1))
                else begin
                    bit_index <= bit_index + 1'b1;
                end // else
            end // SHIFT:
            default: state <= IDLE;
        endcase // state
    end // else
end // always @(posedge clk)

assign load = (state == SHIFT && (|bit_index == 1'b0));


always @(posedge rd_clk) begin
    if (rd_rst) begin
        ram_rd_addr <= 'h0;
    end // if (rd_rst)
    else if (load) begin
        ram_rd_addr <= ram_rd_addr + 1'd1;
    end // else if (ram_rd_ena)
end
// Serilizer
always @(posedge rd_clk) begin
    if(rd_rst) begin
        shift_reg <= 32'h0;
    end else begin
        if (load) begin
            shift_reg <= ram_dout;
        end // if (fifo_dout_valid)
        else begin
            shift_reg[14:0] <= shift_reg[15:1];
        end // else
    end
end

assign bit_o = shift_reg[0];

// ----------------------------- ILA --------------------
/*
wire [0:0] state_ila;
wire [0:0] load_ila;

assign state_ila[0] = state;
assign load_ila[0] = load;


ila_serializer ila_serializer_inst (
	.clk(rd_clk), // input wire clk
	.probe0(bit_index), // input wire [3:0]  probe0
	.probe1(state_ila), // input wire [0:0]  probe1
	.probe2(shift_reg), // input wire [15:0]  probe2
	.probe3(ram_rd_addr), // input wire [7:0]  probe3
	.probe4(ram_dout), // input wire [15:0]  probe4
	.probe5(load_ila), // input wire [0:0]  probe5
	.probe6(ram_rd_cnt), // input wire [7:0]  probe6
	.probe7(length_in_reg) // input wire [7:0]  probe7
);
*/
/*
//--------------------------------------------------------------
wire [7:0] D;
assign D = fifo_dout;
   OSERDESE2 #(
      .DATA_RATE_OQ("SDR"),   // DDR, SDR
      .DATA_RATE_TQ("SDR"),   // DDR, BUF, SDR
      .DATA_WIDTH(8),         // Parallel data width (2-8,10,14)
      .INIT_OQ(1'b0),         // Initial value of OQ output (1'b0,1'b1)
      .INIT_TQ(1'b0),         // Initial value of TQ output (1'b0,1'b1)
      .SERDES_MODE("MASTER"), // MASTER, SLAVE
      .SRVAL_OQ(1'b0),        // OQ output value when SR is used (1'b0,1'b1)
      .SRVAL_TQ(1'b0),        // TQ output value when SR is used (1'b0,1'b1)
      .TBYTE_CTL("FALSE"),    // Enable tristate byte operation (FALSE, TRUE)
      .TBYTE_SRC("FALSE"),    // Tristate byte source (FALSE, TRUE)
      .TRISTATE_WIDTH(1)      // 3-state converter width (1,4)
   )
   OSERDESE2_inst (
      .OFB(),             // 1-bit output: Feedback path for data
      .OQ(Q),               // 1-bit output: Data path output
      // SHIFTOUT1 / SHIFTOUT2: 1-bit (each) output: Data output expansion (1-bit each)
      .SHIFTOUT1(),
      .SHIFTOUT2(),
      .TBYTEOUT(),   // 1-bit output: Byte group tristate
      .TFB(),             // 1-bit output: 3-state control
      .TQ(),               // 1-bit output: 3-state control
      .CLK(clk_bit),             // 1-bit input: High speed clock
      .CLKDIV(clk),       // 1-bit input: Divided clock
      // D1 - D8: 1-bit (each) input: Parallel data inputs (1-bit each)
      .D1(D[0]),
      .D2(D[1]),
      .D3(D[2]),
      .D4(D[3]),
      .D5(D[4]),
      .D6(D[5]),
      .D7(D[6]),
      .D8(D[7]),
      .OCE(1'b1),             // 1-bit input: Output data clock enable
      .RST(rst_n),             // 1-bit input: Reset
      // SHIFTIN1 / SHIFTIN2: 1-bit (each) input: Data input expansion (1-bit each)
      .SHIFTIN1(),
      .SHIFTIN2(),
      // T1 - T4: 1-bit (each) input: Parallel 3-state inputs
      .T1(1'b1),
      .T2(),
      .T3(),
      .T4(),
      .TBYTEIN(),     // 1-bit input: Byte group tristate
      .TCE(1'b1)              // 1-bit input: 3-state clock enable
   );
*/
endmodule
