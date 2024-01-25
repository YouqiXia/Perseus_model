//--------------------------------------------------------------------------------------------
// Module:     register renaming table
// Author:     
// Contact:    
//
// Description: Register renaming table.
//
//--------------------------------------------------------------------------------------------


module rrt #(
    parameter RRT_SIZE = 32,
    parameter PHY_REG_ADDR_WIDTH = 6,
    parameter REG_ADDR_WIDTH = 5
) (
    input wire clk                                         ,
    input wire rst                                         ,
    input wire [4:0] rs1_address_first_i                   ,           
    input wire [4:0] rs2_address_first_i                   ,           
    input wire [4:0] rd_address_first_i                    ,          
    input wire [4:0] rs1_address_second_i                  ,            
    input wire [4:0] rs2_address_second_i                  ,            
    input wire [4:0] rd_address_second_i                   ,           
    input wire uses_rd_first_i                             , 
    input wire do_rob_write_first                          ,    
    input wire uses_rd_second_i                            ,  
    input wire do_rob_write_second                         ,         
    input wire [PHY_REG_ADDR_WIDTH-1:0] free_list_rdata_first  ,                             
    input wire [PHY_REG_ADDR_WIDTH-1:0] free_list_rdata_second ,                             
    input wire [REG_ADDR_WIDTH-1:0] rob_alu1_wrb_rd_address_i, 
    input wire [REG_ADDR_WIDTH-1:0] rob_alu2_wrb_rd_address_i, 
    input wire [REG_ADDR_WIDTH-1:0] rob_lsu_wrb_rd_address_i , 

    input wire func_alu1_done_valid_i                      ,
    input wire func_alu2_done_valid_i                      ,
    input wire func_lsu_done_valid_i                       ,
    input [PHY_REG_ADDR_WIDTH-1:0] physical_alu1_wrb_addr_i    , 
    input [PHY_REG_ADDR_WIDTH-1:0] physical_alu2_wrb_addr_i    , 
    input [PHY_REG_ADDR_WIDTH-1:0] physical_lsu_wrb_addr_i     , 

    output reg [PHY_REG_ADDR_WIDTH-1:0] name_prs1_first        , 
    output reg [PHY_REG_ADDR_WIDTH-1:0] name_prs2_first        , 
    output reg [PHY_REG_ADDR_WIDTH-1:0] name_lprd_first        , 
    output reg [PHY_REG_ADDR_WIDTH-1:0] name_prd_first         , 
    output reg [PHY_REG_ADDR_WIDTH-1:0] name_prs1_second       , 
    output reg [PHY_REG_ADDR_WIDTH-1:0] name_prs2_second       , 
    output reg [PHY_REG_ADDR_WIDTH-1:0] name_lprd_second       , 
    output reg [PHY_REG_ADDR_WIDTH-1:0] name_prd_second        , 
    output reg [RRT_SIZE-1:0] prf_or_rob_table
);

reg [PHY_REG_ADDR_WIDTH-1:0] rename_table[RRT_SIZE-1:0];
reg [RRT_SIZE-1:0] prf_or_rob;
wire do_rrt_write_first, do_rrt_write_second;
assign do_rrt_write_first = uses_rd_first_i & do_rob_write_first & (rd_address_first_i != 0);
assign do_rrt_write_second = uses_rd_second_i & do_rob_write_second & (rd_address_second_i != 0);

/*
    assign wr_first_wr_second = do_rrt_write_second & (rd_address_first_i == rd_address_second_i);
    assign wr_first_cm_first = do_rob_commit_first & (rd_address_first_i == cm_rd_address_first_i);
    assign wr_first_cm_second = do_rob_commit_second & (rd_address_first_i == cm_rd_address_second_i);
*/

// write renaming table
always @(posedge clk) begin
    if (rst) begin
        for (i = 0; i < RRT_SIZE ; i = i + 1) begin
            rename_table[i] <= 0;
        end
    end else begin
        if (do_rrt_write_first) begin
            rename_table[rd_address_first_i] <= free_list_rdata_first;
        end
        if (do_rrt_write_second) begin
            rename_table[rd_address_second_i] <= free_list_rdata_second;
        end
    end
end

always @(posedge clk) begin
    if (rst) begin
        prf_or_rob <= 0;
    end else if (do_rrt_write_first | do_rrt_write_second) begin
        if (do_rrt_write_first) begin
            prf_or_rob[rd_address_first_i] <= 1'b1;
        end
        if (do_rrt_write_second) begin
            prf_or_rob[rd_address_second_i] <= 1'b1;
        end
    end else begin
        if(func_alu1_done_valid_i & (rename_table[rob_alu1_wrb_rd_address_i] == physical_alu1_wrb_addr_i)) begin
            prf_or_rob[rob_alu1_wrb_rd_address_i] <= 1'b0;
        end
        if(func_alu2_done_valid_i & (rename_table[rob_alu2_wrb_rd_address_i] == physical_alu2_wrb_addr_i)) begin
            prf_or_rob[rob_alu2_wrb_rd_address_i] <= 1'b0;
        end
        if(func_lsu_done_valid_i & (rename_table[rob_lsu_wrb_rd_address_i] == physical_lsu_wrb_addr_i)) begin
            prf_or_rob[rob_lsu_wrb_rd_address_i] <= 1'b0;
        end
    end
end

// read renaming table
always @(posedge clk) begin
    if (rst) begin
        name_prs1_first  <= 0;
        name_prs2_first  <= 0;
        name_lprd_first  <= 0;
        name_prs1_second <= 0;
        name_prs2_second <= 0;
        name_lprd_second <= 0;
    end else begin
        name_prs1_first  <= rename_table [rs1_address_first_i] ;
        name_prs2_first  <= rename_table [rs2_address_first_i] ;
        name_lprd_first  <= rename_table [rd_address_first_i]  ;
        name_prs1_second <= rename_table [rs1_address_second_i];
        name_prs2_second <= rename_table [rs2_address_second_i];
        name_lprd_second <= rename_table [rd_address_second_i] ;
    end
end

always @(*) begin
    name_prd_first  = rename_table [rd_address_first_i] ;
    name_prd_second = rename_table [rd_address_second_i];
    prf_or_rob_table = prf_or_rob;
end

endmodule
