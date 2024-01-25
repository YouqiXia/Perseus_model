module rs_alu(
    input clk,
    input rst,

    // from rs
    input instr1_valid_i, instr2_valid_i,
    input [ROB_INDEX_WIDTH-1:0] instr1_robID_i, instr2_robID_i,
    input [PC_WIDTH-1:0] instr1_pc_i, instr2_pc_i,
    input [PC_WIDTH-1:0] instr1_next_pc_i, instr2_next_pc_i,
    input [PC_WIDTH-1:0] instr1_predict_pc_i, instr2_predict_pc_i,
    //input instr1_rd_use_i, instr2_rd_use_i,
    //input instr1_rs1_use_i, instr1_rs2_use_i, instr2_rs1_use_i, instr2_rs2_use_i,
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

    // to rs
    output rs_alu_ready_first_o, rs_alu_ready_second_o,

    // from ALU
    //input alu1_ready_i, alu2_ready_i,
    // bypassing
    input alu1_done_valid_i, alu2_done_valid_i, lsu_done_valid_i,
    input [PHY_REG_ADDR_WIDTH-1:0] alu1_wb_prd_i, alu2_wb_prd_i, lsu_wb_prd_i,
    input [XLEN-1:0] alu1_wb_data_i, alu2_wb_data_i, lsu_wb_data_i,

    // to ALU
    output alu1_req_valid_o, alu2_req_valid_o,
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
    output alu1_func_modifier_o, alu2_func_modifier_o
);

integer i;
reg [MAX_AGE_WIDTH-1:0] age;

// rs field
reg rs_alu_busy[RS_ALU_SIZE-1:0];
reg [2:0] rs_op_func3[RS_ALU_SIZE-1:0];
reg [ROB_INDEX_WIDTH-1:0] rs_op_robID[RS_ALU_SIZE-1:0];
reg [PHY_REG_ADDR_WIDTH-1:0] rs_prd[RS_ALU_SIZE-1:0];
reg [PHY_REG_ADDR_WIDTH-1:0] rs_prs1[RS_ALU_SIZE-1:0];
reg [PHY_REG_ADDR_WIDTH-1:0] rs_prs2[RS_ALU_SIZE-1:0];
reg [XLEN-1:0] rs_data1[RS_ALU_SIZE-1:0];
reg [XLEN-1:0] rs_data2[RS_ALU_SIZE-1:0];
reg rs_data1_ready[RS_ALU_SIZE-1:0];
reg rs_data2_ready[RS_ALU_SIZE-1:0];
reg [IMM_LEN-1:0] rs_op_imm_data[RS_ALU_SIZE-1:0];
reg [PC_WIDTH-1:0] rs_op_pc[RS_ALU_SIZE-1:0];
reg [PC_WIDTH-1:0] rs_op_next_pc[RS_ALU_SIZE-1:0];
reg [PC_WIDTH-1:0] rs_op_predict_pc[RS_ALU_SIZE-1:0];
reg rs_op_alu_modify[RS_ALU_SIZE-1:0];
reg [1:0] rs_op_alu_select_a[RS_ALU_SIZE-1:0];
reg [1:0] rs_op_alu_select_b[RS_ALU_SIZE-1:0];
reg rs_op_alu_jump[RS_ALU_SIZE-1:0];
reg rs_op_alu_branch[RS_ALU_SIZE-1:0];
reg rs_op_half[RS_ALU_SIZE-1:0];

reg [MAX_AGE_WIDTH-1:0] rs_age[RS_ALU_SIZE-1:0];

// rs_alu write ctrl
wire rs_first_write_ready, rs_second_write_ready;
wire do_rs_write_first, do_rs_write_second;
wire [RS_ALU_INDEX_WIDTH-1:0] wr_rs_index_first, wr_rs_index_second;
wire do_rs_issue_first, do_rs_issue_second;
wire issue_rs_valid_first, issue_rs_valid_second;
wire [RS_LSU_INDEX_WIDTH-1:0] issue_rs_index_first, issue_rs_index_second;

wire [RS_ALU_SIZE-1:0] rs_alu_unused;
generate
    for (genvar j = 0; j < RS_ALU_SIZE; j = j + 1) begin
        assign rs_alu_unused[j] = !rs_alu_busy[j];
    end
endgenerate

