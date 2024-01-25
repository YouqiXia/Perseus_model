module rs_lsu(
    input clk,
    input rst,

    // from rs
    input instr1_valid_i, instr2_valid_i,
    input [ROB_INDEX_WIDTH-1:0] instr1_robID_i, instr2_robID_i,
    //input [PC_WIDTH-1:0] instr1_pc_i, instr2_pc_i, 
    //input [PC_WIDTH-1:0] instr1_next_pc_i, instr2_next_pc_i,
    //input [PC_WIDTH-1:0] instr1_predict_pc_i, instr2_predict_pc_i,
    //input instr1_rd_use_i, instr2_rd_use_i,
    //input instr1_rs1_use_i, instr1_rs2_use_i, instr2_rs1_use_i, instr2_rs2_use_i,
    input [PHY_REG_ADDR_WIDTH-1:0] instr1_prd_i, instr2_prd_i,
    input [PHY_REG_ADDR_WIDTH-1:0] instr1_prs1_i, instr1_prs2_i, instr2_prs1_i, instr2_prs2_i,
    input instr1_rs1_ready_i, instr1_rs2_ready_i, instr2_rs1_ready_i, instr2_rs2_ready_i,
    input [XLEN-1:0] instr1_data1_i, instr1_data2_i, instr2_data1_i, instr2_data2_i,
    input [IMM_LEN-1:0] instr1_imm_data_i, instr2_imm_data_i,
    //input [2:0] instr1_func3_i, instr2_func3_i,

    input is_fence_first_i, is_fence_second_i,
    input [1:0] fence_op_first_i, fence_op_second_i,
    input is_aext_first_i, is_aext_second_i,
    input load_first_i, load_second_i,
    input store_first_i, store_second_i,
    input [LDU_OP_WIDTH-1:0] ldu_op_first_i, ldu_op_second_i,
    input [STU_OP_WIDTH-1:0] stu_op_first_i, stu_op_second_i,
    input aq_first_i, aq_second_i,
    input rl_first_i, rl_second_i,

    // to rs
    output rs_lsu_ready_first_o, rs_lsu_ready_second_o,

    // from LSU
    input lsu_ready_i,
    // bypassing
    input alu1_done_valid_i, alu2_done_valid_i, lsu_done_valid_i,
    input [PHY_REG_ADDR_WIDTH-1:0] alu1_wb_prd_i, alu2_wb_prd_i, lsu_wb_prd_i,
    input [XLEN-1:0] alu1_wb_data_i, alu2_wb_data_i, lsu_wb_data_i,

    // to LSU
    output lsu_req_valid_o,
    output [LSU_DATA_WIDTH-1:0] rs_lsu_package_o
);

integer i;
reg [MAX_AGE_WIDTH-1:0] age;

// rs field
reg rs_lsu_busy[RS_LSU_SIZE-1:0];
reg rs_lsu_is_load[RS_LSU_SIZE-1:0];
reg rs_lsu_is_store[RS_LSU_SIZE-1:0];
//reg [2:0] rs_op_func3[RS_LSU_SIZE-1:0];
reg [LDU_OP_WIDTH-1:0] rs_lsu_op_ldu_op[RS_LSU_SIZE-1:0];
reg [STU_OP_WIDTH-1:0] rs_lsu_op_stu_op[RS_LSU_SIZE-1:0];
reg rs_lsu_op_aq[RS_LSU_SIZE-1:0];
reg rs_lsu_op_rl[RS_LSU_SIZE-1:0];
reg [ROB_INDEX_WIDTH-1:0] rs_op_robID[RS_LSU_SIZE-1:0];
reg [PHY_REG_ADDR_WIDTH-1:0] rs_prd[RS_LSU_SIZE-1:0];
reg [PHY_REG_ADDR_WIDTH-1:0] rs_prs1[RS_LSU_SIZE-1:0];
reg [PHY_REG_ADDR_WIDTH-1:0] rs_prs2[RS_LSU_SIZE-1:0];
reg [XLEN-1:0] rs_data1[RS_LSU_SIZE-1:0];
reg [XLEN-1:0] rs_data2[RS_LSU_SIZE-1:0];
reg rs_data1_ready[RS_LSU_SIZE-1:0];
reg rs_data2_ready[RS_LSU_SIZE-1:0];
reg [IMM_LEN-1:0] rs_op_imm_data[RS_LSU_SIZE-1:0];
//reg [PC_WIDTH-1:0] rs_op_pc[RS_LSU_SIZE-1:0];
//reg [PC_WIDTH-1:0] rs_op_next_pc[RS_LSU_SIZE-1:0];
//reg [PC_WIDTH-1:0] rs_op_predict_pc[RS_LSU_SIZE-1:0];
reg rs_op_is_fence[RS_LSU_SIZE-1:0];
reg [1:0] rs_op_fence_op[RS_LSU_SIZE-1:0];
reg rs_op_aext[RS_LSU_SIZE-1:0];

