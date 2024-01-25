
module Backend #(
    parameter ROB_SIZE = 16,
    parameter PHY_REG_SIZE = 48,
    parameter REG_ADDR_WIDTH = 5,
    parameter ROB_SIZE_WIDTH = 4,
    parameter ROB_INDEX_WIDTH = 4,
    parameter PC_WIDTH = 39,
    parameter LDU_OP_WIDTH = 4,
    parameter STU_OP_WIDTH = 5,
    parameter PHY_REG_ADDR_WIDTH = 6,
    parameter IMM_LEN = 32,
    parameter XLEN = 64,
    parameter CSR_ADDR_LEN = 12,
    parameter RRT_SIZE = 32,
    parameter FRLIST_SIZE = PHY_REG_SIZE-1,
    parameter FRLIST_SIZE_WIDTH = 6,
    parameter IQ_SIZE = 32,
    parameter IQ_SIZE_WIDTH = 5,
    parameter IQ_WIDTH = ROB_INDEX_WIDTH+PHY_REG_ADDR_WIDTH*3+PC_WIDTH*3+LDU_OP_WIDTH+STU_OP_WIDTH+73,
    parameter MD_DATA_WIDTH = ROB_INDEX_WIDTH+PHY_REG_ADDR_WIDTH+XLEN*2+4,
    parameter LSU_DATA_WIDTH = ROB_INDEX_WIDTH+PHY_REG_ADDR_WIDTH+XLEN*2+IMM_LEN+LDU_OP_WIDTH+STU_OP_WIDTH+8
) (
    input clk                                                   ,
    input rst                                                   ,
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
    input rl_second_i                                           

);

