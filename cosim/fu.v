module fu #(
    parameter LSQ_ENTRY_NUM = 8,
    parameter LSQ_ENTRY_NUM_WIDTH = 3
)(
    // global
    input clk,
    input rstn,
    // ALU
    input [XLEN - 1 : 0] rcu_fu_alu1_rs1_i,
    input [XLEN - 1 : 0] rcu_fu_alu2_rs1_i,
    input [XLEN - 1 : 0] rcu_fu_alu1_rs2_i,
    input [XLEN - 1 : 0] rcu_fu_alu2_rs2_i,
    input [IMM_LEN - 1 : 0] rcu_fu_alu1_imm_data_i,               
    input [IMM_LEN - 1 : 0] rcu_fu_alu2_imm_data_i,        
    input [1 : 0] rcu_fu_alu1_opr1_sel_i,                
    input [1 : 0] rcu_fu_alu2_opr1_sel_i,                
    input [1 : 0] rcu_fu_alu1_opr2_sel_i,                
    input [1 : 0] rcu_fu_alu2_opr2_sel_i,          
    input [ROB_INDEX_WIDTH - 1 : 0] rcu_fu_alu1_rob_index_i,            
    input [ROB_INDEX_WIDTH - 1 : 0] rcu_fu_alu2_rob_index_i,            
    input [PHY_REG_ADDR_WIDTH - 1 : 0] rcu_fu_alu1_prd_addr_i,             
    input [PHY_REG_ADDR_WIDTH - 1 : 0] rcu_fu_alu2_prd_addr_i,             
    input rcu_fu_alu1_is_branch_i,
    input rcu_fu_alu2_is_branch_i,
    input rcu_fu_alu1_is_jump_i,  
    input rcu_fu_alu2_is_jump_i, 
    input rcu_fu_alu1_req_valid_i,               
    input rcu_fu_alu2_req_valid_i,         
    input rcu_fu_alu1_half_i,             
    input rcu_fu_alu2_half_i,             
    input [PC_WIDTH-1:0] rcu_fu_alu1_pc_i,   
    input [PC_WIDTH-1:0] rcu_fu_alu2_pc_i,   
    input [PC_WIDTH-1:0] rcu_fu_alu1_next_pc_i, // for jal to wb  
    input [PC_WIDTH-1:0] rcu_fu_alu2_next_pc_i, // for jal to wb  
    input [PC_WIDTH-1:0] rcu_fu_alu1_predict_pc_i,
    input [PC_WIDTH-1:0] rcu_fu_alu2_predict_pc_i,
    input [2:0] rcu_fu_alu1_func3_i, //func3
    input [2:0] rcu_fu_alu2_func3_i, //func3
    input rcu_fu_alu1_func_modifier_i,            
    input rcu_fu_alu2_func_modifier_i,            
        
    output fu_rcu_alu1_resp_valid_o,  
    output fu_rcu_alu2_resp_valid_o,  
    output [ROB_INDEX_WIDTH-1:0] fu_rcu_alu1_wrb_rob_index_o,
    output [ROB_INDEX_WIDTH-1:0] fu_rcu_alu2_wrb_rob_index_o,
    output [PHY_REG_ADDR_WIDTH-1:0] fu_rcu_alu1_wrb_prd_addr_o,
    output [PHY_REG_ADDR_WIDTH-1:0] fu_rcu_alu2_wrb_prd_addr_o,
    output [XLEN - 1 : 0] fu_rcu_alu1_wrb_data_o,
    output [XLEN - 1 : 0] fu_rcu_alu2_wrb_data_o,
    output fu_rcu_alu1_branch_predict_miss_o,
    output fu_rcu_alu2_branch_predict_miss_o,
    output fu_rcu_alu1_branch_taken_o,
    output fu_rcu_alu2_branch_taken_o,
    output [PC_WIDTH - 1 : 0] fu_rcu_alu1_final_next_pc_o,
    output [PC_WIDTH - 1 : 0] fu_rcu_alu2_final_next_pc_o,

    //LSU 
    output lsu_rdy_o,
    input rcu_fu_lsu_vld_i,
    input [ROB_INDEX_WIDTH - 1 : 0] rcu_fu_lsu_rob_index_i,
    input [PHY_REG_ADDR_WIDTH - 1 : 0] rcu_fu_lsu_rd_addr_i,
    input [XLEN - 1 : 0] rcu_fu_agu_virt_base_i,
    input [XLEN - 1 : 0] rcu_fu_lsu_data_i,
    input [XLEN - 1 : 0] rcu_fu_agu_virt_offset_i,
    input rcu_fu_lsu_ls_i,
    input [LDU_OP_WIDTH - 1 : 0] rcu_fu_lsu_ld_opcode_i,
    input [STU_OP_WIDTH - 1 : 0] rcu_fu_lsu_st_opcode_i,
    input rcu_fu_lsu_fenced_i,
    
    output fu_rcu_lsu_comm_vld_o,
    output [ROB_INDEX_WIDTH - 1 : 0] fu_rcu_lsu_comm_rob_index_o,
    output [PHY_REG_ADDR_WIDTH - 1 : 0] fu_rcu_lsu_comm_rd_addr_o,
    output [XLEN - 1 : 0] fu_rcu_lsu_comm_data_o
);