select2_unused #(
    .RS_SIZE(RS_ALU_SIZE),
    .RS_INDEX_WIDTH(RS_ALU_INDEX_WIDTH)
)unused2_selector(
    .rs_unused_i(rs_alu_unused),
    .rs_first_write_ready_o(rs_first_write_ready),
    .rs_second_write_ready_o(rs_second_write_ready),
    .wr_rs_index_first_o(wr_rs_index_first),
    .wr_rs_index_second_o(wr_rs_index_second)
);

assign rs_alu_ready_first_o = rs_first_write_ready;
assign rs_alu_ready_second_o = rs_second_write_ready;
assign do_rs_write_first = rs_first_write_ready & instr1_valid_i;
assign do_rs_write_second = rs_second_write_ready & instr2_valid_i;

always @(posedge clk) begin 
    if (rst) begin
        for (i = 0; i < RS_ALU_SIZE; i = i + 1) begin
            rs_alu_busy[i] <= 0;
        end
        age <= 0;
    end else begin
        if (do_rs_write_first) begin 
            rs_alu_busy[wr_rs_index_first] <= 1'b1;
            //$display("write_first_index %d", wr_rs_index_first);
        end
        if (do_rs_write_second) begin 
            rs_alu_busy[wr_rs_index_second] <= 1'b1;
            //$display("write_second_index %d", wr_rs_index_second);
        end
        if (do_rs_issue_first) begin
            rs_alu_busy[issue_rs_index_first] <= 0;
            //$display("issue_first_index %d", issue_rs_index_first);
        end
        if (do_rs_issue_second) begin
            rs_alu_busy[issue_rs_index_second] <= 0;
            //$display("issue_second_index %d", issue_rs_index_second);
        end
        age <= age + 1;
    end
end

always @(posedge clk) begin
    if (do_rs_write_first) begin
        rs_prs1[wr_rs_index_first] <= instr1_prs1_i;
        rs_prs2[wr_rs_index_first] <= instr1_prs2_i;
        rs_data1[wr_rs_index_first] <= instr1_data1_i;
        rs_data2[wr_rs_index_first] <= instr1_data2_i;
        rs_data1_ready[wr_rs_index_first] <= instr1_rs1_ready_i;
        rs_data2_ready[wr_rs_index_first] <= instr1_rs2_ready_i; 
        rs_prd[wr_rs_index_first] <= instr1_prd_i;
        rs_op_robID[wr_rs_index_first] <= instr1_robID_i;
        rs_op_pc[wr_rs_index_first] <= instr1_pc_i;
        rs_op_next_pc[wr_rs_index_first] <= instr1_next_pc_i;
        rs_op_predict_pc[wr_rs_index_first] <= instr1_predict_pc_i;
        rs_op_imm_data[wr_rs_index_first] <= instr1_imm_data_i;
        rs_op_func3[wr_rs_index_first] <= instr1_func3_i;
        rs_op_alu_modify[wr_rs_index_first] <= alu_function_modifier_first_i;
        rs_op_alu_select_a[wr_rs_index_first] <= fu_select_a_first_i;
        rs_op_alu_select_b[wr_rs_index_first] <= fu_select_b_first_i;
        rs_op_half[wr_rs_index_first] <= half_first_i;
        rs_op_alu_jump[wr_rs_index_first] <= jump_first_i;
        rs_op_alu_branch[wr_rs_index_first] <= branch_first_i;
        rs_age[wr_rs_index_first] <= age;
    end
    if (do_rs_write_second) begin
        rs_prs1[wr_rs_index_second] <= instr2_prs1_i;
        rs_prs2[wr_rs_index_second] <= instr2_prs2_i;
        rs_data1[wr_rs_index_second] <= instr2_data1_i;
        rs_data2[wr_rs_index_second] <= instr2_data2_i;
        rs_data1_ready[wr_rs_index_second] <= instr2_rs1_ready_i;
        rs_data2_ready[wr_rs_index_second] <= instr2_rs2_ready_i;  
        rs_prd[wr_rs_index_second] <= instr2_prd_i;
        rs_op_robID[wr_rs_index_second] <= instr2_robID_i;
        rs_op_pc[wr_rs_index_second] <= instr2_pc_i;
        rs_op_next_pc[wr_rs_index_second] <= instr2_next_pc_i;
        rs_op_predict_pc[wr_rs_index_second] <= instr2_predict_pc_i;
        rs_op_imm_data[wr_rs_index_second] <= instr2_imm_data_i;
        rs_op_func3[wr_rs_index_second] <= instr2_func3_i;
        rs_op_alu_modify[wr_rs_index_second] <= alu_function_modifier_second_i;
        rs_op_alu_select_a[wr_rs_index_second] <= fu_select_a_second_i;
        rs_op_alu_select_b[wr_rs_index_second] <= fu_select_b_second_i;
        rs_op_half[wr_rs_index_second] <= half_second_i;
        rs_op_alu_jump[wr_rs_index_second] <= jump_second_i;
        rs_op_alu_branch[wr_rs_index_second] <= branch_second_i;
        rs_age[wr_rs_index_second] <= age;
    end
