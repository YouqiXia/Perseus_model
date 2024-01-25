//--------------------------------------------------------------------------------------------
// Module:     re-order buffer
// Author:     
// Contact:    
//
// Description: Re-order buffer.
//
//--------------------------------------------------------------------------------------------


module rob #(
    parameter ROB_SIZE = 16,
    parameter REG_ADDR_WIDTH = 5,
    parameter PHY_REG_ADDR_WIDTH = 6,
    parameter ROB_SIZE_WIDTH = 4,
    parameter ROB_INDEX_WIDTH = 4,
    parameter PC_WIDTH = 39,
    parameter LDU_OP_WIDTH = 4,
    parameter STU_OP_WIDTH = 5,

    parameter IQ_WIDTH = ROB_INDEX_WIDTH+PHY_REG_ADDR_WIDTH*3+PC_WIDTH*3+LDU_OP_WIDTH+STU_OP_WIDTH+73
) (
    input wire clk                                              ,
    input wire rst                                              ,
    //global ctrl 
    input global_wfi_i                                          ,
    input global_trap_i                                         ,
    input global_ret_i                                          ,
    //hand shake with decode
    input deco_rob_req_valid_first_i                            ,
    input deco_rob_req_valid_second_i                           ,
    output deco_rob_req_ready_first_o                           ,
    output deco_rob_req_ready_second_o                          ,
    output do_rob_commit_first_o                                ,
    output do_rob_commit_second_o                               ,
    output wire [PC_WIDTH-1:0] rob_cmt_pc_first_o               ,
    output wire [PC_WIDTH-1:0] rob_cmt_pc_second_o              ,
    //from decode
    input uses_rs1_first_i                                      ,
    input uses_rs1_second_i                                     ,
    input uses_rs2_first_i                                      ,
    input uses_rs2_second_i                                     ,
    input uses_rd_first_i                                       ,
    input uses_rd_second_i                                      ,
    input uses_csr_first_i                                      ,
    input uses_csr_second_i                                     ,
    input [PC_WIDTH-1:0] pc_first_i                             ,
    input [PC_WIDTH-1:0] pc_second_i                            ,
    input [PC_WIDTH-1:0] next_pc_first_i                        ,
    input [PC_WIDTH-1:0] next_pc_second_i                       ,
    input [PC_WIDTH-1:0] predict_pc_first_i                     ,
    input [PC_WIDTH-1:0] predict_pc_second_i                    ,
    input [4:0] rs1_address_first_i                             ,
    input [4:0] rs1_address_second_i                            ,
    input [4:0] rs2_address_first_i                             ,
    input [4:0] rs2_address_second_i                            ,
    input [4:0] rd_address_first_i                              ,
    input [4:0] rd_address_second_i                             ,
    input [11:0] csr_address_first_i                            ,
    input [11:0] csr_address_second_i                           ,
    input mret_first_i                                          ,
    input mret_second_i                                         ,
    input sret_first_i                                          ,
    input sret_second_i                                         ,
    input wfi_first_i                                           ,
    input wfi_second_i                                          ,
    input half_first_i                                          ,
    input half_second_i                                         ,
    input is_fence_first_i                                      ,
    input is_fence_second_i                                     ,
    input [1:0] fence_op_first_i                                ,
    input [1:0] fence_op_second_i                               ,
    input is_aext_first_i                                       ,
    input is_aext_second_i                                      ,
    input is_mext_first_i                                       ,
    input is_mext_second_i                                      ,
    input csr_read_first_i                                      ,
    input csr_read_second_i                                     ,
    input csr_write_first_i                                     ,
    input csr_write_second_i                                    ,
    input [31:0] imm_data_first_i                               ,
    input [31:0] imm_data_second_i                              ,
    input [2:0] fu_function_first_i                             ,
    input [2:0] fu_function_second_i                            ,
    input alu_function_modifier_first_i                         ,
    input alu_function_modifier_second_i                        ,
    input [1:0] fu_select_a_first_i                             ,
    input [1:0] fu_select_a_second_i                            ,
    input [1:0] fu_select_b_first_i                             ,
    input [1:0] fu_select_b_second_i                            ,
    input jump_first_i                                          ,
    input jump_second_i                                         ,
    input branch_first_i                                        ,
    input branch_second_i                                       ,
    input is_alu_first_i                                        ,
    input is_alu_second_i                                       ,
    input load_first_i                                          ,
    input load_second_i                                         ,
    input store_first_i                                         ,
    input store_second_i                                        ,
    input [LDU_OP_WIDTH-1:0] ldu_op_first_i                     ,
    input [LDU_OP_WIDTH-1:0] ldu_op_second_i                    ,
    input [STU_OP_WIDTH-1:0] stu_op_first_i                     ,
    input [STU_OP_WIDTH-1:0] stu_op_second_i                    ,
    input aq_first_i                                            ,
    input aq_second_i                                           ,
    input rl_first_i                                            ,
    input rl_second_i                                           ,

    //from fu
    input func_alu1_done_valid_i                                ,
    input func_alu2_done_valid_i                                ,
    input func_lsu_done_valid_i                                 ,
    input func_md_done_valid_i                                  ,
    input func_csru_done_valid_i                                ,
    input [ROB_INDEX_WIDTH-1:0] func_alu1_rob_index_i           ,
    input [ROB_INDEX_WIDTH-1:0] func_alu2_rob_index_i           ,
    input [ROB_INDEX_WIDTH-1:0] func_lsu_rob_index_i            ,
    input [ROB_INDEX_WIDTH-1:0] func_md_rob_index_i             ,
    input [ROB_INDEX_WIDTH-1:0] func_csru_rob_index_i           ,
	
    input wire [PHY_REG_ADDR_WIDTH-1:0] name_prs1_first             , 
    input wire [PHY_REG_ADDR_WIDTH-1:0] name_prs2_first             , 
    input wire [PHY_REG_ADDR_WIDTH-1:0] name_lprd_first             , 
    input wire [PHY_REG_ADDR_WIDTH-1:0] name_prd_first              , 
    input wire [PHY_REG_ADDR_WIDTH-1:0] name_prs1_second            , 
    input wire [PHY_REG_ADDR_WIDTH-1:0] name_prs2_second            , 
    input wire [PHY_REG_ADDR_WIDTH-1:0] name_lprd_second            , 
    input wire [PHY_REG_ADDR_WIDTH-1:0] name_prd_second             , 
    input wire [RRT_SIZE-1:0] prf_or_rob_table                  ,

    output wire free_list_wr_first_en                           ,
    output wire free_list_wr_second_en                          ,
    output wire free_list_rd_first_en                           ,
    output wire free_list_rd_second_en                          ,
    output wire [FRLIST_DATA_WIDTH-1:0] free_list_wrdata_first  ,
    output wire [FRLIST_DATA_WIDTH-1:0] free_list_wrdata_second ,

    //to renaming table
    output do_rob_write_first                                   ,
    output do_rob_write_second                                  ,
    output wire [REG_ADDR_WIDTH-1:0] rob_alu1_wrb_rd_address_o  ,
    output wire [REG_ADDR_WIDTH-1:0] rob_alu2_wrb_rd_address_o  ,
    output wire [REG_ADDR_WIDTH-1:0] rob_lsu_wrb_rd_address_o   ,
    //to issue queue
    output wire [IQ_WIDTH-1:0] issue_queue_pkg_first            ,
    output wire [IQ_WIDTH-1:0] issue_queue_pkg_second


);

