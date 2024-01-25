//--------------------------------------------------------------------------------------------
// Module:     f2if2o
// Author:     
// Contact:    
//
// Description: First in first out register with 2 input ports and 2 output ports.
//
//--------------------------------------------------------------------------------------------


module f2if2o #(
    parameter FIFO_SIZE = 32,
    parameter FIFO_SIZE_WIDTH = 5,
    parameter FIFO_DATA_WIDTH = 5
) (
    input wire clk                                  ,
    input wire rst                                  ,
    input wire wr_first_en_i                        ,
    input wire wr_second_en_i                       ,
    input wire rd_first_en_i                        ,
    input wire rd_second_en_i                       ,
    input wire [FIFO_DATA_WIDTH-1:0] wdata_first_i  ,
    input wire [FIFO_DATA_WIDTH-1:0] wdata_second_i ,
    output reg [FIFO_DATA_WIDTH-1:0] rdata_fisrt_o  ,
    output reg [FIFO_DATA_WIDTH-1:0] rdata_second_o ,
    output reg fifo_full_o                          ,
    output reg fifo_almost_full_o                   ,
    output reg fifo_empty_o                         ,
    output reg fifo_almost_empty_o                  ,
    output reg [FIFO_SIZE_WIDTH:0] fifo_num_o       
);

    reg [FIFO_DATA_WIDTH-1:0] fifo_queue[FIFO_SIZE-1:0];
    reg [FIFO_SIZE_WIDTH-1:0] rd_line;
    reg [FIFO_SIZE_WIDTH-1:0] wr_line;
    wire [FIFO_SIZE_WIDTH-1:0] rd_second_line;
    wire [FIFO_SIZE_WIDTH-1:0] wr_second_line;

    integer i;
    /* verilator lint_off WIDTH */
    //write fifo
    always @(posedge clk) begin            
        if (rst) begin
            for (i = 0; i < FIFO_SIZE ; i = i + 1) begin
                fifo_queue[i] <= 0;
            end
        end else if (wr_first_en_i & wr_second_en_i) begin
            fifo_queue[wr_line] <= wdata_first_i;
            fifo_queue[wr_second_line] <= wdata_second_i;
        end else if (wr_first_en_i) begin
            fifo_queue[wr_line] <= wdata_first_i;
        end else if (wr_second_en_i) begin
            fifo_queue[wr_line] <= wdata_second_i;
        end
    end
    assign wr_second_line = (wr_line == FIFO_SIZE - 1) ? 0 
                                                       : wr_line + 1;

    //read fifo
    always @(*) begin
        case({rd_second_en_i, rd_first_en_i})
            2'b00 : begin
                rdata_first_o = fifo_queue[rd_line];
                rdata_second_o = fifo_queue[rd_second_line];
            end
            2'b01 : begin
                rdata_first_o = fifo_queue[rd_line];
            end
            2'b10 : begin
                rdata_second_o = fifo_queue[rd_line];
            end
            2'b11 : begin
                rdata_first_o = fifo_queue[rd_line];
                rdata_second_o = fifo_queue[rd_second_line];
            end
        endcase
    end
    assign rd_second_line = (rd_line == FIFO_SIZE - 1) ? 0 
                                                       : rd_line + 1;

    //fifo number calculate
    always @(posedge clk) begin
        if(rst) begin
            fifo_num_o <= 0;
        end else begin
            case({wr_first_en_i, wr_second_en_i, rd_first_en_i, rd_second_en_i})
                4'b0001, 4'b0010, 4'b0111, 4'b1011 : begin
                    fifo_num_o <= fifo_num_o - 1;
                end
                4'b0011 : begin
                    fifo_num_o <= fifo_num_o - 2;
                end
                4'b0100, 4'b1000, 4'b1101, 4'b1110 : begin
                    fifo_num_o <= fifo_num_o + 1;
                end
                4'b1100 : begin
                    fifo_num_o <= fifo_num_o + 2;
                end
                default : begin
                    fifo_num_o <= fifo_num_o;
                end
            endcase
        end
    end

    always @(*) begin
        fifo_full_o = fifo_num_o == FIFO_SIZE;
        fifo_almost_full_o = fifo_num_o >= FIFO_SIZE - 1;
    end

    always @(*) begin
        fifo_empty_o = fifo_num_o == 0;
        fifo_almost_empty_o = fifo_num_o <= 1;
    end

    //write counter
    always @(posedge clk) begin 
        if (rst) begin
            wr_line <= 0;
        end else if (wr_first_en_i & wr_second_en_i) begin
            wr_line <= (wr_line + 2 > FIFO_SIZE - 1) ? (wr_line + 2 - FIFO_SIZE)
                                                     : (wr_line + 2);
        end else if (wr_first_en_i ^ wr_second_en_i) begin
            wr_line <= (wr_line + 1 > FIFO_SIZE - 1) ? 0
                                                     : (wr_line + 1);
        end
    end

    //read counter
    always @(posedge clk) begin 
        if (rst) begin
            rd_line <= 0;
        end else if (rd_first_en_i & rd_second_en_i) begin
            rd_line <= (rd_line + 2 > FIFO_SIZE - 1) ? (rd_line + 2 - FIFO_SIZE)
                                                     : (rd_line + 2);
        end else if (rd_first_en_i ^ rd_second_en_i) begin
            rd_line <= (rd_line + 1 > FIFO_SIZE - 1) ? 0
                                                     : (rd_line + 1);
        end
    end

endmodule