reg [MAX_AGE_WIDTH-1:0] rs_age[RS_LSU_SIZE-1:0];

// rs_lsu write ctrl
wire rs_first_write_ready, rs_second_write_ready;
wire do_rs_write_first, do_rs_write_second;
wire [RS_LSU_INDEX_WIDTH-1:0] wr_rs_index_first, wr_rs_index_second;
wire do_rs_lsu_issue, issue_rs_valid;
wire [RS_LSU_INDEX_WIDTH-1:0] issue_rs_index;

wire [RS_LSU_SIZE-1:0] rs_lsu_unused;
generate
    for (genvar j = 0; j < RS_LSU_SIZE; j = j + 1) begin
        assign rs_lsu_unused[j] = !rs_lsu_busy[j];
    end
endgenerate

select2_unused #(
    .RS_SIZE(RS_LSU_SIZE),
    .RS_INDEX_WIDTH(RS_LSU_INDEX_WIDTH)
)unused2_selector(
    .rs_unused_i(rs_lsu_unused),
    .rs_first_write_ready_o(rs_first_write_ready),
    .rs_second_write_ready_o(rs_second_write_ready),
    .wr_rs_index_first_o(wr_rs_index_first),
    .wr_rs_index_second_o(wr_rs_index_second)
);

assign rs_lsu_ready_first_o = rs_first_write_ready;
assign rs_lsu_ready_second_o = rs_second_write_ready;
assign do_rs_write_first = rs_first_write_ready & instr1_valid_i;
assign do_rs_write_second = rs_second_write_ready & instr2_valid_i;

always @(posedge clk) begin 
    if (rst) begin
        for (i = 0; i < RS_LSU_SIZE; i = i + 1) begin
            rs_lsu_busy[i] <= 0;
        end
        age <= 0; 
    end else begin
        if (do_rs_write_first) begin 
            rs_lsu_busy[wr_rs_index_first] <= 1'b1;
        end
        if (do_rs_write_second) begin 
            rs_lsu_busy[wr_rs_index_second] <= 1'b1;
        end
        if (do_rs_lsu_issue) begin
            rs_lsu_busy[issue_rs_index] <= 0;
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
        // rs_op_pc[wr_rs_index_first] <= instr1_pc_i;
        // rs_op_next_pc[wr_rs_index_first] <= instr1_next_pc_i;
        // rs_op_predict_pc[wr_rs_index_first] <= instr1_predict_pc_i;
        rs_op_is_fence[wr_rs_index_first] <= is_fence_first_i;
        rs_op_fence_op[wr_rs_index_first] <= fence_op_first_i;
        rs_op_aext[wr_rs_index_first] <= is_aext_first_i;
        rs_op_imm_data[wr_rs_index_first] <= instr1_imm_data_i;
        // rs_op_func3[wr_rs_index_first] <= instr1_func3_i;
        rs_lsu_is_load[wr_rs_index_first] <= load_first_i;
        rs_lsu_is_store[wr_rs_index_first] <= store_first_i;
        rs_lsu_op_ldu_op[wr_rs_index_first] <= ldu_op_first_i;
        rs_lsu_op_stu_op[wr_rs_index_first] <= stu_op_first_i;
        rs_lsu_op_aq[wr_rs_index_first] <= aq_first_i;
        rs_lsu_op_rl[wr_rs_index_first] <= rl_first_i;
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
        // rs_op_pc[wr_rs_index_second] <= instr2_pc_i;
        // rs_op_next_pc[wr_rs_index_second] <= instr2_next_pc_i;
        // rs_op_predict_pc[wr_rs_index_second] <= instr2_predict_pc_i;
        rs_op_is_fence[wr_rs_index_second] <= is_fence_second_i;
        rs_op_fence_op[wr_rs_index_second] <= fence_op_second_i;
        rs_op_aext[wr_rs_index_second] <= is_aext_second_i;
        rs_op_imm_data[wr_rs_index_second] <= instr2_imm_data_i;
        // rs_op_func3[wr_rs_index_second] <= instr2_func3_i;
        rs_lsu_is_load[wr_rs_index_second] <= load_second_i;
        rs_lsu_is_store[wr_rs_index_second] <= store_second_i;
        rs_lsu_op_ldu_op[wr_rs_index_second] <= ldu_op_second_i;
        rs_lsu_op_stu_op[wr_rs_index_second] <= stu_op_second_i;
        rs_lsu_op_aq[wr_rs_index_second] <= aq_second_i;
        rs_lsu_op_rl[wr_rs_index_second] <= rl_second_i;
        rs_age[wr_rs_index_second] <= age;
    end
end