integer i;

//free_list

wire free_list_full, free_list_almost_full, free_list_empty, free_list_almost_empty;


//issue queue
wire is_csr_first, is_csr_second;

//rob decode op signal
//reg rob_op_mret[ROB_SIZE-1:0];
//reg rob_op_sret[ROB_SIZE-1:0];
//reg rob_op_wfi[ROB_SIZE-1:0];

//rob use signal
reg rob_used[ROB_SIZE-1:0];

//rob prs1
reg [PHY_REG_ADDR_WIDTH-1:0] rob_prs1[ROB_SIZE-1:0];
//rob prs2
reg [PHY_REG_ADDR_WIDTH-1:0] rob_prs2[ROB_SIZE-1:0];
//rob prd
reg [PHY_REG_ADDR_WIDTH-1:0] rob_prd[ROB_SIZE-1:0];
//rob rd
reg [4:0] rob_rd[ROB_SIZE-1:0];
//rob lprd
reg [PHY_REG_ADDR_WIDTH-1:0] rob_lprd[ROB_SIZE-1:0];
//rob rs1 wait
reg [ROB_SIZE-1:0] rob_rs1_wait;
//rob rs2 wait
reg [ROB_SIZE-1:0] rob_rs2_wait;
//rob pc
reg [PC_WIDTH-1:0] rob_op_pc[ROB_SIZE-1:0];
//rob finish
reg rob_finish[ROB_SIZE-1:0];
//rob wake up
reg rob_wakeup[ROB_SIZE-1:0];
//rob skip
wire do_rob_select_skip_first, do_rob_select_skip_second;
//rob write ctrl
wire rob_first_write_ready, rob_second_write_ready;
wire do_rob_write_first, do_rob_write_second;
wire [1:0] do_rob_write;
wire [ROB_INDEX_WIDTH-1:0] wr_rob_index, wr_rob_index_first, wr_rob_index_second;
//rob select ctrl
wire bypass_select_first_valid, bypass_select_second_valid;
wire rob_select_first_valid, rob_select_second_valid;
wire [ROB_INDEX_WIDTH-1:0] rob_select_first_index, rob_select_second_index;
//rob commit ctrl
wire do_rob_commit_first, do_rob_commit_second;
wire [1:0] do_rob_commit;
wire [ROB_INDEX_WIDTH-1:0] cmt_rob_index, cmt_rob_index_first, cmt_rob_index_second;