wire [4:0] rs1_address_first_i                          ;
wire [4:0] rs2_address_first_i                          ;
wire [4:0] rd_address_first_i                           ;
wire [4:0] rs1_address_second_i                         ;
wire [4:0] rs2_address_second_i                         ;
wire [4:0] rd_address_second_i                          ;
wire uses_rd_first_i                                    ;
wire do_rob_write_first                                 ;
wire uses_rd_second_i                                   ;
wire do_rob_write_second                                ;
wire [PHY_REG_ADDR_WIDTH-1:0] free_list_rdata_first     ;
wire [PHY_REG_ADDR_WIDTH-1:0] free_list_rdata_second    ;
wire [REG_ADDR_WIDTH-1:0] rob_alu1_wrb_rd_address       ;
wire [REG_ADDR_WIDTH-1:0] rob_alu2_wrb_rd_address       ;
wire [REG_ADDR_WIDTH-1:0] rob_lsu_wrb_rd_address        ;
wire fu2rcu_alu1_resp_valid                             ;
wire fu2rcu_alu2_resp_valid                             ;
wire fu2rcu_lsu_done_valid                              ;
wire [PHY_REG_ADDR_WIDTH-1:0] fu2rcu_alu1_wrb_prd_addr  ;
wire [PHY_REG_ADDR_WIDTH-1:0] fu2rcu_alu2_wrb_prd_addr  ;
wire [PHY_REG_ADDR_WIDTH-1:0] fu2rcu_lsu_wrb_addr       ;
wire [PHY_REG_ADDR_WIDTH-1:0] name_prs1_first           ;
wire [PHY_REG_ADDR_WIDTH-1:0] name_prs2_first           ;
wire [PHY_REG_ADDR_WIDTH-1:0] name_lprd_first           ;
wire [PHY_REG_ADDR_WIDTH-1:0] name_prd_first            ;
wire [PHY_REG_ADDR_WIDTH-1:0] name_prs1_second          ;
wire [PHY_REG_ADDR_WIDTH-1:0] name_prs2_second          ;
wire [PHY_REG_ADDR_WIDTH-1:0] name_lprd_second          ;
wire [PHY_REG_ADDR_WIDTH-1:0] name_prd_second           ;
wire [RRT_SIZE-1:0] prf_or_rob_table                    ;
wire free_list_wr_first_en                              ;
wire free_list_wr_second_en                             ;
wire free_list_rd_first_en                              ;
wire free_list_rd_second_en                             ;
wire [PHY_REG_ADDR_WIDTH-1:0] free_list_wrdata_first    ;
wire [PHY_REG_ADDR_WIDTH-1:0] free_list_wrdata_second   ;
wire free_list_almost_empty                             ;
wire deco_rob_req_ready_first                           ;
wire deco_rob_req_ready_second                          ;
wire fu2rcu_md_wrb_resp_valid                           ;
wire func_csru_done_valid_i                             ;
wire fu2rcu_csr_resp_valid                              ;
wire [ROB_INDEX_WIDTH-1:0] fu2rcu_alu1_wrb_rob_index    ;
wire [ROB_INDEX_WIDTH-1:0] fu2rcu_alu2_wrb_rob_index    ;
wire [ROB_INDEX_WIDTH-1:0] fu2rcu_lsu_comm_rob_index_fix;
wire [ROB_INDEX_WIDTH-1:0] fu2rcu_md_wrb_rob_index      ;
wire [ROB_INDEX_WIDTH-1:0] fu2rcu_csr_wrb_rob_index     ;
wire [IQ_WIDTH-1:0] iq_wr_pkg_first                     ;
wire [IQ_WIDTH-1:0] iq_wr_pkg_second                    ;
wire rs_ready_first                                     ;
wire rs_ready_second                                    ;
wire [IQ_WIDTH-1:0] iq_rd_pkg_first                     ;
wire [IQ_WIDTH-1:0] iq_rd_pkg_second                    ;
wire iq_almost_full                                     ;
wire [ROB_INDEX_WIDTH-1:0] iq_wr_rob_index_first        ;
wire [PHY_REG_ADDR_WIDTH-1:0] iq_prd_first              ;
wire [2:0] iq_fu_function_first                         ;
wire [PC_WIDTH-1:0] iq_pc_first                         ;
wire [PC_WIDTH-1:0] iq_next_pc_first                    ;
wire [PC_WIDTH-1:0] iq_predict_pc_first                 ;
wire [31:0] iq_imm_data_first                           ;
wire [1:0] iq_fu_select_a_first                         ;
wire [1:0] iq_fu_select_b_first                         ;
wire [PHY_REG_ADDR_WIDTH-1:0] iq_prs1_first             ;
wire [PHY_REG_ADDR_WIDTH-1:0] iq_prs2_first             ;
wire iq_is_alu_first                                    ;
wire iq_jump_first                                      ;
wire iq_branch_first                                    ;
wire iq_half_first                                      ;
wire iq_alu_function_modifier_first                     ;
wire iq_is_mext_first                                   ;
wire iq_load_first                                      ;
wire iq_store_first                                     ;
wire [LDU_OP_WIDTH-1:0] iq_ldu_op_first                 ;
wire [STU_OP_WIDTH-1:0] iq_stu_op_first                 ;
wire iq_is_fence_first                                  ;
wire [1:0] iq_fence_op_first                            ;
wire iq_is_aext_first                                   ;
wire iq_aq_first                                        ;
wire iq_rl_first                                        ;
wire iq_is_csr_first                                    ;
wire [11:0] iq_csr_address_first                        ;
wire iq_csr_read_first                                  ;
wire iq_csr_write_first                                 ;
wire iq_uses_rs1_first                                  ;
wire iq_uses_rs2_first                                  ;
wire iq_uses_rd_first                                   ;
wire iq_rs1_wait_first                                  ;
wire iq_rs2_wait_first                                  ;
wire [ROB_INDEX_WIDTH-1:0] iq_wr_rob_index_second       ;
wire [PHY_REG_ADDR_WIDTH-1:0] iq_prd_second             ;
wire [2:0] iq_fu_function_second                        ;
wire [PC_WIDTH-1:0] iq_pc_second                        ;
wire [PC_WIDTH-1:0] iq_next_pc_second                   ;
wire [PC_WIDTH-1:0] iq_predict_pc_second                ;
wire [31:0] iq_imm_data_second                          ;
wire [1:0] iq_fu_select_a_second                        ;
wire [1:0] iq_fu_select_b_second                        ;
wire [PHY_REG_ADDR_WIDTH-1:0] iq_prs1_second            ;
wire [PHY_REG_ADDR_WIDTH-1:0] iq_prs2_second            ;
wire iq_is_alu_second                                   ;
wire iq_jump_second                                     ;
wire iq_branch_second                                   ;
wire iq_half_second                                     ;
wire iq_alu_function_modifier_second                    ;
wire iq_is_mext_second                                  ;
wire iq_load_second                                     ;
wire iq_store_second                                    ;
wire [LDU_OP_WIDTH-1:0] iq_ldu_op_second                ;
wire [STU_OP_WIDTH-1:0] iq_stu_op_second                ;
wire iq_is_fence_second                                 ;
wire [1:0] iq_fence_op_second                           ;
wire iq_is_aext_second                                  ;
wire iq_aq_second                                       ;
wire iq_rl_second                                       ;
wire iq_is_csr_second                                   ;
wire [11:0] iq_csr_address_second                       ;
wire iq_csr_read_second                                 ;
wire iq_csr_write_second                                ;
wire iq_uses_rs1_second                                 ;
wire iq_uses_rs2_second                                 ;
wire iq_uses_rd_second                                  ;
wire iq_rs1_wait_second                                 ;
wire iq_rs2_wait_second                                 ;
wire [63:0] prs1_data_first                             ;
wire [63:0] prs2_data_first                             ;
wire [63:0] prs1_data_second                            ;
wire [63:0] prs2_data_second                            ;
wire [63:0] fu2rcu_alu1_wrb_data                        ;
wire [63:0] fu2rcu_alu2_wrb_data                        ;
wire [63:0] fu2rcu_lsu_comm_data                        ;
wire [ROB_INDEX_WIDTH-1:0] rcu2fu_lsu_rob_index         ;
wire [PHY_REG_ADDR_WIDTH-1:0] rcu2fu_lsu_rd_addr        ;
wire [XLEN-1:0] rcu2fu_agu_virt_base                    ;
wire [XLEN-1:0] rcu2fu_lsu_data                         ;
wire [IMM_LEN-1:0] lsu_imm                              ;
wire lsu_is_load                                        ;
wire lsu_is_store                                       ;
wire [LDU_OP_WIDTH-1:0] rcu2fu_lsu_ld_opcode            ;
wire [STU_OP_WIDTH-1:0] rcu2fu_lsu_st_opcode            ;
wire rcu2fu_lsu_fenced                                  ;
wire [1:0] lsu_lsu_fence_op                             ;
wire lsu_aext                                           ;
wire rcu2fu_lsu_aq                                      ;
wire rcu2fu_lsu_rl                                      ;
wire [XLEN-1:0] rcu2fu_alu1_rs1_data                    ;
wire [XLEN-1:0] rcu2fu_alu2_rs1_data                    ;
wire [XLEN-1:0] rcu2fu_alu1_rs2_data                    ;
wire [XLEN-1:0] rcu2fu_alu2_rs2_data                    ;
wire [IMM_LEN-1:0] rcu2fu_alu1_imm_data                 ;
wire [IMM_LEN-1:0] rcu2fu_alu2_imm_data                 ;
wire [1:0] rcu2fu_alu1_opr1_sel                         ;
wire [1:0] rcu2fu_alu2_opr1_sel                         ;
wire [1:0] rcu2fu_alu1_opr2_sel                         ;
wire [1:0] rcu2fu_alu2_opr2_sel                         ;
wire [ROB_INDEX_WIDTH-1:0] rcu2fu_alu1_rob_index        ;
wire [ROB_INDEX_WIDTH-1:0] rcu2fu_alu2_rob_index        ;
wire [PHY_REG_ADDR_WIDTH-1:0] rcu2fu_alu1_prd_addr      ;
wire [PHY_REG_ADDR_WIDTH-1:0] rcu2fu_alu2_prd_addr      ;
wire rcu2fu_alu1_is_branch                              ;
wire rcu2fu_alu2_is_branch                              ;
wire rcu2fu_alu1_is_jump                                ;
wire rcu2fu_alu2_is_jump                                ;
wire rcu2fu_alu1_req_valid                              ;
wire rcu2fu_alu2_req_valid                              ;
wire rcu2fu_alu1_half                                   ;
wire rcu2fu_alu2_half                                   ;
wire [PC_WIDTH-1:0] rcu2fu_alu1_pc                      ;
wire [PC_WIDTH-1:0] rcu2fu_alu2_pc                      ;
wire [PC_WIDTH-1:0] rcu2fu_alu1_next_pc                 ;
wire [PC_WIDTH-1:0] rcu2fu_alu2_next_pc                 ;
wire [PC_WIDTH-1:0] rcu2fu_alu1_predict_pc              ;
wire [PC_WIDTH-1:0] rcu2fu_alu2_predict_pc              ;
wire [2:0] rcu2fu_alu1_func3                            ;
wire [2:0] rcu2fu_alu2_func3                            ;
wire rcu2fu_alu1_func_modifier                          ;
wire rcu2fu_alu2_func_modifier                          ;
wire fu2rcu_alu1_predict_miss                           ;
wire fu2rcu_alu2_predict_miss                           ;
wire fu2rcu_alu1_branch_taken                           ;
wire fu2rcu_alu2_branch_taken                           ;
wire [PC_WIDTH-1:0] fu2rcu_alu1_final_branch_pc         ;
wire [PC_WIDTH-1:0] fu2rcu_alu2_final_branch_pc         ;
wire fu2rcu_lsu_req_ready                               ;
wire rcu2fu_lsu_req_valid                               ;
wire [XLEN-1:0] rcu2fu_agu_virt_offset                  ;
wire rcu2fu_lsu_ls                                      ;
wire rcu2fu_lsu_fenced_final                            ;
wire [ROB_INDEX_WIDTH-1:0] lsu2rcu_rob_index            ;
wire [LSU_DATA_WIDTH-1:0]  rcu2fu_lsu_package           ;
reg global_wfi                      ;
reg global_trap                     ;
reg global_ret                      ;
reg deco_rob_req_valid_first        ;
reg deco_rob_req_valid_second       ;
reg uses_rs1_first                  ;
reg uses_rs1_second                 ;
reg uses_rs2_first                  ;
reg uses_rs2_second                 ;
reg uses_rd_first                   ;
reg uses_rd_second                  ;
reg uses_csr_first                  ;
reg uses_csr_second                 ;
reg [PC_WIDTH-1:0] pc_first         ;
reg [PC_WIDTH-1:0] pc_second        ;
reg [PC_WIDTH-1:0] next_pc_first    ;
reg [PC_WIDTH-1:0] next_pc_second   ;
reg [PC_WIDTH-1:0] predict_pc_first ;
reg [PC_WIDTH-1:0] predict_pc_second;
reg [4:0] rs1_address_first         ;
reg [4:0] rs1_address_second        ;
reg [4:0] rs2_address_first         ;
reg [4:0] rs2_address_second        ;
reg [4:0] rd_address_first          ;
reg [4:0] rd_address_second         ;
reg [11:0] csr_address_first        ;
reg [11:0] csr_address_second       ;
reg mret_first                      ;
reg mret_second                     ;
reg sret_first                      ;
reg sret_second                     ;
reg wfi_first                       ;
reg wfi_second                      ;
reg half_first                      ;
reg half_second                     ;
reg is_fence_first                  ;
reg is_fence_second                 ;
reg [1:0] fence_op_first            ;
reg [1:0] fence_op_second           ;
reg is_aext_first                   ;
reg is_aext_second                  ;
reg is_mext_first                   ;
reg is_mext_second                  ;
reg csr_read_first                  ;
reg csr_read_second                 ;
reg csr_write_first                 ;
reg csr_write_second                ;
reg [31:0] imm_data_first           ;
reg [31:0] imm_data_second          ;
reg [2:0] fu_function_first         ;
reg [2:0] fu_function_second        ;
reg alu_function_modifier_first     ;
reg alu_function_modifier_second    ;
reg [1:0] fu_select_a_first         ;
reg [1:0] fu_select_a_second        ;
reg [1:0] fu_select_b_first         ;
reg [1:0] fu_select_b_second        ;
reg jump_first                      ;
reg jump_second                     ;
reg branch_first                    ;
reg branch_second                   ;
reg is_alu_first                    ;
reg is_alu_second                   ;
reg load_first                      ;
reg load_second                     ;
reg store_first                     ;
reg store_second                    ;
reg [LDU_OP_WIDTH-1:0] ldu_op_first ;
reg [LDU_OP_WIDTH-1:0] ldu_op_second;
reg [STU_OP_WIDTH-1:0] stu_op_first ;
reg [STU_OP_WIDTH-1:0] stu_op_second;
reg aq_first                        ;
reg aq_second                       ;
reg rl_first                        ;
reg rl_second                       ;

