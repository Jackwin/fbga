`timescale 1ns/1ps
`define CLK_CYCLE 32
module serializer_tb (
);

logic           wr_clk = 0;
logic           wr_rst;
logic           rd_clk;
logic           rd_rst;

logic [7:0]     data_gen_data;
logic           data_gen_valid;
logic           data_gen;
logic [7:0]     data_gen_num;
logic           ser_ena;
logic           data_ready;
logic           start;
logic [7:0]     length_in;
logic           clk_o;
logic           bit_o;


initial begin
    wr_clk = 0;
    forever # (`CLK_CYCLE/2) wr_clk = ~wr_clk;
end

initial begin
    wr_rst = 1;
    # 95 wr_rst <= 1'b0;
end

initial begin
    rd_clk = 0;
    forever #(`CLK_CYCLE/16) rd_clk = ~rd_clk;
end

initial begin
    rd_rst = 1;
    #105 rd_rst <= 1'b0;
end
/*
initial begin
    data_gen_data <= 0;
    data_gen_valid <= 0;
    # 255;
    @(posedge clk) begin
        data_gen_valid <= 1'b1;
        data_gen_data <= 32'hffffffff;
    end

    @(posedge clk) data_gen_data <= 32'haaaa5555;
    @(posedge clk) data_gen_data <= 32'hbbbbbbbb;
    @(posedge clk) data_gen_data <= 32'hcccccccc;
    @(posedge clk) data_gen_data <= 32'h44444444;
    @(posedge clk) data_gen_valid <= 1'b0;
end // initial
*/

initial begin
    data_gen = 0;
    #250;
    @(posedge wr_clk) data_gen <= 1'b1;
    data_gen_num = 8'd15;

    #40;
    @(posedge wr_clk) data_gen <= 1'b0;
end

initial begin
    start = 0;
    length_in = 0;
    #500;
    @(posedge wr_clk) begin
        start <= 1'b1;
        length_in = 8'd16;
    end
    @(posedge wr_clk) begin
        start <= 1'b0;
        length_in <= 'h0;
    end
end

/*
serializer serializer_inst (
    .wr_clk       (wr_clk),
    .wr_rst       (wr_rst),

    .data_in      (data_gen_data[15:0]),
    .data_valid_in(data_gen_valid),
    .rd_clk       (rd_clk),
    .rd_rst       (rd_rst),
    .start        (start),
    .length_in    (length_in),
    .clk_o        (clk_o),
    .bit_o        (bit_o)

    );
*/

oserdes oserdes_inst (
    .wr_clk       (wr_clk),
    .wr_rst       (wr_rst),
    .data_in      (data_gen_data[7:0]),
    .data_valid_in(data_gen_valid),
    .rd_clk       (rd_clk),
    .rd_rst       (rd_rst),
    .start        (start),
    .length_in    (length_in),
    .bit_o        (bit_o)
   //.valid_o      (bit_valid_o)
    );

data_gen data_gen_inst(
    .clk             (wr_clk),
    .rst_n           (~wr_rst),
    .data_gen_in     (data_gen),
    .data_gen_num_in (data_gen_num),
    .data_gen_o      (data_gen_data),
    .data_gen_valid_o(data_gen_valid)
    );

// -------------------------------------------------------------------
logic       clk;
logic       rst_n;

logic       adc_clk_o;
logic [7:0] adc_data_in;
logic       adc_oeb;
logic       adc_start_in;

logic       cfg_start;
logic       sclk;
logic       sdata_i;
logic       sdata_o;
logic       tri_en;
logic       sload;

logic [8:0] AD9826_reg[0:7];
logic [3:0] wr_addr;
logic [8:0] data;
logic [3:0] bit_cnt;
logic [2:0] state;
logic [2:0] addr;
logic       wr;
logic       wr_ena;

localparam IDLE = 3'd0,
            ADDR = 3'd1,
            NOP = 3'd2,
            WRITE = 3'd3,
            READ = 3'd4;
initial begin
    clk = 0;
    forever
       #2.5 clk = ~clk;
end // initial

initial begin
    rst_n = 0;
    #95;
    rst_n = 1;
end


initial begin
    cfg_start = 1'b0;

    #400;
    @(posedge clk)
        cfg_start <= 1'b1;
    @(posedge clk)
        cfg_start <= 1'b0;

    #2000;
    $stop;
end

always @(posedge sclk) begin
    if (~rst_n) begin
        bit_cnt <= 'h0;
        state <= IDLE;
        wr <= 1'b0;
        wr_ena  <= 1'b0;
    end // if (rst_n)
    else begin
        wr_ena <= 1'b0;
        case(state)
            IDLE: begin
                wr <= 1'b0;
                //addr <= 'h0;
                bit_cnt <= 'h0;
                if (~sload & sdata_o) begin
                    state <= ADDR;
                    wr <= 1'b0;
                    addr <= 'h0;
                end // if (~sload & sdata_o)
                else if (~sload & ~sdata_o) begin
                    state <= ADDR;
                    wr <= 1'b1;
                    addr <= 'h0;
                end // if (sload & sdata)
            end // IDLE:
            ADDR: begin
                addr<= {addr[1:0], sdata_o};
                bit_cnt <= bit_cnt + 1'd1;
                if (bit_cnt == 4'd2) begin
                    state <= NOP;
                    bit_cnt <= 'h0;
                end // if (bit_cnt == 4'd2)
            end
            NOP: begin
                bit_cnt <= bit_cnt + 1'd1;
                if (bit_cnt == 4'd2 && wr == 1'b1) begin
                    state <= WRITE;
                    bit_cnt <= 'h0;
                end
                else if (bit_cnt == 4'd2 && wr == 1'b0) begin
                    state <= READ;
                    data <= AD9826_reg[addr];
                    bit_cnt <= 4'd8;
                end // else
            end // NOP:
            WRITE: begin
                data <= {data[7:0], sdata_o};
                bit_cnt <= bit_cnt + 1'd1;
                if (bit_cnt == 4'd8 && wr == 1'b1) begin
                    AD9826_reg[addr] <= data;
                    wr_ena <= 1'b1;
                    state <= IDLE;
                    $display("Write data is %x, address is %x", data, addr);
                end
            end // WRITE:
            READ: begin
                sdata_i <= data[bit_cnt];
                bit_cnt <= bit_cnt - 1'd1;
                if (bit_cnt == 'h0) begin
                    state <= IDLE;
                end
            end // READ:
            default: begin
                state <= IDLE;
            end // default:
        endcase // state
    end // else
end // always @(posedge sclk)

always @(posedge clk) begin
    if (wr_ena) begin
       AD9826_reg[addr] <= data;
   end
end // always @(posedge clk)

/*
adc9826 adc9826_inst (
    .clk         (clk),
    .rst_n       (rst_n),
    .adc_clk_o   (adc_clk_o),
    .adc_data_in (adc_data_in),
    .adc_oeb     (adc_oeb),
    .adc_start_in(adc_start_in),

    .cfg_start_in(cfg_start),
    .sclk        (sclk),
    .sload_o     (sload),
    .sdata_i     (sdata_i),
    .sdata_o     (sdata_o),
    .tri_en      (tri_en)
    //.sdata_io    (sdata),
    //.sload_o     (sload)
    );
*/
endmodule