//freelist
assign free_list_wrdata_first = rob_lprd[cmt_rob_index_first];
assign free_list_wrdata_second = rob_lprd[cmt_rob_index_second];
assign free_list_wr_first_en = do_rob_commit_first & (rob_lprd[cmt_rob_index_first] != 0);
assign free_list_wr_second_en = do_rob_commit_second & (rob_lprd[cmt_rob_index_second] != 0);
assign free_list_rd_first_en = do_rob_write_first & uses_rd_first_i & (rd_address_first_i != 0);
assign free_list_rd_second_en = do_rob_write_second & uses_rd_second_i & (rd_address_second_i != 0);
//assign free_list_rds_first_en = do_rob_commit_first & (rob_prd[cmt_rob_index_first] != 0) & !global_speculate_fault;
//assign free_list_rds_second_en = do_rob_commit_second & (rob_prd[cmt_rob_index_second] != 0) & !global_speculate_fault;


// renaming signals
assign prs1_first = uses_rs1_first_i ? name_prs1_first
                                     : 0;
assign prs2_first = uses_rs2_first_i ? name_prs2_first
                                     : 0;
assign prd_first  = free_list_rd_first_en ? name_prd_first
                                          : 0;
assign lprd_first = uses_rd_first_i ? name_lprd_first
                                    : 0;
assign prd_second = free_list_rd_second_en ? name_prd_second
                                           : 0;
assign prs1_second = uses_rs1_second_i ? (((rs1_address_second_i == rd_address_first_i) & uses_rd_first_i) ? name_prd_first
                                                                                                           : name_prs1_second)
                                       : 0;
assign prs2_second = uses_rs2_second_i ? (((rs2_address_second_i == rd_address_first_i) & uses_rd_first_i) ? name_prd_first
                                                                                                           : name_prs2_second)
                                       : 0;
assign lprd_second = uses_rd_second_i ? ((rd_address_second_i == rd_address_first_i) & uses_rd_first_i) ? name_prd_first
                                                                                                        : name_lprd_second
                                      : 0;
assign rs1_wait_first = uses_rs1_first_i ? prf_or_rob_table[rs1_address_first_i]
                                         : 0;
assign rs2_wait_first = uses_rs2_first_i ? prf_or_rob_table[rs2_address_first_i]
                                         : 0;
assign rs1_wait_second = uses_rs1_second_i ? (((rs1_address_second_i == rd_address_first_i) & uses_rd_first_i) ? 1
                                                                                                               : prf_or_rob_table[rs1_address_second_i])
                                           : 0;
