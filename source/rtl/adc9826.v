`timescale 1ns/1ps

module adc9826 # (
    parameter PIX_NUM = 9'd511
)
(
    input           clk,
    input           rst_n,

    input           cfg_done_in,
    output          adc_clk_o,
    input [7:0]     adc_data_in,
    output reg      adc_oeb, //active low
    input           adc_start_in, // from g11620
    output reg      adc_done_out,

    output  [31:0]    ram_addr_o,
    output [31:0]   ram_data_o,
    output          ram_wr_o
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

localparam  IDLE = 2'd0,
            ADC_DELAY = 2'd1,
            IDDR_DELAY = 2'd2,
            DATA = 2'd3;

// AD capture signals
reg [1:0]   state;
wire        iddr_ce;
reg         iddr_rst;
wire [7:0]  iddr_q1/*synthesis keep*/, iddr_q2/*synthesis keep*/;
wire [15:0] iddr_data/*synthesis keep*/;
reg [7:0]   iddr_q2_r;
reg [1:0]   delay_cnt;
reg         adc_data_valid, adc_data_valid_r;
reg [8:0]   adc_data_cnt;
wire [15:0] adc_data;
reg [15:0]  adc_data_r;
reg         cfg_done;
reg         cnt;
// Debug signals
wire [0:0]  adc_ena_ila;
wire [0:0]  cfg_done_ila;
wire [0:0]  iddr_ce_ila;
wire [0:0]  adc_data_valid_ila;
// RAM signals
reg [31:0]   ram_addr;
wire         ram_wr;

assign adc_clk_o = clk;
assign sclk = clk;

always @(posedge clk) begin
    cfg_done <= cfg_done_in;
end // always @(posedge clk)


always @(posedge clk) begin
    if (~rst_n) begin
        state <= IDLE;
        adc_oeb <= 1'b0;
        iddr_rst <= 1'b1;
        adc_done_out <= 1'b0;
    end // if (~rst_n)
    else begin
        iddr_rst <= 1'b0;
        adc_oeb <= 1'b0;
        adc_done_out <= 1'b0;
        case(state)
           IDLE: begin
                if (adc_start_in) begin
                    state <= ADC_DELAY;
                end // if (start_in)
                delay_cnt <= 'h0;
                adc_oeb <= 1'b0;
                iddr_rst <= 1'b1;
            end
            ADC_DELAY: begin
                if (delay_cnt == 2'b11) begin
                    state <= IDDR_DELAY;
                    delay_cnt <= 'h0;
                end // if (delay_cnt == 2'h11)
                else begin
                    delay_cnt <= delay_cnt + 1'd1;
                end // else
            end // DELAY:
            IDDR_DELAY: begin
               if (delay_cnt == 2'h1) begin
                    state <= DATA;
                    delay_cnt <= 'h0;
                end // if (delay_cnt == 2'h11)
                else begin
                    delay_cnt <= delay_cnt + 1'd1;
                end // else
             end
            DATA: begin
                // The adc outout delay is 4 clock cycles
                // two 8-bit to combine 16-bit
                // Use chipscope to capture the ADC output data
                // Use a counter to exit the DATA state
                // Use IDDR in the same_edge_pipeline mode, and the delay is 2 cycles
                adc_data_cnt <= adc_data_cnt + 1'd1;
                if(&adc_data_cnt == 1'd1) begin
                    state <= IDLE;
                    adc_done_out <= 1'b1;
                end // if(&adc_data_cnt == 1'd1)
            end // DATA:
            default: state <= IDLE;
        endcase // state
    end // else
end // always @(posedge clk)

assign iddr_ce = (state == DATA);

always @(posedge clk) begin
    if (~rst_n) begin
        adc_data_valid <= 1'b0;
    end // if (~rst_n)
    else begin
       adc_data_valid <= (state == DATA);
   end
end // always @(posedge clk)


genvar var;
generate
    for (var = 0; var < 8; var = var + 1) begin
        IDDR #(
            .DDR_CLK_EDGE("SAME_EDGE_PIPELINED"), // "OPPOSITE_EDGE", "SAME_EDGE"
                                      //    or "SAME_EDGE_PIPELINED"
            .INIT_Q1(1'b0), // Initial value of Q1: 1'b0 or 1'b1
            .INIT_Q2(1'b0), // Initial value of Q2: 1'b0 or 1'b1
            .SRTYPE("SYNC") // Set/Reset type: "SYNC" or "ASYNC"
        ) IDDR_inst (
            .Q1(iddr_q1[var]), // 1-bit output for positive edge of clock
            .Q2(iddr_q2[var]), // 1-bit output for negative edge of clock
            .C(clk),   // 1-bit clock input
            .CE(iddr_ce), // 1-bit clock enable input
            .D(adc_data_in[var]),   // 1-bit DDR data input
            .R(iddr_rst),   // 1-bit reset
            .S(1'b0)    // 1-bit set
        );
    end
endgenerate


// Strange: Use the SAME_EDGE_PIPELINED in IDDR, but the Q1 and Q2 are not at the same edge
// Q1 is one clock cycle later than Q2.
always @(posedge clk) begin
    iddr_q2_r <= iddr_q2;
end

assign adc_data = {iddr_q1, iddr_q2_r};

always @(posedge clk) begin
    adc_data_r <= adc_data;
end // always @(posedge clk)

always @(posedge clk) begin
    adc_data_valid_r <= adc_data_valid;
end // always @(posedge clk)

always @(posedge clk) begin
    if (~rst_n) begin
        cnt <= 'h0;
    end // if (~rst_n)
    else begin
        if (adc_data_valid_r) cnt <= cnt + 1'b1;
    end // else
end // always @(posedge clk)

assign ram_wr = cnt; //Write for every two 16-bit data

always @(posedge clk) begin
    if (~rst_n) begin
        ram_addr <= 'h0;
    end // if (~rst_n)
    else begin
        if (ram_wr) begin
            ram_addr <= ram_addr + 3'd4; // The address is byte-address
        end // if (adc_data_valid)
    end // else
end // always @(posedge clk)

assign ram_addr_o = ram_addr;
assign ram_wr_o = ram_wr;
assign ram_data_o = {adc_data_r, adc_data};


// -------------------------- debug ---------------------------
assign iddr_ce_ila[0] = iddr_ce;
assign cfg_done_ila[0] = cfg_done;
assign adc_data_valid_ila[0] = adc_data_valid_r;
assign iddr_data = {iddr_q2_r,iddr_q1};

ila_adc ila_adc_inst (
    .clk(clk), // input wire clk
    .probe0(state), // input wire [1:0]  probe0
    .probe1(adc_data_in), // input wire [7:0]  probe1
    .probe2(iddr_data), // input wire [7:0]  probe2
    .probe3(iddr_q2_r), // input wire [7:0]  probe3
    .probe4(delay_cnt), // input wire [1:0]  probe4
    .probe5(adc_data_cnt), // input wire [8:0]  probe5
    .probe6(cfg_done_ila), // input wire [0:0]  probe6
    .probe7(iddr_ce_ila), // input wire [0:0]  probe7
    .probe8(adc_data_valid_ila), // input wire [0:0]  probe8
    .probe9(1'b0) // input wire [0:0]  probe9
);
/*
vio_data_gen ad9826_ena_inst (
    .clk       (clk),
    .probe_out0(adc_ena_ila),
    .probe_out1()
    );
*/

endmodule