assign deco_rob_req_ready_first_o = deco_rob_req_ready_first & (~free_list_almost_empty) & (iq_almost_full);
assign deco_rob_req_ready_second_o = deco_rob_req_ready_second & (~free_list_almost_empty) & (iq_almost_full);

assign fu2rcu_lsu_comm_rob_index_fix = lsu2rcu_rob_index;

//pipeline regs
always @(posedge clk) begin
    if(rst) begin
        global_wfi                   <= 0;
        global_trap                  <= 0;
        global_ret                   <= 0;     
        deco_rob_req_valid_first     <= 0;
        deco_rob_req_valid_second    <= 0;
        uses_rs1_first               <= 0;
        uses_rs1_second              <= 0;
        uses_rs2_first               <= 0;
        uses_rs2_second              <= 0;
        uses_rd_first                <= 0;
        uses_rd_second               <= 0;
        uses_csr_first               <= 0;
        uses_csr_second              <= 0;
        pc_first                     <= 0;
        pc_second                    <= 0;
        next_pc_first                <= 0;
        next_pc_second               <= 0;
        predict_pc_first             <= 0;
        predict_pc_second            <= 0;
        rs1_address_first            <= 0;
        rs1_address_second           <= 0;
        rs2_address_first            <= 0;
        rs2_address_second           <= 0;
        rd_address_first             <= 0;
        rd_address_second            <= 0;
        csr_address_first            <= 0;
        csr_address_second           <= 0;
        mret_first                   <= 0;
        mret_second                  <= 0;
        sret_first                   <= 0;
        sret_second                  <= 0;
        wfi_first                    <= 0;
        wfi_second                   <= 0;
        half_first                   <= 0;
        half_second                  <= 0;
        is_fence_first               <= 0;
        is_fence_second              <= 0;
        fence_op_first               <= 0;
        fence_op_second              <= 0;
        is_aext_first                <= 0;
        is_aext_second               <= 0;
        is_mext_first                <= 0;
        is_mext_second               <= 0;
        csr_read_first               <= 0;
        csr_read_second              <= 0;
        csr_write_first              <= 0;
        csr_write_second             <= 0;
        imm_data_first               <= 0;
        imm_data_second              <= 0;
        fu_function_first            <= 0;
        fu_function_second           <= 0;
        alu_function_modifier_first  <= 0;
        alu_function_modifier_second <= 0;
        fu_select_a_first            <= 0;
        fu_select_a_second           <= 0;
        fu_select_b_first            <= 0;
        fu_select_b_second           <= 0;
        jump_first                   <= 0;
        jump_second                  <= 0;
        branch_first                 <= 0;
        branch_second                <= 0;
        is_alu_first                 <= 0;
        is_alu_second                <= 0;
        load_first                   <= 0;
        load_second                  <= 0;
        store_first                  <= 0;
        store_second                 <= 0;
        ldu_op_first                 <= 0;
        ldu_op_second                <= 0;
        stu_op_first                 <= 0;
        stu_op_second                <= 0;
        aq_first                     <= 0;
        aq_second                    <= 0;
        rl_first                     <= 0;
        rl_second                    <= 0;
    end else begin
        global_wfi                   <= global_wfi_i                  ;
        global_trap                  <= global_trap_i                 ;
        global_ret                   <= global_ret_i                  ;
        deco_rob_req_valid_first     <= deco_rob_req_valid_first_i    ;
        deco_rob_req_valid_second    <= deco_rob_req_valid_second_i   ;
        uses_rs1_first               <= uses_rs1_first_i              ;
        uses_rs1_second              <= uses_rs1_second_i             ;
        uses_rs2_first               <= uses_rs2_first_i              ;
        uses_rs2_second              <= uses_rs2_second_i             ;
        uses_rd_first                <= uses_rd_first_i               ;
        uses_rd_second               <= uses_rd_second_i              ;
        uses_csr_first               <= uses_csr_first_i              ;
        uses_csr_second              <= uses_csr_second_i             ;
        pc_first                     <= pc_first_i                    ;
        pc_second                    <= pc_second_i                   ;
        next_pc_first                <= next_pc_first_i               ;
        next_pc_second               <= next_pc_second_i              ;
        predict_pc_first             <= predict_pc_first_i            ;
        predict_pc_second            <= predict_pc_second_i           ;
        rs1_address_first            <= rs1_address_first_i           ;
        rs1_address_second           <= rs1_address_second_i          ;
        rs2_address_first            <= rs2_address_first_i           ;
        rs2_address_second           <= rs2_address_second_i          ;
        rd_address_first             <= rd_address_first_i            ;
        rd_address_second            <= rd_address_second_i           ;
        csr_address_first            <= csr_address_first_i           ;
        csr_address_second           <= csr_address_second_i          ;
        mret_first                   <= mret_first_i                  ;
        mret_second                  <= mret_second_i                 ;
        sret_first                   <= sret_first_i                  ;
        sret_second                  <= sret_second_i                 ;
        wfi_first                    <= wfi_first_i                   ;
        wfi_second                   <= wfi_second_i                  ;
        half_first                   <= half_first_i                  ;
        half_second                  <= half_second_i                 ;
        is_fence_first               <= is_fence_first_i              ;
        is_fence_second              <= is_fence_second_i             ;
        fence_op_first               <= fence_op_first_i              ;
        fence_op_second              <= fence_op_second_i             ;
        is_aext_first                <= is_aext_first_i               ;
        is_aext_second               <= is_aext_second_i              ;
        is_mext_first                <= is_mext_first_i               ;
        is_mext_second               <= is_mext_second_i              ;
        csr_read_first               <= csr_read_first_i              ;
        csr_read_second              <= csr_read_second_i             ;
        csr_write_first              <= csr_write_first_i             ;
        csr_write_second             <= csr_write_second_i            ;
        imm_data_first               <= imm_data_first_i              ;
        imm_data_second              <= imm_data_second_i             ;
        fu_function_first            <= fu_function_first_i           ;
        fu_function_second           <= fu_function_second_i          ;
        alu_function_modifier_first  <= alu_function_modifier_first_i ;
        alu_function_modifier_second <= alu_function_modifier_second_i;
        fu_select_a_first            <= fu_select_a_first_i           ;
        fu_select_a_second           <= fu_select_a_second_i          ;
        fu_select_b_first            <= fu_select_b_first_i           ;
        fu_select_b_second           <= fu_select_b_second_i          ;
        jump_first                   <= jump_first_i                  ;
        jump_second                  <= jump_second_i                 ;
        branch_first                 <= branch_first_i                ;
        branch_second                <= branch_second_i               ;
        is_alu_first                 <= is_alu_first_i                ;
        is_alu_second                <= is_alu_second_i               ;
        load_first                   <= load_first_i                  ;
        load_second                  <= load_second_i                 ;
        store_first                  <= store_first_i                 ;
        store_second                 <= store_second_i                ;
        ldu_op_first                 <= ldu_op_first_i                ;
        ldu_op_second                <= ldu_op_second_i               ;
        stu_op_first                 <= stu_op_first_i                ;
        stu_op_second                <= stu_op_second_i               ;
        aq_first                     <= aq_first_i                    ;
        aq_second                    <= aq_second_i                   ;
        rl_first                     <= rl_first_i                    ;
        rl_second                    <= rl_second_i                   ;
    end
