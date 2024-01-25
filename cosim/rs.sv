module rs(
    input clk,
    input rst,

    // from instr window
    input instr1_valid_i, instr2_valid_i,
    input [ROB_INDEX_WIDTH-1:0] instr1_robID_i, instr2_robID_i,
    input [PC_WIDTH-1:0] instr1_pc_i, instr2_pc_i, 
    input [PC_WIDTH-1:0] instr1_next_pc_i, instr2_next_pc_i,
    input [PC_WIDTH-1:0] instr1_predict_pc_i, instr2_predict_pc_i,
    //input instr1_rd_use_i, instr2_rd_use_i,
    input instr1_rs1_use_i, instr1_rs2_use_i, instr2_rs1_use_i, instr2_rs2_use_i,
    input [PHY_REG_ADDR_WIDTH-1:0] instr1_prd_i, instr2_prd_i,
    input [PHY_REG_ADDR_WIDTH-1:0] instr1_prs1_i, instr1_prs2_i, instr2_prs1_i, instr2_prs2_i,
    input instr1_rs1_ready_i, instr1_rs2_ready_i, instr2_rs1_ready_i, instr2_rs2_ready_i,
    input [XLEN-1:0] instr1_data1_i, instr1_data2_i, instr2_data1_i, instr2_data2_i,
    input [IMM_LEN-1:0] instr1_imm_data_i, instr2_imm_data_i,
    
    input [2:0] instr1_func3_i, instr2_func3_i,
    input alu_function_modifier_first_i, alu_function_modifier_second_i,
    input [1:0] fu_select_a_first_i, fu_select_a_second_i,
    input [1:0] fu_select_b_first_i, fu_select_b_second_i,
    input half_first_i, half_second_i,
    input jump_first_i, jump_second_i,
    input branch_first_i, branch_second_i,
    input is_alu_first_i, is_alu_second_i,
    input is_fence_first_i, is_fence_second_i,
    input [1:0] fence_op_first_i, fence_op_second_i,
    input is_aext_first_i, is_aext_second_i,
    input load_first_i, load_second_i,
    input store_first_i, store_second_i,
    input [LDU_OP_WIDTH-1:0] ldu_op_first_i, ldu_op_second_i,
    input [STU_OP_WIDTH-1:0] stu_op_first_i, stu_op_second_i,
    input aq_first_i, aq_second_i,
    input rl_first_i, rl_second_i,

    // to issue queue
    output rs_ready_first_o, rs_ready_second_o,

    // from FU
    //input alu1_ready_i, alu2_ready_i,
    input lsu_ready_i,
    // bypassing
    input alu1_done_valid_i, alu2_done_valid_i, lsu_done_valid_i,
    input [PHY_REG_ADDR_WIDTH-1:0] alu1_wb_prd_i, alu2_wb_prd_i, lsu_wb_prd_i,
    input [XLEN-1:0] alu1_wb_data_i, alu2_wb_data_i, lsu_wb_data_i,
    
    // to FU
    output alu1_req_valid_o, alu2_req_valid_o, lsu_req_valid_o,
    // alu
    output [ROB_INDEX_WIDTH-1:0] alu1_robID_o, alu2_robID_o,
    output [PHY_REG_ADDR_WIDTH-1:0] alu1_prd_o, alu2_prd_o,
    output [2:0] alu1_func3_o, alu2_func3_o,
    output [PC_WIDTH-1:0] alu1_pc_o, alu2_pc_o,
    output [PC_WIDTH-1:0] alu1_next_pc_o, alu2_next_pc_o,
    output [PC_WIDTH-1:0] alu1_predict_pc_o, alu2_predict_pc_o,
    output [IMM_LEN-1:0] alu1_imm_data_o, alu2_imm_data_o,
    output [XLEN-1:0] alu1_rs1_data_o, alu1_rs2_data_o, alu2_rs1_data_o, alu2_rs2_data_o,
    output [1:0] alu1_select_a_o, alu1_select_b_o,
    output [1:0] alu2_select_a_o, alu2_select_b_o,
    output alu1_half_o, alu2_half_o,
    output alu1_jump_o, alu2_jump_o,
    output alu1_branch_o, alu2_branch_o,
    output alu1_func_modifier_o, alu2_func_modifier_o,
    // lsu
    output [LSU_DATA_WIDTH-1:0] rs_lsu_package_o
);