wire [XLEN - 1 : 0] alu1_result;
wire [XLEN - 1 : 0] alu2_result;
wire alu1_resp_valid;
wire alu2_resp_valid;

wire alu1_is_jump;
wire alu2_is_jump;
wire alu1_is_branch;
wire alu2_is_branch;
wire [VIRTUAL_ADDR_LEN-1:0] alu1_pc;    //pc of the currently done instr
wire [VIRTUAL_ADDR_LEN-1:0] alu2_pc;    //pc of the currently done instr
wire [VIRTUAL_ADDR_LEN-1:0] alu1_next_pc;  //pc + 4 of the currently done instr
wire [VIRTUAL_ADDR_LEN-1:0] alu2_next_pc;  //pc + 4 of the currently done instr
reg [XLEN - 1 : 0] alu1_opr1;
reg [XLEN - 1 : 0] alu2_opr1;
reg [XLEN - 1 : 0] alu1_opr2;
reg [XLEN - 1 : 0] alu2_opr2;

wire alu1_cmp_valid;
wire alu2_cmp_valid;
wire alu1_cmp_result;
wire alu2_cmp_result;
wire [63 : 0] alu1_imm_64;
wire [63 : 0] alu2_imm_64;
wire [63 : 0] alu1_pc_64;
wire [63 : 0] alu2_pc_64;

wire [PC_WIDTH - 1 : 0] alu1_final_next_pc;
wire [PC_WIDTH - 1 : 0] alu2_final_next_pc;