end

rrt rrt_u(
    .clk                      (clk),
    .rst                      (rst),
    .rs1_address_first_i      (rs1_address_first_i),
    .rs2_address_first_i      (rs2_address_first_i),
    .rd_address_first_i       (rd_address_first_i),
    .rs1_address_second_i     (rs1_address_second_i),
    .rs2_address_second_i     (rs2_address_second_i),
    .rd_address_second_i      (rd_address_second_i),
    .uses_rd_first_i          (uses_rd_first_i),
    .do_rob_write_first       (do_rob_write_first),
    .uses_rd_second_i         (uses_rd_second_i),
    .do_rob_write_second      (do_rob_write_second),
    .free_list_rdata_first    (free_list_rdata_first),
    .free_list_rdata_second   (free_list_rdata_second),
    .rob_alu1_wrb_rd_address_i(rob_alu1_wrb_rd_address), 
    .rob_alu2_wrb_rd_address_i(rob_alu2_wrb_rd_address), 
    .rob_lsu_wrb_rd_address_i (rob_lsu_wrb_rd_address), 

    .func_alu1_done_valid_i   (fu2rcu_alu1_resp_valid),
    .func_alu2_done_valid_i   (fu2rcu_alu2_resp_valid),
    .func_lsu_done_valid_i    (fu2rcu_lsu_done_valid),
    .physical_alu1_wrb_addr_i (fu2rcu_alu1_wrb_prd_addr), 
    .physical_alu2_wrb_addr_i (fu2rcu_alu2_wrb_prd_addr), 
    .physical_lsu_wrb_addr_i  (fu2rcu_lsu_wrb_addr), 
	
    .name_prs1_first          (name_prs1_first),
    .name_prs2_first          (name_prs2_first),
    .name_lprd_first          (name_lprd_first),
    .name_prd_first           (name_prd_first),
    .name_prs1_second         (name_prs1_second),
    .name_prs2_second         (name_prs2_second),
    .name_lprd_second         (name_lprd_second),
    .name_prd_second          (name_prd_second),
    .prf_or_rob_table         (prf_or_rob_table)
);