wire is_lsu_first, is_lsu_second;
wire alurs_instr1_valid, alurs_instr2_valid;
wire lsurs_instr1_valid, lsurs_instr2_valid;

assign is_lsu_first = load_first_i | store_first_i;
assign is_lsu_second = load_second_i | store_second_i;
assign alurs_instr1_valid = instr1_valid_i & is_alu_first_i;
assign alurs_instr2_valid = instr2_valid_i & is_alu_second_i;
assign lsurs_instr1_valid = instr1_valid_i & is_lsu_first;
assign lsurs_instr2_valid = instr2_valid_i & is_lsu_second;

reg instr1_data1_ready, instr1_data2_ready;
reg instr2_data1_ready, instr2_data2_ready;
reg [XLEN-1:0] instr1_select_data1, instr1_select_data2;
reg [XLEN-1:0] instr2_select_data1, instr2_select_data2;

// input bypassing
always @(*) begin
    instr1_select_data1 = instr1_data1_i;
    instr1_data1_ready = instr1_rs1_use_i ? instr1_rs1_ready_i : 1;
    if((!instr1_data1_ready) & alu1_done_valid_i & (instr1_prs1_i == alu1_wb_prd_i)) begin
        instr1_select_data1 = alu1_wb_data_i;
        instr1_data1_ready = 1;
    end
    if((!instr1_data1_ready) & alu2_done_valid_i & (instr1_prs1_i == alu2_wb_prd_i)) begin
        instr1_select_data1 = alu2_wb_data_i;
        instr1_data1_ready = 1;
    end
    if((!instr1_data1_ready) & lsu_done_valid_i & (instr1_prs1_i == lsu_wb_prd_i)) begin
        instr1_select_data1 = lsu_wb_data_i;
        instr1_data1_ready = 1;
    end
end

always @(*) begin
    instr1_select_data2 = instr1_data2_i;
    instr1_data2_ready = instr1_rs2_use_i ? instr1_rs2_ready_i : 1;
    if((!instr1_data2_ready) & alu1_done_valid_i & (instr1_prs2_i == alu1_wb_prd_i)) begin
        instr1_select_data2 = alu1_wb_data_i;
        instr1_data2_ready = 1;
    end
    if((!instr1_data2_ready) & alu2_done_valid_i & (instr1_prs2_i == alu2_wb_prd_i)) begin
        instr1_select_data2 = alu2_wb_data_i;
        instr1_data2_ready = 1;
    end
    if((!instr1_data2_ready) & lsu_done_valid_i & (instr1_prs2_i == lsu_wb_prd_i)) begin
        instr1_select_data2 = lsu_wb_data_i;
        instr1_data2_ready = 1;
    end
end

always @(*) begin
    instr2_select_data1 = instr2_data1_i;
    instr2_data1_ready = instr2_rs1_use_i ? instr2_rs1_ready_i : 1;
    if((!instr2_data1_ready) & alu1_done_valid_i & (instr2_prs1_i == alu1_wb_prd_i)) begin
        instr2_select_data1 = alu1_wb_data_i;
        instr2_data1_ready = 1;
    end
    if((!instr2_data1_ready) & alu2_done_valid_i & (instr2_prs1_i == alu2_wb_prd_i)) begin
        instr2_select_data1 = alu2_wb_data_i;
        instr2_data1_ready = 1;
    end
    if((!instr2_data1_ready) & lsu_done_valid_i & (instr2_prs1_i == lsu_wb_prd_i)) begin
        instr2_select_data1 = lsu_wb_data_i;
        instr2_data1_ready = 1;
    end
end

always @(*) begin
    instr2_select_data2 = instr2_data2_i;
    instr2_data2_ready = instr2_rs2_use_i ? instr2_rs2_ready_i : 1;
    if((!instr2_data2_ready) & alu1_done_valid_i & (instr2_prs2_i == alu1_wb_prd_i)) begin
        instr2_select_data2 = alu1_wb_data_i;
        instr2_data2_ready = 1;
    end
    if((!instr2_data2_ready) & alu2_done_valid_i & (instr2_prs2_i == alu2_wb_prd_i)) begin
        instr2_select_data2 = alu2_wb_data_i;
        instr2_data2_ready = 1;
    end
    if((!instr2_data2_ready) & lsu_done_valid_i & (instr2_prs2_i == lsu_wb_prd_i)) begin
        instr2_select_data2 = lsu_wb_data_i;
        instr2_data2_ready = 1;
    end
