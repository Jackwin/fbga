`timescale 1ns/1ps

module oserdes (
    input           wr_clk,    // Clock
    input           wr_rst,  // Asynchronous reset active low

    input [7:0]     data_in,
    input           data_valid_in,

    // Serilizer interface
    input           rd_clk,
    input           rd_rst,
    input           start,      // start to serialize the data
    input [7:0]     length_in,  // The number of data to send

    output          bit_o,  // The serial bit output
    output          clk_o,
    output reg      valid_o

);
localparam IDLE = 1'b0, READ = 1'b1;
reg         state;
reg [7:0]   ram_wr_addr;
reg [7:0]   ram_rd_addr;
wire [7:0]  ram_dout;
reg         ram_dout_valid;
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

dpbram_8inx256 dpbram_8inx256_inst (
  .clka(wr_clk),    // input wire clka
  .ena(1'b1),      // input wire ena
  .wea(data_valid_in),      // input wire [0 : 0] wea
  .addra(ram_wr_addr),  // input wire [7 : 0] addra
  .dina(data_in),    // input wire [7 : 0] dina
  .clkb(wr_clk),    // input wire clkb
  .enb(1'b1),      // input wire enb
  .addrb(ram_rd_addr),  // input wire [7 : 0] addrb
  .doutb(ram_dout)  // output wire [7 : 0] doutb
);

always @(posedge wr_clk) begin
    if (wr_rst) begin
        state <= IDLE;
        length_in_reg <= 'h0;
        ram_dout_valid <= 1'b0;
    end // if (~rst_n)
    else begin
        case(state)
            IDLE: begin
                if (start)begin
                    state <= READ;
                    length_in_reg <= length_in;
                end // if (start)
                ram_rd_cnt <= 'h0;
                ram_dout_valid <= 1'b0;
            end
            READ: begin
                ram_dout_valid <= 1'b1;
                if (ram_rd_cnt == (length_in_reg - 1'd1))
                    state <= IDLE;
                else
                    ram_rd_cnt <= ram_rd_cnt + 1'd1;
            end
            default: state <= IDLE;
        endcase // state
    end // else
end // always @(posedge clk)

always @(posedge wr_clk) begin
    if (wr_rst) begin
        ram_rd_addr <= 'h0;
    end // if (rd_rst)
    else if (state == READ) begin
        ram_rd_addr <= ram_rd_addr + 1'd1;
    end // else if (ram_rd_ena)
end

always @(posedge rd_clk) begin
    valid_o <= ram_dout_valid;
end // always @(posedge rd_clk)



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
      .OQ(bit_o),               // 1-bit output: Data path output
      // SHIFTOUT1 / SHIFTOUT2: 1-bit (each) output: Data output expansion (1-bit each)
      .SHIFTOUT1(),
      .SHIFTOUT2(),
      .TBYTEOUT(),   // 1-bit output: Byte group tristate
      .TFB(),             // 1-bit output: 3-state control
      .TQ(),               // 1-bit output: 3-state control
      .CLK(rd_clk),             // 1-bit input: High speed clock
      .CLKDIV(wr_clk),       // 1-bit input: Divided clock
      // D1 - D8: 1-bit (each) input: Parallel data inputs (1-bit each)
      .D1(ram_dout[0]),
      .D2(ram_dout[1]),
      .D3(ram_dout[2]),
      .D4(ram_dout[3]),
      .D5(ram_dout[4]),
      .D6(ram_dout[5]),
      .D7(ram_dout[6]),
      .D8(ram_dout[7]),
      .OCE(1'b1),             // 1-bit input: Output data clock enable
      .RST(wr_rst),             // 1-bit input: Reset
      // SHIFTIN1 / SHIFTIN2: 1-bit (each) input: Data input expansion (1-bit each)
      .SHIFTIN1(),
      .SHIFTIN2(),
      // T1 - T4: 1-bit (each) input: Parallel 3-state inputs
      .T1(),
      .T2(),
      .T3(),
      .T4(),
      .TBYTEIN(),     // 1-bit input: Byte group tristate
      .TCE()              // 1-bit input: 3-state clock enable
   );

endmodule