freelist #(
    .FIFO_SIZE          (FRLIST_SIZE),
    .FIFO_SIZE_WIDTH    (FRLIST_SIZE_WIDTH),
    .FIFO_DATA_WIDTH    (PHY_REG_ADDR_WIDTH)
) freelist_u(
    .clk                (clk),
    .rst                (rst),
    .wr_first_en_i      (free_list_wr_first_en),
    .wr_second_en_i     (free_list_wr_second_en),
    .rd_first_en_i      (free_list_rd_first_en),
    .rd_second_en_i     (free_list_rd_second_en),
    .wdata_first_i      (free_list_wrdata_first),
    .wdata_second_i     (free_list_wrdata_second),
    .rdata_first_o      (free_list_rdata_first),
    .rdata_second_o     (free_list_rdata_second),
    .fifo_full_o        (),
    .fifo_almost_full_o (),
    .fifo_empty_o       (),
    .fifo_almost_empty_o(free_list_almost_empty),
    .fifo_num_o         ()
);

rob rob_u(
    .clk                            (clk                         ),
    .rst                            (rst                         ),
    //global ctrl
    .global_wfi_i                   (global_wfi                  ),
    .global_trap_i                  (global_trap                 ),
    .global_ret_i                   (global_ret                  ),
    //hand shake with decode        								
    .deco_rob_req_valid_first_i     (deco_rob_req_valid_first    ),
    .deco_rob_req_valid_second_i    (deco_rob_req_valid_second   ),
    .deco_rob_req_ready_first_o     (deco_rob_req_ready_first    ),
    .deco_rob_req_ready_second_o    (deco_rob_req_ready_second   ),
    .do_rob_commit_first_o          (do_rob_commit_first_o),
    .do_rob_commit_second_o         (do_rob_commit_second_o),
    .rob_cmt_pc_first_o             (rob_cmt_pc_first_o),
    .rob_cmt_pc_second_o            (rob_cmt_pc_second_o),
    //from decode                   								
    .uses_rs1_first_i               (uses_rs1_first              ),
    .uses_rs1_second_i              (uses_rs1_second             ),
    .uses_rs2_first_i               (uses_rs2_first              ),
    .uses_rs2_second_i              (uses_rs2_second             ),
    .uses_rd_first_i                (uses_rd_first               ),
    .uses_rd_second_i               (uses_rd_second              ),
    .uses_csr_first_i               (uses_csr_first              ),
    .uses_csr_second_i              (uses_csr_second             ),
    .pc_first_i                     (pc_first                    ),
    .pc_second_i                    (pc_second                   ),
    .next_pc_first_i                (next_pc_first               ),
    .next_pc_second_i               (next_pc_second              ),
    .predict_pc_first_i             (predict_pc_first            ),
    .predict_pc_second_i            (predict_pc_second           ),
    .rs1_address_first_i            (rs1_address_first           ),
    .rs1_address_second_i           (rs1_address_second          ),
    .rs2_address_first_i            (rs2_address_first           ),
    .rs2_address_second_i           (rs2_address_second          ),
    .rd_address_first_i             (rd_address_first            ),
    .rd_address_second_i            (rd_address_second           ),
    .csr_address_first_i            (csr_address_first           ),
    .csr_address_second_i           (csr_address_second          ),
    .mret_first_i                   (mret_first                  ),
    .mret_second_i                  (mret_second                 ),
    .sret_first_i                   (sret_first                  ),
    .sret_second_i                  (sret_second                 ),
    .wfi_first_i                    (wfi_first                   ),
    .wfi_second_i                   (wfi_second                  ),
    .half_first_i                   (half_first                  ),
    .half_second_i                  (half_second                 ),
    .is_fence_first_i               (is_fence_first              ),
    .is_fence_second_i              (is_fence_second             ),
    .fence_op_first_i               (fence_op_first              ),
    .fence_op_second_i              (fence_op_second             ),
    .is_aext_first_i                (is_aext_first               ),
    .is_aext_second_i               (is_aext_second              ),
    .is_mext_first_i                (is_mext_first               ),
    .is_mext_second_i               (is_mext_second              ),
    .csr_read_first_i               (csr_read_first              ),
    .csr_read_second_i              (csr_read_second             ),
    .csr_write_first_i              (csr_write_first             ),
    .csr_write_second_i             (csr_write_second            ),
    .imm_data_first_i               (imm_data_first              ),
    .imm_data_second_i              (imm_data_second             ),
    .fu_function_first_i            (fu_function_first           ),
    .fu_function_second_i           (fu_function_second          ),
    .alu_function_modifier_first_i  (alu_function_modifier_first ),
    .alu_function_modifier_second_i (alu_function_modifier_second),
    .fu_select_a_first_i            (fu_select_a_first           ),
    .fu_select_a_second_i           (fu_select_a_second          ),
    .fu_select_b_first_i            (fu_select_b_first           ),
    .fu_select_b_second_i           (fu_select_b_second          ),
    .jump_first_i                   (jump_first                  ),
    .jump_second_i                  (jump_second                 ),
    .branch_first_i                 (branch_first                ),
    .branch_second_i                (branch_second               ),
    .is_alu_first_i                 (is_alu_first                ),
    .is_alu_second_i                (is_alu_second               ),
    .load_first_i                   (load_first                  ),
    .load_second_i                  (load_second                 ),
    .store_first_i                  (store_first                 ),
    .store_second_i                 (store_second                ),
    .ldu_op_first_i                 (ldu_op_first                ),
    .ldu_op_second_i                (ldu_op_second               ),
    .stu_op_first_i                 (stu_op_first                ),
    .stu_op_second_i                (stu_op_second               ),
    .aq_first_i                     (aq_first                    ),
    .aq_second_i                    (aq_second                   ),
    .rl_first_i                     (rl_first                    ),
    .rl_second_i                    (rl_second                   ),

    //from fu
    .func_alu1_done_valid_i                (fu2rcu_alu1_resp_valid),
    .func_alu2_done_valid_i                (fu2rcu_alu2_resp_valid),
    .func_lsu_done_valid_i                 (fu2rcu_lsu_done_valid),
    .func_md_done_valid_i                  (fu2rcu_md_wrb_resp_valid),
    .func_csru_done_valid_i                (fu2rcu_csr_resp_valid),
    .func_alu1_rob_index_i                 (fu2rcu_alu1_wrb_rob_index),
    .func_alu2_rob_index_i                 (fu2rcu_alu2_wrb_rob_index),
    .func_lsu_rob_index_i                  (fu2rcu_lsu_comm_rob_index_fix),
    .func_md_rob_index_i                   (fu2rcu_md_wrb_rob_index),
    .func_csru_rob_index_i                 (fu2rcu_csr_wrb_rob_index),

    .name_prs1_first                       (name_prs1_first),
    .name_prs2_first                       (name_prs2_first),
    .name_lprd_first                       (name_lprd_first),
    .name_prd_first                        (name_prd_first),
    .name_prs1_second                      (name_prs1_second),
    .name_prs2_second                      (name_prs2_second),
    .name_lprd_second                      (name_lprd_second),
    .name_prd_second                       (name_prd_second),
    .prf_or_rob_table                      (prf_or_rob_table),

    .free_list_wr_first_en                 (free_list_wr_first_en),
    .free_list_wr_second_en                (free_list_wr_second_en),
    .free_list_rd_first_en                 (free_list_rd_first_en),
    .free_list_rd_second_en                (free_list_rd_second_en),
    .free_list_wrdata_first                (free_list_wrdata_first),
    .free_list_wrdata_second               (free_list_wrdata_second),

    //to renaming table
    .do_rob_write_first                    (do_rob_write_first),
    .do_rob_write_second                   (do_rob_write_second),
    .rob_alu1_wrb_rd_address_o             (rob_alu1_wrb_rd_address),
    .rob_alu2_wrb_rd_address_o             (rob_alu2_wrb_rd_address),
    .rob_lsu_wrb_rd_address_o              (rob_lsu_wrb_rd_address),
    //to issue queue
    .issue_queue_pkg_first                 (iq_wr_pkg_first),
    .issue_queue_pkg_second                (iq_wr_pkg_second)
);