assign rs2_wait_second = uses_rs2_second_i ? (((rs2_address_second_i == rd_address_first_i) & uses_rd_first_i) ? 1
                                                                                                               : prf_or_rob_table[rs2_address_second_i])
                                          : 0;
assign rob_alu1_wrb_rd_address_o = rob_rd[func_alu1_rob_index_i];
assign rob_alu2_wrb_rd_address_o = rob_rd[func_alu2_rob_index_i];
assign rob_lsu_wrb_rd_address_o = rob_rd[func_lsu_rob_index_i];

assign is_csr_first = csr_read_first_i | csr_write_first_i;
assign is_csr_second = csr_read_second_i | csr_write_second_i;

//simulation only
assign do_rob_commit_first_o = do_rob_commit_first;
assign do_rob_commit_second_o = do_rob_commit_second;
assign rob_cmt_pc_first_o = rob_op_pc[cmt_rob_index_first];
assign rob_cmt_pc_second_o = rob_op_pc[cmt_rob_index_second];

//issue queue signals
assign issue_queue_pkg_first = {
    wr_rob_index_first,
    prd_first,
    fu_function_first_i,
    pc_first_i,
    next_pc_first_i,
    predict_pc_first_i,
    imm_data_first_i,
    fu_select_a_first_i,
    fu_select_b_first_i,
    prs1_first,
    prs2_first,
    is_alu_first_i,
    jump_first_i,
    branch_first_i,
    half_first_i,
    alu_function_modifier_first_i,
    is_mext_first_i,
    load_first_i,
    store_first_i,
    ldu_op_first_i,
    stu_op_first_i,
    is_fence_first_i,
    fence_op_first_i,
    is_aext_first_i,
    aq_first_i,
    rl_first_i,
    is_csr_first,
    csr_address_first_i,
    csr_read_first_i,
    csr_write_first_i,
    uses_rs1_first_i,
    uses_rs2_first_i,
    uses_rd_first_i,
    rs1_wait_first,
    rs2_wait_first
};

assign issue_queue_pkg_second = {
    wr_rob_index_second,
    prd_second,
    fu_function_second_i,
    pc_second_i,
    next_pc_second_i,
    predict_pc_second_i,
    imm_data_second_i,
    fu_select_a_second_i,
    fu_select_b_second_i,
    prs1_second,
    prs2_second,
    is_alu_second_i,
    jump_second_i,
    branch_second_i,
    half_second_i,
    alu_function_modifier_second_i,
    is_mext_second_i,
    load_second_i,
    store_second_i,
    ldu_op_second_i,
    stu_op_second_i,
    is_fence_second_i,
    fence_op_second_i,
    is_aext_second_i,
    aq_second_i,
    rl_second_i,
    is_csr_second,
    csr_address_second_i,
    csr_read_second_i,
    csr_write_second_i,
    uses_rs1_second_i,
    uses_rs2_second_i,
    uses_rd_second_i,
    rs1_wait_second,
    rs2_wait_second
};


//Rob Entry

//rob use signal
always @(posedge clk) begin 
    if (rst) begin //trapped = (sip || tip || eip || exception) as input
        for (i = 0; i < ROB_SIZE; i = i + 1) begin //when trapped and reset clean all rob
                rob_used[i] <= 0;
            end 
    end else begin
        if (do_rob_write_first) begin 
            rob_used[wr_rob_index_first] <= 1;
        end
        if (do_rob_write_second) begin 
            rob_used[wr_rob_index_second] <= 1;
        end
        if (do_rob_commit_first) begin 
            rob_used[cmt_rob_index_first] <= 0;
        end
        if (do_rob_commit_second) begin 
            rob_used[cmt_rob_index_second] <= 0;
        end
    end
end
//: rob use signal

//rob prs1
always @(posedge clk) begin 
    if (do_rob_write_first) begin
        rob_prs1[wr_rob_index_first] <= prs1_first; 
    end
    if (do_rob_write_second) begin
        rob_prs1[wr_rob_index_second] <= prs1_second; 
    end
end 
// : rob prs1

//rob prs2
always @(posedge clk) begin 
    if (do_rob_write_first) begin
        rob_prs2[wr_rob_index_first] <= prs2_first; 
    end
    if (do_rob_write_second) begin
        rob_prs2[wr_rob_index_second] <= prs2_second; 
    end
