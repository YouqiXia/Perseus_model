#include "BusyTableUnit.hpp"


namespace TimingModel {

    const char* BusyTableUnit::name = "busy_table";

    BusyTableUnit::BusyTableUnit(sparta::TreeNode *node,
                                 const TimingModel::BusyTableUnit::BusyTableParameter *p) :
            sparta::Unit(node),
            scoreboard_(p->phy_reg_num)
    {
        busy_table_flush_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(
                BusyTableUnit, HandleFlush_, FlushingCriteria));
        check_busy_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(
                BusyTableUnit, CheckBusy_, InstGroupPtr));
        update_busy_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(
                BusyTableUnit, FuncUpdate_, InstGroupPtr));

        update_busy_in >> sparta::GlobalOrderingPoint(node, "busy_table_update");
        sparta::GlobalOrderingPoint(node, "busy_table_update") >> check_busy_in;
    }

    void BusyTableUnit::CheckBusy_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getRs1Type() != RegType_t::NONE) {
                inst_ptr->setIsRs1Forward(scoreboard_.GetBusyBit(inst_ptr->getPhyRs1()));
            }

            if (inst_ptr->getRs2Type() != RegType_t::NONE) {
                inst_ptr->setIsRs2Forward(scoreboard_.GetBusyBit(inst_ptr->getPhyRs2()));
            }

            if (inst_ptr->getRdType() != RegType_t::NONE) {
                scoreboard_.SetBusyBit(inst_ptr->getPhyRd());
            }
        }
    }

    void BusyTableUnit::FuncUpdate_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getRdType() != RegType_t::NONE) {
                scoreboard_.ClearBusyBit(inst_ptr->getPhyRd());
            }
        }
    }

    void BusyTableUnit::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        scoreboard_.Flush();
    }
}