f2if2o #(
    .FIFO_SIZE(IQ_SIZE),
    .FIFO_SIZE_WIDTH(IQ_SIZE_WIDTH),
    .FIFO_DATA_WIDTH(IQ_WIDTH)
) issue_queue_u(
    .clk                (clk),
    .rst                (rst),
    .wr_first_en_i      (do_rob_write_first),
    .wr_second_en_i     (do_rob_write_second),
    .rd_first_en_i      (rs_ready_first),
    .rd_second_en_i     (rs_ready_second),
    .wdata_first_i      (iq_wr_pkg_first),
    .wdata_second_i     (iq_wr_pkg_second),
    .rdata_first_o      (iq_rd_pkg_first),
    .rdata_second_o     (iq_rd_pkg_second),
    .fifo_full_o        (),
    .fifo_almost_full_o (iq_almost_full),
    .fifo_empty_o       (),
    .fifo_almost_empty_o(),
    .fifo_num_o         ()
);

//issue queue signals
assign {
    iq_wr_rob_index_first,
    iq_prd_first,
    iq_fu_function_first,
    iq_pc_first,
    iq_next_pc_first,
    iq_predict_pc_first,
    iq_imm_data_first,
    iq_fu_select_a_first,
    iq_fu_select_b_first,
    iq_prs1_first,
    iq_prs2_first,
    iq_is_alu_first,
    iq_jump_first,
    iq_branch_first,
    iq_half_first,
    iq_alu_function_modifier_first,
    iq_is_mext_first,//
    iq_load_first,
    iq_store_first,
    iq_ldu_op_first,
    iq_stu_op_first,
    iq_is_fence_first,
    iq_fence_op_first,
    iq_is_aext_first,
    iq_aq_first,
    iq_rl_first,
    iq_is_csr_first,//
    iq_csr_address_first,//
    iq_csr_read_first,//
    iq_csr_write_first,//
    iq_uses_rs1_first,
    iq_uses_rs2_first,
    iq_uses_rd_first,
    iq_rs1_wait_first,
    iq_rs2_wait_first
} = iq_rd_pkg_first;

assign {
    iq_wr_rob_index_second,
    iq_prd_second,
    iq_fu_function_second,
    iq_pc_second,
    iq_next_pc_second,
    iq_predict_pc_second,
    iq_imm_data_second,
    iq_fu_select_a_second,
    iq_fu_select_b_second,
    iq_prs1_second,
    iq_prs2_second,
    iq_is_alu_second,
    iq_jump_second,
    iq_branch_second,
    iq_half_second,
    iq_alu_function_modifier_second,
    iq_is_mext_second,//
    iq_load_second,
    iq_store_second,
    iq_ldu_op_second,
    iq_stu_op_second,
    iq_is_fence_second,
    iq_fence_op_second,
    iq_is_aext_second,
    iq_aq_second,
    iq_rl_second,
    iq_is_csr_second,//
    iq_csr_address_second,//
    iq_csr_read_second,//
    iq_csr_write_second,//
    iq_uses_rs1_second,
    iq_uses_rs2_second,
    iq_uses_rd_second,
    iq_rs1_wait_second,
    iq_rs2_wait_second
} = iq_rd_pkg_second;