end 
// : rob prs2

//rob prd
always @(posedge clk) begin 
    if (do_rob_write_first) begin
        rob_prd[wr_rob_index_first] <= prd_first;
    end
    if (do_rob_write_second) begin
        rob_prd[wr_rob_index_second] <= prd_second;
    end
end
// : rob prd

//rob lprd
always @(posedge clk) begin
    if (do_rob_write_first) begin
        rob_lprd[wr_rob_index_first] <= lprd_first;
    end
    if (do_rob_write_second) begin
        rob_lprd[wr_rob_index_second] <= lprd_second;
    end
end
// : rob lprd

//rob rs1 wait
always @(posedge clk) begin
    if (do_rob_write_first) begin
        rob_rs1_wait[wr_rob_index_first] <= rs1_wait_first;
    end
    if (do_rob_write_second) begin
        rob_rs1_wait[wr_rob_index_second] <= rs1_wait_second;
    end
end
// : rob rs1 wait

//rob rs2 wait
always @(posedge clk) begin
    if (do_rob_write_first) begin
        rob_rs2_wait[wr_rob_index_first] <= rs2_wait_first;
    end
    if (do_rob_write_second) begin
        rob_rs2_wait[wr_rob_index_second] <= rs2_wait_second;
    end
end
// : rob rs2 wait

//rob rd 
always @(posedge clk) begin
    if (do_rob_write_first) begin
        if (uses_rd_first_i) begin
            rob_rd[wr_rob_index_first] <= rd_address_first_i;
        end else begin
            rob_rd[wr_rob_index_first] <= 0;
        end
    end
    if (do_rob_write_second) begin
        if (uses_rd_second_i) begin
            rob_rd[wr_rob_index_second] <= rd_address_second_i;
        end else begin
            rob_rd[wr_rob_index_second] <= 0;
        end
    end
end
// : rob rd

//rob pc
always @(posedge clk) begin 
    if (do_rob_write_first) begin
        rob_op_pc[wr_rob_index_first] <= pc_first_i;
    end
    if (do_rob_write_second) begin
        rob_op_pc[wr_rob_index_second] <= pc_second_i;
    end
end
// : rob pc

`ifdef REG_TEST
reg [PHY_REG_ADDR_WIDTH-1:0] rob_rs2[ROB_SIZE-1:0];
always @(posedge clk) begin
    if (do_rob_write_first) begin
        if (uses_rs2_first_i) begin
            rob_rs2[wr_rob_index_first] <= rs2_address_first_i;
        end else begin
            rob_rs2[wr_rob_index_first] <= 0;
        end
    end
    if (do_rob_write_second) begin
        if (uses_rs2_second_i) begin
            rob_rs2[wr_rob_index_second] <= rs2_address_second_i;
        end else begin
            rob_rs2[wr_rob_index_second] <= 0;
        end
    end
end
`endif

//rob FU finish
always @(posedge clk) begin
    if (rst) begin
        for (i = 0; i < ROB_SIZE; i = i + 1) begin
            rob_finish[i] <= 0;
        end
    end else begin
        if (func_alu1_done_valid_i) begin
            rob_finish[func_alu1_rob_index_i] <= 1;
        end
        if (func_alu2_done_valid_i) begin
            rob_finish[func_alu2_rob_index_i] <= 1;
        end
        if (func_lsu_done_valid_i) begin
            rob_finish[func_lsu_rob_index_i] <= 1;
        end
        if (func_md_done_valid_i) begin
            rob_finish[func_md_rob_index_i] <= 1;
        end
        if (func_csru_done_valid_i) begin
            rob_finish[func_csru_rob_index_i] <= 1;
        end
        if (do_rob_commit_first) begin
            rob_finish[cmt_rob_index_first] <= 0;
        end
        if (do_rob_commit_second) begin
            rob_finish[cmt_rob_index_second] <= 0;
        end
        if (do_rob_select_skip_first) begin
            rob_finish[wr_rob_index_first] <= 1;
        end
        if (do_rob_select_skip_second) begin
            rob_finish[wr_rob_index_second] <= 1;
        end
    end
end
// : rob FU finish