// bypass
always@(posedge clk) begin
    if (alu1_done_valid_i) begin
        for (i = 0; i < RS_LSU_SIZE; i = i + 1) begin
            if (rs_lsu_busy[i]) begin
                if(!rs_data1_ready[i] && (rs_prs1[i] == alu1_wb_prd_i)) begin
                    rs_data1[i] <= alu1_wb_data_i;
                    rs_data1_ready[i] <= 1'b1;
                end
                if(!rs_data2_ready[i] && (rs_prs2[i] == alu1_wb_prd_i)) begin
                    rs_data2[i] <= alu1_wb_data_i;
                    rs_data2_ready[i] <= 1'b1;
                end
            end
        end
    end
    if (alu2_done_valid_i) begin
        for (i = 0; i < RS_LSU_SIZE; i = i + 1) begin
            if (rs_lsu_busy[i]) begin
                if(!rs_data1_ready[i] && (rs_prs1[i] == alu2_wb_prd_i)) begin
                    rs_data1[i] <= alu2_wb_data_i;
                    rs_data1_ready[i] <= 1'b1;
                end
                if(!rs_data2_ready[i] && (rs_prs2[i] == alu2_wb_prd_i)) begin
                    rs_data2[i] <= alu2_wb_data_i;
                    rs_data2_ready[i] <= 1'b1;
                end
            end
        end
    end
    if (lsu_done_valid_i) begin
        for (i = 0; i < RS_LSU_SIZE; i = i + 1) begin
            if (rs_lsu_busy[i]) begin
                if(!rs_data1_ready[i] && (rs_prs1[i] == lsu_wb_prd_i)) begin
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


// rs to lsu reg
reg lsu_valid;
reg [ROB_INDEX_WIDTH-1:0] issue_rob_index;
reg [PHY_REG_ADDR_WIDTH-1:0] issue_prd_address;
reg [XLEN-1:0] issue_rs1_data, issue_rs2_data;
reg [IMM_LEN-1:0] issue_imm;
reg issue_is_load, issue_is_store;
reg [LDU_OP_WIDTH-1:0] issue_ld_opcode;
reg [STU_OP_WIDTH-1:0] issue_st_opcode;
reg issue_lsu_fence;
reg [1:0] issue_lsu_fence_op; 
reg issue_aext, issue_aq, issue_rl;

wire [LSU_DATA_WIDTH-1:0] rs_lsu_issue_data;

// rs issue 
wire [RS_LSU_SIZE-1:0] rs_select_ready;
generate
    for (genvar j = 0; j < RS_LSU_SIZE; j = j + 1) begin
        assign rs_select_ready[j] = rs_lsu_busy[j] & rs_data1_ready[j] & rs_data2_ready[j];
    end
endgenerate

oldest2_ready #(
    .RS_SIZE(RS_LSU_SIZE),
    .RS_INDEX_WIDTH(RS_LSU_INDEX_WIDTH),
    .AGE_WIDTH(MAX_AGE_WIDTH)
)oldest2_selector(
    .ready_i(rs_select_ready),
    .ages_i(rs_age),
    .first_valid_o(issue_rs_valid),
    .first_index_o(issue_rs_index),
    .second_valid_o(),
    .second_index_o()
);

assign do_rs_lsu_issue = lsu_ready_i & issue_rs_valid;

always @(posedge clk) begin
    if (do_rs_lsu_issue) begin
        lsu_valid <= 1'b1;
        issue_rob_index <= rs_op_robID[issue_rs_index];
        issue_prd_address <= rs_prd[issue_rs_index];
        issue_rs1_data <= rs_data1[issue_rs_index];
        issue_rs2_data <= rs_data2[issue_rs_index];
        issue_imm <= rs_op_imm_data[issue_rs_index];
        issue_is_load <= rs_lsu_is_load[issue_rs_index];
        issue_is_store <= rs_lsu_is_store[issue_rs_index];
        issue_ld_opcode <= rs_lsu_op_ldu_op[issue_rs_index];
        issue_st_opcode <= rs_lsu_op_stu_op[issue_rs_index];
        issue_lsu_fence <= rs_op_is_fence[issue_rs_index];
        issue_lsu_fence_op <= rs_op_fence_op[issue_rs_index];
        issue_aext <= rs_op_aext[issue_rs_index];
        issue_aq <= rs_lsu_op_aq[issue_rs_index];
        issue_rl <= rs_lsu_op_rl[issue_rs_index];
    end
    else begin
        lsu_valid <= 0;
    end
end

// lsu package
assign lsu_req_valid_o = lsu_valid;
assign rs_lsu_issue_data = {issue_rob_index,
                            issue_prd_address,
                            issue_rs1_data,
                            issue_rs2_data,
                            issue_imm,
                            issue_is_load,
                            issue_is_store,
                            issue_ld_opcode,
                            issue_st_opcode,
                            issue_lsu_fence,
                            issue_lsu_fence_op,
                            issue_aext,
                            issue_aq,
                            issue_rl
                            };
assign rs_lsu_package_o = rs_lsu_issue_data;


endmodule