end

wire rs_alu_ready_first, rs_alu_ready_second;
wire rs_lsu_ready_first, rs_lsu_ready_second;

// instantiate rs_alu and rs_lsu
rs_alu u_rs_alu (
    .clk(clk),
    .rst(rst),
    .instr1_valid_i(alurs_instr1_valid),
    .instr2_valid_i(alurs_instr2_valid),
    .instr1_robID_i(instr1_robID_i), 
    .instr2_robID_i(instr2_robID_i),
    .instr1_pc_i(instr1_pc_i),
    .instr2_pc_i(instr2_pc_i),
    .instr1_next_pc_i(instr1_next_pc_i),
    .instr2_next_pc_i(instr2_next_pc_i),
    .instr1_predict_pc_i(instr1_predict_pc_i),
    .instr2_predict_pc_i(instr2_predict_pc_i),
    .instr1_prd_i(instr1_prd_i), 
    .instr2_prd_i(instr2_prd_i),
    .instr1_prs1_i(instr1_prs1_i), 
    .instr1_prs2_i(instr1_prs2_i), 
    .instr2_prs1_i(instr2_prs1_i), 
    .instr2_prs2_i(instr2_prs2_i),
    .instr1_rs1_ready_i(instr1_data1_ready), 
    .instr1_rs2_ready_i(instr1_data2_ready), 
    .instr2_rs1_ready_i(instr2_data1_ready), 
    .instr2_rs2_ready_i(instr2_data2_ready),
    .instr1_data1_i(instr1_select_data1), 
    .instr1_data2_i(instr1_select_data2), 
    .instr2_data1_i(instr2_select_data1), 
    .instr2_data2_i(instr2_select_data2),
    .instr1_imm_data_i(instr1_imm_data_i), 
    .instr2_imm_data_i(instr2_imm_data_i),
    .instr1_func3_i(instr1_func3_i),
    .instr2_func3_i(instr2_func3_i),
    .alu_function_modifier_first_i(alu_function_modifier_first_i),
    .alu_function_modifier_second_i(alu_function_modifier_second_i),
    .fu_select_a_first_i(fu_select_a_first_i),
    .fu_select_a_second_i(fu_select_a_second_i),
    .fu_select_b_first_i(fu_select_b_first_i),
    .fu_select_b_second_i(fu_select_b_second_i),
    .half_first_i(half_first_i),
    .half_second_i(half_second_i),
    .jump_first_i(jump_first_i),
    .jump_second_i(jump_second_i),
    .branch_first_i(branch_first_i),
    .branch_second_i(branch_second_i),
    .rs_alu_ready_first_o(rs_alu_ready_first),
    .rs_alu_ready_second_o(rs_alu_ready_second),
    //.alu1_ready_i(alu1_ready_i),
    //.alu2_ready_i(alu2_ready_i),
    .alu1_done_valid_i(alu1_done_valid_i),
    .alu2_done_valid_i(alu2_done_valid_i),
    .lsu_done_valid_i(lsu_done_valid_i),
    .alu1_wb_prd_i(alu1_wb_prd_i), 
    .alu2_wb_prd_i(alu2_wb_prd_i), 
    .lsu_wb_prd_i(lsu_wb_prd_i),
    .alu1_wb_data_i(alu1_wb_data_i), 
    .alu2_wb_data_i(alu2_wb_data_i), 
    .lsu_wb_data_i(lsu_wb_data_i),
    .alu1_req_valid_o(alu1_req_valid_o),
    .alu2_req_valid_o(alu2_req_valid_o),
    .alu1_robID_o(alu1_robID_o),
    .alu2_robID_o(alu2_robID_o),
    .alu1_prd_o(alu1_prd_o),
    .alu2_prd_o(alu2_prd_o),
    .alu1_func3_o(alu1_func3_o),
    .alu2_func3_o(alu2_func3_o),
    .alu1_pc_o(alu1_pc_o),
    .alu2_pc_o(alu2_pc_o),
    .alu1_next_pc_o(alu1_next_pc_o),
    .alu2_next_pc_o(alu2_next_pc_o),
    .alu1_predict_pc_o(alu1_predict_pc_o),
    .alu2_predict_pc_o(alu2_predict_pc_o),
    .alu1_imm_data_o(alu1_imm_data_o),
    .alu2_imm_data_o(alu2_imm_data_o),
    .alu1_rs1_data_o(alu1_rs1_data_o),
    .alu1_rs2_data_o(alu1_rs2_data_o),
    .alu2_rs1_data_o(alu2_rs1_data_o),
    .alu2_rs2_data_o(alu2_rs2_data_o),
    .alu1_select_a_o(alu1_select_a_o),
    .alu1_select_b_o(alu1_select_b_o),
    .alu2_select_a_o(alu2_select_a_o),
    .alu2_select_b_o(alu2_select_b_o),
    .alu1_half_o(alu1_half_o),
    .alu2_half_o(alu2_half_o),
    .alu1_jump_o(alu1_jump_o),
    .alu2_jump_o(alu2_jump_o),
    .alu1_branch_o(alu1_branch_o),
    .alu2_branch_o(alu2_branch_o),
    .alu1_func_modifier_o(alu1_func_modifier_o),
    .alu2_func_modifier_o(alu2_func_modifier_o)
);