//: Rob Entry

//rob wake up
always @(posedge clk) begin
    if(do_rob_write_first) begin
        if(load_first_i | store_first_i) begin
            rob_wakeup[wr_rob_index_first] <= 1;
        end else begin
            rob_wakeup[wr_rob_index_first] <= 0;
        end
    end
    if(do_rob_write_second) begin
        if(load_second_i | store_second_i) begin
            rob_wakeup[wr_rob_index_second] <= 1;
        end else begin
            rob_wakeup[wr_rob_index_second] <= 0;
        end
    end
end
assign rcu_lsu_wakeup_o = rob_wakeup[cmt_rob_index_first] & rob_used[cmt_rob_index_first];
assign rcu_lsu_wakeup_index_o = cmt_rob_index_first;
//: rob wake up

//rob skip gen
assign do_rob_select_skip_first = (//exception_first_i | //& !ecause_first_i[EXCEPTION_CAUSE_WIDTH-1] |
                                  mret_first_i |
                                  sret_first_i |
                                  wfi_first_i) &
                                  do_rob_write_first
                                  ;
assign do_rob_select_skip_second = (//exception_second_i | //& !ecause_second_i[EXCEPTION_CAUSE_WIDTH-1] |
                                   mret_second_i |
                                   sret_second_i |
                                   wfi_second_i) &
                                   do_rob_write_second
                                   ;
// : rob skip gen

//rob write ctrl
configurable_2mode_counter #(
    .CNT_SIZE(ROB_SIZE),
    .CNT_SIZE_WIDTH(ROB_SIZE_WIDTH)
) wr_rob_counter (
    .clk(clk),
    .rst(rst),
    .mode_i(do_rob_write),
    .cnt_rst_vector_i({ROB_SIZE_WIDTH{1'b0}}),
    .cnt_o(wr_rob_index),
    .cnt_end_o()
);
assign deco_rob_req_ready_first_o = rob_first_write_ready;
assign deco_rob_req_ready_second_o = rob_second_write_ready;
assign rob_first_write_ready = !rob_used[wr_rob_index_first];
assign rob_second_write_ready = !rob_used[wr_rob_index_second];
assign do_rob_write_first = rob_first_write_ready & 
                            deco_rob_req_valid_first_i & 
                            !global_wfi_i
                            ;
assign do_rob_write_second = rob_second_write_ready & 
                             deco_rob_req_valid_second_i & 
                             !global_wfi_i
                             ;
assign do_rob_write = {do_rob_write_second, do_rob_write_first};
assign wr_rob_index_first = wr_rob_index;
assign wr_rob_index_second = (wr_rob_index == ROB_SIZE - 1) ? 0
                                                            : wr_rob_index + 1;
//: rob entry ctrl


//rob commit ctrl
configurable_2mode_counter #(
    .CNT_SIZE(ROB_SIZE),
    .CNT_SIZE_WIDTH(ROB_SIZE_WIDTH)
) cmt_rob_counter (
    .clk(clk),
    .rst(rst),
    .mode_i(do_rob_commit),
    .cnt_rst_vector_i(4'b0),
    .cnt_o(cmt_rob_index),
    .cnt_end_o()
);

assign do_rob_commit_first = rob_used[cmt_rob_index_first] & 
                             rob_finish[cmt_rob_index_first] & 
                             !global_wfi_i;
assign do_rob_commit_second = rob_used[cmt_rob_index_second] & 
                              rob_finish[cmt_rob_index_second] & 
//                              !rob_op_mret[cmt_rob_index_second] & !rob_op_mret[cmt_rob_index_first] &
//                              !rob_op_sret[cmt_rob_index_second] & !rob_op_sret[cmt_rob_index_first] &
//                              !rob_op_wfi[cmt_rob_index_second] & !rob_op_wfi[cmt_rob_index_first] &
                              do_rob_commit_first;
assign do_rob_commit = {do_rob_commit_second, do_rob_commit_first};
assign cmt_rob_index_first = cmt_rob_index;                                                          
assign cmt_rob_index_second = (cmt_rob_index == ROB_SIZE - 1) ? 0
                                                              : cmt_rob_index + 1;

//: rob commit ctrl


endmodule