end


// bypass
always@(posedge clk) begin
    if (alu1_done_valid_i) begin
        for (i = 0; i < RS_ALU_SIZE; i = i + 1) begin
            if (rs_alu_busy[i]) begin
                if((!rs_data1_ready[i]) && (rs_prs1[i] == alu1_wb_prd_i)) begin
                    rs_data1[i] <= alu1_wb_data_i;
                    rs_data1_ready[i] <= 1'b1;
                end
                if((!rs_data2_ready[i]) && (rs_prs2[i] == alu1_wb_prd_i)) begin
                    rs_data2[i] <= alu1_wb_data_i;
                    rs_data2_ready[i] <= 1'b1;
                end
            end
        end
    end
    if (alu2_done_valid_i) begin
        for (i = 0; i < RS_ALU_SIZE; i = i + 1) begin
            if (rs_alu_busy[i]) begin
                if((!rs_data1_ready[i]) && (rs_prs1[i] == alu2_wb_prd_i)) begin
                    rs_data1[i] <= alu2_wb_data_i;
                    rs_data1_ready[i] <= 1'b1;
                end
                if((!rs_data2_ready[i]) && (rs_prs2[i] == alu2_wb_prd_i)) begin
                    rs_data2[i] <= alu2_wb_data_i;
                    rs_data2_ready[i] <= 1'b1;
                end
            end
        end
    end
    if (lsu_done_valid_i) begin
        for (i = 0; i < RS_ALU_SIZE; i = i + 1) begin
            if (rs_alu_busy[i]) begin
                if((!rs_data1_ready[i]) && (rs_prs1[i] == lsu_wb_prd_i)) begin
                    rs_data1[i] <= lsu_wb_data_i;
                    rs_data1_ready[i] <= 1'b1;
                end
                if(!rs_data2_ready[i] && (rs_prs2[i] == lsu_wb_prd_i)) begin
                    rs_data2[i] <= lsu_wb_data_i;
                    rs_data2_ready[i] <= 1'b1;
                end
            end
        end
    end
end


// rs to alu reg
reg alu1_valid, alu2_valid;
reg [ROB_INDEX_WIDTH-1:0] alu1_rob_index, alu2_rob_index;
reg [PHY_REG_ADDR_WIDTH-1:0] alu1_prd_address, alu2_prd_address;
reg [2:0] alu1_func, alu2_func;
reg [PC_WIDTH-1:0] alu1_pc, alu2_pc;
reg [PC_WIDTH-1:0] alu1_next_pc, alu2_next_pc;
reg [PC_WIDTH-1:0] alu1_predict_pc, alu2_predict_pc;
reg [IMM_LEN-1:0] alu1_imm, alu2_imm;
reg [1:0] alu1_select_a, alu2_select_a;
reg [1:0] alu1_select_b, alu2_select_b;
reg [XLEN-1:0] alu1_rs1_data, alu2_rs1_data;
reg [XLEN-1:0] alu1_rs2_data, alu2_rs2_data;
reg alu1_jump, alu2_jump;
reg alu1_branch, alu2_branch;
reg alu1_half, alu2_half;
reg alu1_func_modifier, alu2_func_modifier;

// rs issue
wire [RS_ALU_SIZE-1:0] rs_select_ready;
generate
    for (genvar j = 0; j < RS_ALU_SIZE; j = j + 1) begin
        assign rs_select_ready[j] = rs_alu_busy[j] & rs_data1_ready[j] & rs_data2_ready[j];
    end
endgenerate

oldest2_ready #(
    .RS_SIZE(RS_ALU_SIZE),
    .RS_INDEX_WIDTH(RS_ALU_INDEX_WIDTH),
    .AGE_WIDTH(MAX_AGE_WIDTH)
)oldest2_selector(
    .ready_i(rs_select_ready),
    .ages_i(rs_age),
    .first_valid_o(issue_rs_valid_first),
    .first_index_o(issue_rs_index_first),
    .second_valid_o(issue_rs_valid_second),
    .second_index_o(issue_rs_index_second)
);

assign do_rs_issue_first = issue_rs_valid_first;
assign do_rs_issue_second = issue_rs_valid_second;