rs_lsu u_rs_lsu (
    .clk(clk),
    .rst(rst),
    .instr1_valid_i(lsurs_instr1_valid),
    .instr2_valid_i(lsurs_instr2_valid),
    .instr1_robID_i(instr1_robID_i), 
    .instr2_robID_i(instr2_robID_i),
    .instr1_prd_i(instr1_prd_i), 
    .instr2_prd_i(instr2_prd_i),
    .instr1_prs1_i(instr1_prs1_i), 
    .instr1_prs2_i(instr1_prs2_i), 
    .instr2_prs1_i(instr2_prs1_i), 
    .instr2_prs2_i(instr2_prs2_i),
    .instr1_rs1_ready_i(instr1_data1_ready), 
    .instr1_rs2_ready_i(instr1_data2_ready), 
    .instr2_rs1_ready_i(instr2_data1_ready), 
    .instr2_rs2_ready_i(instr2_data2_ready),
    .instr1_data1_i(instr1_select_data1), 
    .instr1_data2_i(instr1_select_data2), 
    .instr2_data1_i(instr2_select_data1), 
    .instr2_data2_i(instr2_select_data2),
    .instr1_imm_data_i(instr1_imm_data_i), 
    .instr2_imm_data_i(instr2_imm_data_i),
    .is_fence_first_i(is_fence_first_i), 
    .is_fence_second_i(is_fence_second_i),
    .fence_op_first_i(fence_op_first_i), 
    .fence_op_second_i(fence_op_second_i),
    .is_aext_first_i(is_aext_first_i), 
    .is_aext_second_i(is_aext_second_i),
    .load_first_i(load_first_i), 
    .load_second_i(load_second_i),
    .store_first_i(store_first_i), 
    .store_second_i(store_second_i),
    .ldu_op_first_i(ldu_op_first_i), 
    .ldu_op_second_i(ldu_op_second_i),
    .stu_op_first_i(stu_op_first_i), 
    .stu_op_second_i(stu_op_second_i),
    .aq_first_i(aq_first_i), 
    .aq_second_i(aq_second_i),
    .rl_first_i(rl_first_i), 
    .rl_second_i(rl_second_i),
    .rs_lsu_ready_first_o(rs_lsu_ready_first),
    .rs_lsu_ready_second_o(rs_lsu_ready_second),
    .lsu_ready_i(lsu_ready_i),
    .alu1_done_valid_i(alu1_done_valid_i),
    .alu2_done_valid_i(alu2_done_valid_i),
    .lsu_done_valid_i(lsu_done_valid_i),
    .alu1_wb_prd_i(alu1_wb_prd_i), 
    .alu2_wb_prd_i(alu2_wb_prd_i), 
    .lsu_wb_prd_i(lsu_wb_prd_i),
    .alu1_wb_data_i(alu1_wb_data_i), 
    .alu2_wb_data_i(alu2_wb_data_i), 
    .lsu_wb_data_i(lsu_wb_data_i),
    .lsu_req_valid_o(lsu_req_valid_o),
    .rs_lsu_package_o(rs_lsu_package_o)
);

assign rs_ready_first_o = rs_alu_ready_first | rs_lsu_ready_first;
assign rs_ready_second_o = rs_alu_ready_second | rs_lsu_ready_second;

endmodule