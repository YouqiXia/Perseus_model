#include <algorithm>

#include "sparta/utils/LogUtils.hpp"

#include "PerfectFrontend.hpp"

namespace TimingModel {

    const char* PerfectFrontend::name = "perfect_frontend";

    PerfectFrontend::PerfectFrontend(sparta::TreeNode* node, const PerfectFrontendParameter* p) : 
        sparta::Unit(node),
        node_(node),
        issue_num_(p->issue_num),
        mavis_facade_(getMavis(node))
    {
        inst_generator_ = InstGenerator::createGenerator(mavis_facade_, p->input_file, false);
        backend_fetch_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFrontend, AcceptCredit, Credit));

        dut_ = new VBackend;
        Verilated::traceEverOn(true);
        m_trace_ = new VerilatedVcdC;
        dut_->trace(m_trace_, 7);
        m_trace_->open("waveform.vcd");

        /* Init backend signals */
        dut_->clk = 0;
        dut_->rst = 0;
        dut_->global_wfi_i = 0;
        dut_->global_trap_i = 0;
        dut_->global_ret_i = 0;
        dut_->deco_rob_req_valid_first_i = 0;
        dut_->deco_rob_req_valid_second_i = 0;
        dut_->uses_rs1_first_i = 0;
        dut_->uses_rs1_second_i = 0;
        dut_->uses_rs2_first_i = 0;
        dut_->uses_rs2_second_i = 0;
        dut_->uses_rd_first_i = 0;
        dut_->uses_rd_second_i = 0;
        dut_->uses_csr_first_i = 0;
        dut_->uses_csr_second_i = 0;
        dut_->pc_first_i = 0;
        dut_->pc_second_i = 0;
        dut_->next_pc_first_i = 0;
        dut_->next_pc_second_i = 0;
        dut_->rs1_address_first_i = 0;
        dut_->rs1_address_second_i = 0;
        dut_->rs2_address_first_i = 0;
        dut_->rs2_address_second_i = 0;
        dut_->rd_address_first_i = 0;
        dut_->rd_address_second_i = 0;
        dut_->csr_address_first_i = 0;
        dut_->csr_address_second_i = 0;
        dut_->mret_first_i = 0;
        dut_->mret_second_i = 0;
        dut_->sret_first_i = 0;
        dut_->sret_second_i = 0;
        dut_->wfi_first_i = 0;
        dut_->wfi_second_i = 0;
        dut_->half_first_i = 0;
        dut_->half_second_i = 0;
        dut_->is_fence_first_i = 0;
        dut_->is_fence_second_i = 0;
        dut_->fence_op_first_i = 0;
        dut_->fence_op_second_i = 0;
        dut_->is_aext_first_i = 0;
        dut_->is_aext_second_i = 0;
        dut_->is_mext_first_i = 0;
        dut_->is_mext_second_i = 0;
        dut_->csr_read_first_i = 0;
        dut_->csr_read_second_i = 0;
        dut_->csr_write_first_i = 0;
        dut_->csr_write_second_i = 0;
        dut_->imm_data_first_i = 0;
        dut_->imm_data_second_i = 0;
        dut_->fu_function_first_i = 0;
        dut_->fu_function_second_i = 0;
        dut_->alu_function_modifier_first_i = 0;
        dut_->alu_function_modifier_second_i = 0;
        dut_->fu_select_a_first_i = 0;
        dut_->fu_select_a_second_i = 0;
        dut_->fu_select_b_first_i = 0;
        dut_->fu_select_b_second_i = 0;
        dut_->jump_first_i = 0;
        dut_->jump_second_i = 0;
        dut_->branch_first_i = 0;
        dut_->branch_second_i = 0;
        dut_->is_alu_first_i = 0;
        dut_->is_alu_second_i = 0;
        dut_->load_first_i = 0;
        dut_->load_second_i = 0;
        dut_->store_first_i = 0;
        dut_->store_second_i = 0;
        dut_->ldu_op_first_i = 0;
        dut_->ldu_op_second_i = 0;
        dut_->stu_op_first_i = 0;
        dut_->stu_op_second_i = 0;
        dut_->aq_first_i = 0;
        dut_->aq_second_i = 0;
        dut_->rl_first_i = 0;
        dut_->rl_second_i = 0;

    }

    void PerfectFrontend::AcceptCredit(const Credit& credit) {
        credit_ += credit;

        ILOG("perfect frontend get credits from backend: " << credit);

        produce_inst_event_.schedule(sparta::Clock::Cycle(0));
    }

    void PerfectFrontend::ProduceInst() {
        uint64_t produce_num = std::min(issue_num_, credit_);
        InstGroupPtr inst_group_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        if (!produce_num) { 
            return; 
        }
        while(produce_num--) {
            if (!inst_generator_) {
                return;
            }
            InstPtr dinst;
            dinst = inst_generator_->getNextInst(getClock());

            if(nullptr == dinst) {
                return;
            }
            inst_group_ptr->emplace_back(dinst);
            --credit_;
        }

        if (credit_) {
            produce_inst_event_.schedule(1);
        }
        ILOG("perfect frontend send " << inst_group_ptr->size() << " instructions to backend");
        fetch_backend_inst_out.send(inst_group_ptr);

        /* Verilator feed signals into backend RTL */
        int index = 0;
        dut_->clk = 1;
        for (auto & inst : *inst_group_ptr) {
            if (index == 0) {
                dut_->deco_rob_req_valid_first_i = 1;
                dut_->uses_rs1_first_i = inst->getIsaRs1() != 0;
                dut_->uses_rs2_first_i = inst->getIsaRs2() != 0;
                dut_->uses_rd_first_i = inst->getIsaRd() != 0;
                dut_->uses_csr_first_i = inst->getFuType() == CSR;
                dut_->pc_first_i = inst->getPc();
                dut_->next_pc_first_i = inst->getPc() + 4;
                dut_->rs1_address_first_i = inst->getIsaRs1();
                dut_->rs2_address_first_i = inst->getIsaRs2();
                dut_->rd_address_first_i = inst->getIsaRd();
                dut_->imm_data_first_i = inst->getImmediate();
                if (inst->getFuType() == ALU) {
                    dut_->is_alu_first_i = 1;
                    if (inst->getSubOp() == ALU_ADD || inst->getSubOp() == ALU_SUB || inst->getSubOp() == ALU_ADDW) {
                        dut_->fu_function_first_i = 0;
                        if (inst->getSubOp() == ALU_SUB || inst->getSubOp() == ALU_SUBW) {
                            dut_->alu_function_modifier_first_i = 1;
                        }
                    }
                    else if (inst->getSubOp() == ALU_SLL || inst->getSubOp() == ALU_SLLW) {
                        dut_->fu_function_first_i = 1;
                    }
                    else if (inst->getSubOp() == ALU_SLT) {
                        dut_->fu_function_first_i = 2;
                    }
                    else if (inst->getSubOp() == ALU_SLTU) {
                        dut_->fu_function_first_i = 3;
                    }
                    else if (inst->getSubOp() == ALU_XOR) {
                        dut_->fu_function_first_i = 4;
                    }
                    else if (inst->getSubOp() == ALU_SRL || inst->getSubOp() == ALU_SRA || inst->getSubOp() == ALU_SRLW || inst->getSubOp() == ALU_SRAW) {
                        dut_->fu_function_first_i = 5;
                        if (inst->getSubOp() == ALU_SRA || inst->getSubOp() == ALU_SRAW) {
                            dut_->alu_function_modifier_first_i = 1;
                        }
                    }
                    else if (inst->getSubOp() == ALU_OR) {
                        dut_->fu_function_first_i = 6;
                    }
                    else if (inst->getSubOp() == ALU_AND) {
                        dut_->fu_function_first_i = 7;
                    }
                }
                else if (inst->getFuType() == BRU) {
                    dut_->is_alu_first_i = 1;
                    if (inst->getSubOp() == BRU_JAR || inst->getSubOp() == BRU_JALR) {
                        dut_->jump_first_i = 1;
                        dut_->fu_select_b_first_i = 1;
                    }
                    else {
                        dut_->branch_first_i = 1;
                    }
                }
                else if (inst->getFuType() == LDU) {
                    dut_->load_first_i = 1;
                    dut_->ldu_op_first_i = inst->getSubOp();
                }
                else if (inst->getFuType() == STU) {
                    dut_->store_first_i = 1;
                    dut_->stu_op_first_i = inst->getSubOp();
                }
            }
            else if (index == 1) {
                dut_->deco_rob_req_valid_second_i = 1;
                dut_->uses_rs1_second_i = inst->getIsaRs1() != 0;
                dut_->uses_rs2_second_i = inst->getIsaRs2() != 0;
                dut_->uses_rd_second_i = inst->getIsaRd() != 0;
                dut_->uses_csr_second_i = inst->getFuType() == CSR;
                dut_->pc_second_i = inst->getPc();
                dut_->next_pc_second_i = inst->getPc() + 4;
                dut_->rs1_address_second_i = inst->getIsaRs1();
                dut_->rs2_address_second_i = inst->getIsaRs2();
                dut_->rd_address_second_i = inst->getIsaRd();
                dut_->imm_data_second_i = inst->getImmediate();
                if (inst->getFuType() == ALU) {
                    dut_->is_alu_second_i = 1;
                    if (inst->getSubOp() == ALU_ADD || inst->getSubOp() == ALU_SUB || inst->getSubOp() == ALU_ADDW || inst->getSubOp() == ALU_SUBW) {
                        dut_->fu_function_second_i = 0;
                        if (inst->getSubOp() == ALU_SUB || inst->getSubOp() == ALU_SUBW) {
                            dut_->alu_function_modifier_second_i = 1;
                        }
                    }
                    else if (inst->getSubOp() == ALU_SLL || inst->getSubOp() == ALU_SLLW) {
                        dut_->fu_function_second_i = 1;
                    }
                    else if (inst->getSubOp() == ALU_SLT) {
                        dut_->fu_function_second_i = 2;
                    }
                    else if (inst->getSubOp() == ALU_SLTU) {
                        dut_->fu_function_second_i = 3;
                    }
                    else if (inst->getSubOp() == ALU_XOR) {
                        dut_->fu_function_second_i = 4;
                    }
                    else if (inst->getSubOp() == ALU_SRL || inst->getSubOp() == ALU_SRA || inst->getSubOp() == ALU_SRLW || inst->getSubOp() == ALU_SRAW) {
                        dut_->fu_function_second_i = 5;
                        if (inst->getSubOp() == ALU_SRA || inst->getSubOp() == ALU_SRAW) {
                            dut_->alu_function_modifier_second_i = 1;
                        }
                    }
                    else if (inst->getSubOp() == ALU_OR) {
                        dut_->fu_function_second_i = 6;
                    }
                    else if (inst->getSubOp() == ALU_AND) {
                        dut_->fu_function_second_i = 7;
                    }
                }
                else if (inst->getFuType() == BRU) {
                    dut_->is_alu_second_i = 1;
                    if (inst->getSubOp() == BRU_JAR || inst->getSubOp() == BRU_JALR) {
                        dut_->jump_second_i = 1;
                        dut_->fu_select_b_second_i = 1;
                    }
                    else {
                        dut_->branch_second_i = 1;
                    }
                }
                else if (inst->getFuType() == LDU) {
                    dut_->load_second_i = 1;
                    dut_->ldu_op_second_i = inst->getSubOp();
                }
                else if (inst->getFuType() == STU) {
                    dut_->store_second_i = 1;
                    dut_->stu_op_second_i = inst->getSubOp();
                }
            }
            index ++;
        }
        dut_->eval();
        m_trace_->dump(sim_time_);
        sim_time_ ++;

        dut_->clk = 0;
        dut_->rst = 0;
        dut_->global_wfi_i = 0;
        dut_->global_trap_i = 0;
        dut_->global_ret_i = 0;
        dut_->deco_rob_req_valid_first_i = 0;
        dut_->deco_rob_req_valid_second_i = 0;
        dut_->uses_rs1_first_i = 0;
        dut_->uses_rs1_second_i = 0;
        dut_->uses_rs2_first_i = 0;
        dut_->uses_rs2_second_i = 0;
        dut_->uses_rd_first_i = 0;
        dut_->uses_rd_second_i = 0;
        dut_->uses_csr_first_i = 0;
        dut_->uses_csr_second_i = 0;
        dut_->pc_first_i = 0;
        dut_->pc_second_i = 0;
        dut_->next_pc_first_i = 0;
        dut_->next_pc_second_i = 0;
        dut_->rs1_address_first_i = 0;
        dut_->rs1_address_second_i = 0;
        dut_->rs2_address_first_i = 0;
        dut_->rs2_address_second_i = 0;
        dut_->rd_address_first_i = 0;
        dut_->rd_address_second_i = 0;
        dut_->csr_address_first_i = 0;
        dut_->csr_address_second_i = 0;
        dut_->mret_first_i = 0;
        dut_->mret_second_i = 0;
        dut_->sret_first_i = 0;
        dut_->sret_second_i = 0;
        dut_->wfi_first_i = 0;
        dut_->wfi_second_i = 0;
        dut_->half_first_i = 0;
        dut_->half_second_i = 0;
        dut_->is_fence_first_i = 0;
        dut_->is_fence_second_i = 0;
        dut_->fence_op_first_i = 0;
        dut_->fence_op_second_i = 0;
        dut_->is_aext_first_i = 0;
        dut_->is_aext_second_i = 0;
        dut_->is_mext_first_i = 0;
        dut_->is_mext_second_i = 0;
        dut_->csr_read_first_i = 0;
        dut_->csr_read_second_i = 0;
        dut_->csr_write_first_i = 0;
        dut_->csr_write_second_i = 0;
        dut_->imm_data_first_i = 0;
        dut_->imm_data_second_i = 0;
        dut_->fu_function_first_i = 0;
        dut_->fu_function_second_i = 0;
        dut_->alu_function_modifier_first_i = 0;
        dut_->alu_function_modifier_second_i = 0;
        dut_->fu_select_a_first_i = 0;
        dut_->fu_select_a_second_i = 0;
        dut_->fu_select_b_first_i = 0;
        dut_->fu_select_b_second_i = 0;
        dut_->jump_first_i = 0;
        dut_->jump_second_i = 0;
        dut_->branch_first_i = 0;
        dut_->branch_second_i = 0;
        dut_->is_alu_first_i = 0;
        dut_->is_alu_second_i = 0;
        dut_->load_first_i = 0;
        dut_->load_second_i = 0;
        dut_->store_first_i = 0;
        dut_->store_second_i = 0;
        dut_->ldu_op_first_i = 0;
        dut_->ldu_op_second_i = 0;
        dut_->stu_op_first_i = 0;
        dut_->stu_op_second_i = 0;
        dut_->aq_first_i = 0;
        dut_->aq_second_i = 0;
        dut_->rl_first_i = 0;
        dut_->rl_second_i = 0;
        dut_->eval();
        m_trace_->dump(sim_time_);
        sim_time_ ++;

    }

}