always @(posedge clk) begin                     
    if (do_rs_issue_first) begin
        alu1_valid <= 1'b1;
        alu1_rob_index <= rs_op_robID[issue_rs_index_first];                                      
        alu1_prd_address <= rs_prd[issue_rs_index_first];                                        
        alu1_func <= rs_op_func3[issue_rs_index_first];                                                           
        alu1_pc <= rs_op_pc[issue_rs_index_first];                               
        alu1_next_pc <= rs_op_next_pc[issue_rs_index_first];                                           
        alu1_predict_pc <= rs_op_predict_pc[issue_rs_index_first];                                           
        alu1_imm <= rs_op_imm_data[issue_rs_index_first];       
        alu1_select_a <= rs_op_alu_select_a[issue_rs_index_first];                                     
        alu1_select_b <= rs_op_alu_select_b[issue_rs_index_first];                   
        alu1_rs1_data <= rs_data1[issue_rs_index_first];                                                                   
        alu1_rs2_data <= rs_data2[issue_rs_index_first];                                                                 
        alu1_jump <= rs_op_alu_jump[issue_rs_index_first];               
        alu1_branch <= rs_op_alu_branch[issue_rs_index_first];                 
        alu1_half <= rs_op_half[issue_rs_index_first];               
        alu1_func_modifier <= rs_op_alu_modify[issue_rs_index_first];                        
    end
    else begin
        alu1_valid <= 0;
    end
    if (do_rs_issue_second) begin
        alu2_valid <= 1'b1;
        alu2_rob_index <= rs_op_robID[issue_rs_index_second];                                      
        alu2_prd_address <= rs_prd[issue_rs_index_second];                                        
        alu2_func <= rs_op_func3[issue_rs_index_second];                                                           
        alu2_pc <= rs_op_pc[issue_rs_index_second];                               
        alu2_next_pc <= rs_op_next_pc[issue_rs_index_second];                                           
        alu2_predict_pc <= rs_op_predict_pc[issue_rs_index_second];                                           
        alu2_imm <= rs_op_imm_data[issue_rs_index_second];       
        alu2_select_a <= rs_op_alu_select_a[issue_rs_index_second];                                     
        alu2_select_b <= rs_op_alu_select_b[issue_rs_index_second];                   
        alu2_rs1_data <= rs_data1[issue_rs_index_second];                                                                   
        alu2_rs2_data <= rs_data2[issue_rs_index_second];                                                                 
        alu2_jump <= rs_op_alu_jump[issue_rs_index_second];               
        alu2_branch <= rs_op_alu_branch[issue_rs_index_second];                 
        alu2_half <= rs_op_half[issue_rs_index_second];               
        alu2_func_modifier <= rs_op_alu_modify[issue_rs_index_second];
    end
    else begin
        alu2_valid <= 0;
    end
end

assign alu1_req_valid_o = alu1_valid;
assign alu1_robID_o = alu1_rob_index;
assign alu1_prd_o = alu1_prd_address;
assign alu1_func3_o = alu1_func;
assign alu1_pc_o = alu1_pc;
assign alu1_next_pc_o = alu1_next_pc;
assign alu1_predict_pc_o = alu1_predict_pc;
assign alu1_imm_data_o = alu1_imm;
assign alu1_select_a_o = alu1_select_a;
assign alu1_select_b_o = alu1_select_b;
assign alu1_rs1_data_o = alu1_rs1_data;
assign alu1_rs2_data_o = alu1_rs2_data;
assign alu1_jump_o = alu1_jump;
assign alu1_branch_o = alu1_branch;
assign alu1_half_o = alu1_half;
assign alu1_func_modifier_o = alu1_func_modifier;

assign alu2_req_valid_o = alu2_valid;
assign alu2_robID_o = alu2_rob_index;
assign alu2_prd_o = alu2_prd_address;
assign alu2_func3_o = alu2_func;
assign alu2_pc_o = alu2_pc;
assign alu2_next_pc_o = alu2_next_pc;
assign alu2_predict_pc_o = alu2_predict_pc;
assign alu2_imm_data_o = alu2_imm;
assign alu2_select_a_o = alu2_select_a;
assign alu2_select_b_o = alu2_select_b;
assign alu2_rs1_data_o = alu2_rs1_data;
assign alu2_rs2_data_o = alu2_rs2_data;
assign alu2_jump_o = alu2_jump;
assign alu2_branch_o = alu2_branch;
assign alu2_half_o = alu2_half;
assign alu2_func_modifier_o = alu2_func_modifier;


endmodule