physical_regfile physical_regfile_u(
    .clk                   (clk),
    .rst                   (rst),
    // from rcu (read ports)
    .prs1_address_first_i  (iq_prs1_first),
    .prs2_address_first_i  (iq_prs2_first),
    .prs1_address_second_i (iq_prs1_second),
    .prs2_address_second_i (iq_prs2_second),
    // to rcu (read ports)
    .prs1_data_first_o     (prs1_data_first),
    .prs2_data_first_o     (prs2_data_first),
    .prs1_data_second_o    (prs1_data_second),
    .prs2_data_second_o    (prs2_data_second),
    // Quadruple write port
    .alu1_wrb_address_i    (fu2rcu_alu1_wrb_prd_addr),
    .alu2_wrb_address_i    (fu2rcu_alu2_wrb_prd_addr),
    .lsu_wrb_address_i     (fu2rcu_lsu_wrb_addr),
    .md_wrb_address_i      (0),
    .alu1_wrb_data_i       (fu2rcu_alu1_wrb_data),
    .alu2_wrb_data_i       (fu2rcu_alu2_wrb_data),
    .lsu_wrb_data_i        (fu2rcu_lsu_comm_data),
    .md_wrb_data_i         (0),
    .alu1_rcu_resp_valid_i (fu2rcu_alu1_resp_valid),
    .alu2_rcu_resp_valid_i (fu2rcu_alu2_resp_valid),
    .lsu_rcu_resp_valid_i  (fu2rcu_lsu_done_valid),
    .md_rcu_resp_valid_i   (1'b0)
);

rs rs_u(
    .clk(clk),
    .rst(rst),

    .instr1_valid_i(1'b1),
    .instr2_valid_i(1'b1),
    .instr1_robID_i(iq_wr_rob_index_first),
    .instr2_robID_i(iq_wr_rob_index_second),
    .instr1_pc_i(iq_pc_first),
    .instr2_pc_i(iq_pc_second),
    .instr1_next_pc_i(iq_next_pc_first),
    .instr2_next_pc_i(iq_next_pc_second),
    .instr1_predict_pc_i(iq_predict_pc_first),
    .instr2_predict_pc_i(iq_predict_pc_second),
    .instr1_rs1_use_i(iq_uses_rs1_first),
    .instr1_rs2_use_i(iq_uses_rs2_first),
    .instr2_rs1_use_i(iq_uses_rs1_second),
    .instr2_rs2_use_i(iq_uses_rs2_second),
    .instr1_prd_i(iq_prd_first),
    .instr2_prd_i(iq_prd_second),
    .instr1_prs1_i(iq_prs1_first), 
    .instr1_prs2_i(iq_prs2_first),
    .instr2_prs1_i(iq_prs1_second),
    .instr2_prs2_i(iq_prs2_second),
    .instr1_rs1_ready_i(~iq_rs1_wait_first),
    .instr1_rs2_ready_i(~iq_rs2_wait_first),
    .instr2_rs1_ready_i(~iq_rs1_wait_second),
    .instr2_rs2_ready_i(~iq_rs2_wait_second),
    .instr1_data1_i(prs1_data_first),
    .instr1_data2_i(prs2_data_first),
    .instr2_data1_i(prs1_data_second),
    .instr2_data2_i(prs2_data_second),
    .instr1_imm_data_i(iq_imm_data_first),
    .instr2_imm_data_i(iq_imm_data_second),
    .instr1_func3_i(iq_fu_function_first),
    .instr2_func3_i(iq_fu_function_second),
    .alu_function_modifier_first_i(iq_alu_function_modifier_first),
    .alu_function_modifier_second_i(iq_alu_function_modifier_second),
    .fu_select_a_first_i(iq_fu_select_a_first),
    .fu_select_a_second_i(iq_fu_select_a_second),
    .fu_select_b_first_i(iq_fu_select_b_first),
    .fu_select_b_second_i(iq_fu_select_b_second),
    .half_first_i(iq_half_first),
    .half_second_i(iq_half_second),
    .jump_first_i(iq_jump_first),
    .jump_second_i(iq_jump_second),
    .branch_first_i(iq_branch_first),
    .branch_second_i(iq_branch_second),
    .is_alu_first_i(iq_is_alu_first),
    .is_alu_second_i(iq_is_alu_second),
    .is_fence_first_i(iq_is_fence_first),
    .is_fence_second_i(iq_is_fence_second),
    .fence_op_first_i(iq_fence_op_first),
    .fence_op_second_i(iq_fence_op_second),
    .is_aext_first_i(iq_is_aext_first),
    .is_aext_second_i(iq_is_aext_second),
    .load_first_i(iq_load_first),
    .load_second_i(iq_load_second),
    .store_first_i(iq_store_first),
    .store_second_i(iq_store_second),
    .ldu_op_first_i(iq_ldu_op_first),
    .ldu_op_second_i(iq_ldu_op_second),
    .stu_op_first_i(iq_stu_op_first),
    .stu_op_second_i(iq_stu_op_second),
    .aq_first_i(iq_aq_first),
    .aq_second_i(iq_aq_second),
    .rl_first_i(iq_rl_first),
    .rl_second_i(iq_rl_second),
    .rs_ready_first_o(rs_ready_first),
    .rs_ready_second_o(rs_ready_second),

    .lsu_ready_i(fu2rcu_lsu_req_ready),
    .alu1_done_valid_i(fu2rcu_alu1_resp_valid),
    .alu2_done_valid_i(fu2rcu_alu2_resp_valid),
    .lsu_done_valid_i(fu2rcu_lsu_done_valid),
    .alu1_wb_prd_i(fu2rcu_alu1_wrb_prd_addr), 
    .alu2_wb_prd_i(fu2rcu_alu2_wrb_prd_addr), 
    .lsu_wb_prd_i(fu2rcu_lsu_wrb_addr),
    .alu1_wb_data_i(fu2rcu_alu1_wrb_data), 
    .alu2_wb_data_i(fu2rcu_alu2_wrb_data), 
    .lsu_wb_data_i(fu2rcu_lsu_comm_data), 
    
    .alu1_req_valid_o(rcu2fu_alu1_req_valid),
    .alu2_req_valid_o(rcu2fu_alu2_req_valid),
    .lsu_req_valid_o(rcu2fu_lsu_req_valid),

    .alu1_robID_o(rcu2fu_alu1_rob_index),
    .alu2_robID_o(rcu2fu_alu2_rob_index),
    .alu1_prd_o(rcu2fu_alu1_prd_addr),
    .alu2_prd_o(rcu2fu_alu2_prd_addr),
    .alu1_func3_o(rcu2fu_alu1_func3),
    .alu2_func3_o(rcu2fu_alu2_func3),
    .alu1_pc_o(rcu2fu_alu1_pc),
    .alu2_pc_o(rcu2fu_alu2_pc), 
    .alu1_next_pc_o(rcu2fu_alu1_next_pc),
    .alu2_next_pc_o(rcu2fu_alu2_next_pc),
    .alu1_predict_pc_o(rcu2fu_alu1_predict_pc),
    .alu2_predict_pc_o(rcu2fu_alu2_predict_pc),
    .alu1_imm_data_o(rcu2fu_alu1_imm_data),
    .alu2_imm_data_o(rcu2fu_alu2_imm_data),
    .alu1_rs1_data_o(rcu2fu_alu1_rs1_data),
    .alu1_rs2_data_o(rcu2fu_alu1_rs2_data),
    .alu2_rs1_data_o(rcu2fu_alu2_rs1_data),
    .alu2_rs2_data_o(rcu2fu_alu2_rs2_data),
    .alu1_select_a_o(rcu2fu_alu1_opr1_sel),
    .alu1_select_b_o(rcu2fu_alu1_opr2_sel),
    .alu2_select_a_o(rcu2fu_alu2_opr1_sel),
    .alu2_select_b_o(rcu2fu_alu2_opr2_sel),
    .alu1_half_o(rcu2fu_alu1_half),
    .alu1_jump_o(rcu2fu_alu1_is_jump),
    .alu1_branch_o(rcu2fu_alu1_is_branch),
    .alu1_func_modifier_o(rcu2fu_alu1_func_modifier),
    .alu2_jump_o(rcu2fu_alu2_is_jump),
    .alu2_branch_o(rcu2fu_alu2_is_branch),
    .alu2_half_o(rcu2fu_alu2_half),
    .alu2_func_modifier_o(rcu2fu_alu2_func_modifier),
    .rs_lsu_package_o(rcu2fu_lsu_package)
); // end rs

assign {rcu2fu_lsu_rob_index, 
        rcu2fu_lsu_rd_addr, 
        rcu2fu_agu_virt_base, // rs1_data
        rcu2fu_lsu_data, 
        lsu_imm,
        lsu_is_load,
        lsu_is_store, 
        rcu2fu_lsu_ld_opcode,
        rcu2fu_lsu_st_opcode,
        rcu2fu_lsu_fenced,       
        lsu_lsu_fence_op, //Not used
        lsu_aext,        //Not used
        rcu2fu_lsu_aq,
        rcu2fu_lsu_rl
        } = rcu2fu_lsu_package;

assign rcu2fu_agu_virt_offset = {{32{lsu_imm[31]}},lsu_imm};
assign rcu2fu_lsu_ls = lsu_is_store;
assign rcu2fu_lsu_fenced_final = rcu2fu_lsu_fenced | rcu2fu_lsu_aq   | rcu2fu_lsu_rl;

fu fu_u(
    .clk(clk),
    .rstn(rst),

    .rcu_fu_alu1_rs1_i(rcu2fu_alu1_rs1_data),
    .rcu_fu_alu2_rs1_i(rcu2fu_alu2_rs1_data),
    .rcu_fu_alu1_rs2_i(rcu2fu_alu1_rs2_data),
    .rcu_fu_alu2_rs2_i(rcu2fu_alu2_rs2_data),
    .rcu_fu_alu1_imm_data_i(rcu2fu_alu1_imm_data),
    .rcu_fu_alu2_imm_data_i(rcu2fu_alu2_imm_data),
    .rcu_fu_alu1_opr1_sel_i(rcu2fu_alu1_opr1_sel),
    .rcu_fu_alu2_opr1_sel_i(rcu2fu_alu2_opr1_sel),
    .rcu_fu_alu1_opr2_sel_i(rcu2fu_alu1_opr2_sel),
    .rcu_fu_alu2_opr2_sel_i(rcu2fu_alu2_opr2_sel),
    .rcu_fu_alu1_rob_index_i(rcu2fu_alu1_rob_index),
    .rcu_fu_alu2_rob_index_i(rcu2fu_alu2_rob_index),
    .rcu_fu_alu1_prd_addr_i(rcu2fu_alu1_prd_addr),
    .rcu_fu_alu2_prd_addr_i(rcu2fu_alu2_prd_addr),
    .rcu_fu_alu1_is_branch_i(rcu2fu_alu1_is_branch),
    .rcu_fu_alu2_is_branch_i(rcu2fu_alu2_is_branch),
    .rcu_fu_alu1_is_jump_i(rcu2fu_alu1_is_jump),
    .rcu_fu_alu2_is_jump_i(rcu2fu_alu2_is_jump),
    .rcu_fu_alu1_req_valid_i(rcu2fu_alu1_req_valid),
    .rcu_fu_alu2_req_valid_i(rcu2fu_alu2_req_valid),
    .rcu_fu_alu1_half_i(rcu2fu_alu1_half),
    .rcu_fu_alu2_half_i(rcu2fu_alu2_half),
    .rcu_fu_alu1_pc_i(rcu2fu_alu1_pc),
    .rcu_fu_alu2_pc_i(rcu2fu_alu2_pc),
    .rcu_fu_alu1_next_pc_i(rcu2fu_alu1_next_pc),
    .rcu_fu_alu2_next_pc_i(rcu2fu_alu2_next_pc),
    .rcu_fu_alu1_predict_pc_i(rcu2fu_alu1_predict_pc),
    .rcu_fu_alu2_predict_pc_i(rcu2fu_alu2_predict_pc),
    .rcu_fu_alu1_func3_i(rcu2fu_alu1_func3),
    .rcu_fu_alu2_func3_i(rcu2fu_alu2_func3),
    .rcu_fu_alu1_func_modifier_i(rcu2fu_alu1_func_modifier),
    .rcu_fu_alu2_func_modifier_i(rcu2fu_alu2_func_modifier),

    .fu_rcu_alu1_resp_valid_o(fu2rcu_alu1_resp_valid),
    .fu_rcu_alu2_resp_valid_o(fu2rcu_alu2_resp_valid),
    .fu_rcu_alu1_wrb_rob_index_o(fu2rcu_alu1_wrb_rob_index),
    .fu_rcu_alu2_wrb_rob_index_o(fu2rcu_alu2_wrb_rob_index),
    .fu_rcu_alu1_wrb_prd_addr_o(fu2rcu_alu1_wrb_prd_addr),
    .fu_rcu_alu2_wrb_prd_addr_o(fu2rcu_alu2_wrb_prd_addr),
    .fu_rcu_alu1_wrb_data_o(fu2rcu_alu1_wrb_data),
    .fu_rcu_alu2_wrb_data_o(fu2rcu_alu2_wrb_data),
    .fu_rcu_alu1_branch_predict_miss_o(fu2rcu_alu1_predict_miss),
    .fu_rcu_alu2_branch_predict_miss_o(fu2rcu_alu2_predict_miss),
    .fu_rcu_alu1_branch_taken_o(fu2rcu_alu1_branch_taken),
    .fu_rcu_alu2_branch_taken_o(fu2rcu_alu2_branch_taken),
    .fu_rcu_alu1_final_next_pc_o(fu2rcu_alu1_final_branch_pc),
    .fu_rcu_alu2_final_next_pc_o(fu2rcu_alu2_final_branch_pc),

    .lsu_rdy_o(fu2rcu_lsu_req_ready),
    .rcu_fu_lsu_vld_i(rcu2fu_lsu_req_valid),
    .rcu_fu_lsu_rob_index_i(rcu2fu_lsu_rob_index),
    .rcu_fu_lsu_rd_addr_i(rcu2fu_lsu_rd_addr),
    .rcu_fu_agu_virt_base_i(rcu2fu_agu_virt_base),
    .rcu_fu_lsu_data_i(rcu2fu_lsu_data),
    .rcu_fu_agu_virt_offset_i(rcu2fu_agu_virt_offset),
    .rcu_fu_lsu_ls_i(rcu2fu_lsu_ls),
    .rcu_fu_lsu_ld_opcode_i(rcu2fu_lsu_ld_opcode),
    .rcu_fu_lsu_st_opcode_i(rcu2fu_lsu_st_opcode),
    .rcu_fu_lsu_fenced_i(rcu2fu_lsu_fenced_final),
    
    .fu_rcu_lsu_comm_vld_o(fu2rcu_lsu_done_valid),
    .fu_rcu_lsu_comm_rob_index_o(lsu2rcu_rob_index),
    .fu_rcu_lsu_comm_rd_addr_o(fu2rcu_lsu_wrb_addr),
    .fu_rcu_lsu_comm_data_o(fu2rcu_lsu_comm_data)
); //end fu

reg [MAX_AGE_WIDTH-1:0] commit_cnt;

always @ (posedge clk) begin
    if (rst) begin
        commit_cnt <= 32'b0;
    end
    else begin
        if (do_rob_commit_first_o & do_rob_commit_second_o) begin
            commit_cnt <= commit_cnt + 32'd2;
        end
        else if (do_rob_commit_first_o ^ do_rob_commit_second_o) begin
            commit_cnt <= commit_cnt + 32'd1;
        end
    end
end

endmodule