assign alu1_imm_64 = {{XLEN_M_IMMLEN{rcu_fu_alu1_imm_data_i[IMM_LEN-1]}}, rcu_fu_alu1_imm_data_i};
assign alu2_imm_64 = {{XLEN_M_IMMLEN{rcu_fu_alu2_imm_data_i[IMM_LEN-1]}}, rcu_fu_alu2_imm_data_i};
assign alu1_pc_64 = {{25'b0}, rcu_fu_alu1_pc_i};
assign alu2_pc_64 = {{25'b0}, rcu_fu_alu2_pc_i};

assign fu_rcu_alu1_resp_valid_o = alu1_resp_valid;
assign fu_rcu_alu2_resp_valid_o = alu2_resp_valid;
assign fu_rcu_alu1_wrb_data_o = (alu1_is_jump) ? {{XLEN_M_PCWIDTH{1'b0}}, alu1_next_pc} : // jal
            alu1_result;  //op
assign fu_rcu_alu2_wrb_data_o = (alu2_is_jump) ? {{XLEN_M_PCWIDTH{1'b0}}, alu2_next_pc} : // jal
            alu2_result;  //op*verilator lint_on UNUSED */

// <> PC_GEN   
assign alu1_final_next_pc = alu1_is_jump ? alu1_result[VIRTUAL_ADDR_LEN - 1 : 0]
         : (alu1_is_branch & alu1_cmp_result) ? alu1_result[VIRTUAL_ADDR_LEN - 1 : 0]  
       : alu1_next_pc;
assign alu2_final_next_pc = alu2_is_jump ? alu2_result[VIRTUAL_ADDR_LEN - 1 : 0]
         : (alu2_is_branch & alu2_cmp_result) ? alu2_result[VIRTUAL_ADDR_LEN - 1 : 0]  
       : alu2_next_pc;

assign fu_rcu_alu1_final_next_pc_o = alu1_final_next_pc;
assign fu_rcu_alu2_final_next_pc_o = alu2_final_next_pc;
assign fu_rcu_alu1_branch_predict_miss_o = alu1_final_next_pc != rcu_fu_alu1_predict_pc_i;
assign fu_rcu_alu2_branch_predict_miss_o = alu2_final_next_pc != rcu_fu_alu2_predict_pc_i;

// cmp 
assign alu1_cmp_valid = rcu_fu_alu1_is_branch_i & ~rcu_fu_alu1_is_jump_i;
assign alu2_cmp_valid = rcu_fu_alu2_is_branch_i & ~rcu_fu_alu2_is_jump_i;

reg [2:0] alu1_func_sel;
reg [2:0] alu2_func_sel;

assign fu_rcu_alu1_branch_taken_o = alu1_cmp_result;
assign fu_rcu_alu2_branch_taken_o = alu2_cmp_result;


always @(*) begin
    if(rcu_fu_alu1_req_valid_i) begin
        case (rcu_fu_alu1_opr1_sel_i)
            ALU_SEL_REG : alu1_opr1 = rcu_fu_alu1_rs1_i;
            ALU_SEL_IMM : alu1_opr1 = 0; // FIXME: it seems that when this is 0, csrrxi and lui are settled
            ALU_SEL_PC  : alu1_opr1 = alu1_pc_64; 
            default : alu1_opr1 = 0;
        endcase
        case (rcu_fu_alu1_opr2_sel_i)
            ALU_SEL_REG : alu1_opr2 = rcu_fu_alu1_rs2_i;
            ALU_SEL_IMM : alu1_opr2 = alu1_imm_64; 
            ALU_SEL_PC  : alu1_opr2 = alu1_pc_64;
            default : alu1_opr2 = 0;
        endcase
    end
    else if(alu1_cmp_valid) begin
        alu1_opr1 = rcu_fu_alu1_rs1_i;
        alu1_opr2 = rcu_fu_alu1_rs2_i;
    end
    else begin
        alu1_opr1 = '0;
        alu1_opr2 = '0;
    end

    if(rcu_fu_alu1_is_branch_i) begin
        alu1_func_sel = ALU_ADD_SUB;
    end 
    else begin
        alu1_func_sel = rcu_fu_alu1_func3_i;
    end

    if(rcu_fu_alu2_is_branch_i) begin
        alu2_func_sel = ALU_ADD_SUB;
    end 
    else begin
        alu2_func_sel = rcu_fu_alu2_func3_i;
    end
end

always @(*) begin
    if(rcu_fu_alu2_req_valid_i) begin
        case (rcu_fu_alu2_opr1_sel_i)
            ALU_SEL_REG : alu2_opr1 = rcu_fu_alu2_rs1_i;
            ALU_SEL_IMM : alu2_opr1 = 0; // FIXME: it seems that when this is 0, csrrxi and lui are settled
            ALU_SEL_PC  : alu2_opr1 = alu2_pc_64;
            default : alu2_opr1 = 0;
        endcase
        case (rcu_fu_alu2_opr2_sel_i)
            ALU_SEL_REG : alu2_opr2 = rcu_fu_alu2_rs2_i;
            ALU_SEL_IMM : alu2_opr2 = alu2_imm_64; 
            ALU_SEL_PC  : alu2_opr2 = alu2_pc_64;
            default : alu2_opr2 = 0;
        endcase
    end
    else if (alu2_cmp_valid) begin
        alu2_opr1 = rcu_fu_alu2_rs1_i;
        alu2_opr2 = rcu_fu_alu2_rs2_i;
    end
    else begin
        alu2_opr1 = '0;
        alu2_opr2 = '0;
    end
end

wire wfi;
wire flush;

assign wfi = 0;
assign flush = 0;

alu alu1(
    .clk(clk),
    .rstn(rstn),
    .wfi(wfi),
    .trap(flush),

    .opr1_i(alu1_opr1),
    .opr2_i(alu1_opr2),
    .half_i(rcu_fu_alu1_half_i),
    .alu_function_select_i(alu1_func_sel),
    .function_modifier_i(rcu_fu_alu1_func_modifier_i),
    .rob_index_i(rcu_fu_alu1_rob_index_i),
    .prd_addr_i(rcu_fu_alu1_prd_addr_i),
    .rcu_fu_alu_req_valid_i(rcu_fu_alu1_req_valid_i),
    .cmp_input_a_i(rcu_fu_alu1_rs1_i),
    .cmp_input_b_i(rcu_fu_alu1_rs2_i),
    .cmp_function_select_i(rcu_fu_alu1_func3_i),
    .is_jump_i(rcu_fu_alu1_is_jump_i),
    .is_branch_i(rcu_fu_alu1_is_branch_i),
    .pc_i(rcu_fu_alu1_pc_i),
    .next_pc_i(rcu_fu_alu1_next_pc_i),

    // for frontend
    .is_jump_o(alu1_is_jump),
    .is_branch_o(alu1_is_branch),
    .pc_o(alu1_pc),
    .next_pc_o(alu1_next_pc),

    //for rcu
    .fu_rcu_alu_resp_valid_o(alu1_resp_valid),
    .prd_addr_o(fu_rcu_alu1_wrb_prd_addr_o),
    .rob_index_o(fu_rcu_alu1_wrb_rob_index_o),
    .alu_result_o(alu1_result),
    .cmp_result_o(alu1_cmp_result)
);

alu alu2(
    .clk(clk),
    .rstn(rstn),
    .wfi(wfi),
    .trap(flush),

    .opr1_i(alu2_opr1),
    .opr2_i(alu2_opr2),
    .half_i(rcu_fu_alu2_half_i),
    .alu_function_select_i(alu2_func_sel),
    .function_modifier_i(rcu_fu_alu2_func_modifier_i),
    .rob_index_i(rcu_fu_alu2_rob_index_i),
    .prd_addr_i(rcu_fu_alu2_prd_addr_i),
    .rcu_fu_alu_req_valid_i(rcu_fu_alu2_req_valid_i),
    .cmp_input_a_i(rcu_fu_alu2_rs1_i),
    .cmp_input_b_i(rcu_fu_alu2_rs2_i),
    .cmp_function_select_i(rcu_fu_alu2_func3_i),
    .is_jump_i(rcu_fu_alu2_is_jump_i),
    .is_branch_i(rcu_fu_alu2_is_branch_i),
    .pc_i(rcu_fu_alu2_pc_i),
    .next_pc_i(rcu_fu_alu2_next_pc_i),

    // for frontend
    .is_jump_o(alu2_is_jump),
    .is_branch_o(alu2_is_branch),
    .pc_o(alu2_pc),
    .next_pc_o(alu2_next_pc),

    //for rcu
    .fu_rcu_alu_resp_valid_o(alu2_resp_valid),
    .prd_addr_o(fu_rcu_alu2_wrb_prd_addr_o),
    .rob_index_o(fu_rcu_alu2_wrb_rob_index_o),
    .alu_result_o(alu2_result),
    .cmp_result_o(alu2_cmp_result)
);


assign lsu_rdy_o = 1'b1;
assign fu_rcu_lsu_comm_vld_o = rcu_fu_lsu_vld_i;
assign fu_rcu_lsu_comm_rob_index_o = rcu_fu_lsu_rob_index_i;
assign fu_rcu_lsu_comm_rd_addr_o = rcu_fu_lsu_rd_addr_i;
assign fu_rcu_lsu_comm_data_o = {XLEN{1'b1}};

endmodule
