//
// Created by yzhang on 1/1/24.
//

#include "sparta/utils/SpartaAssert.hpp"

#include "PhyRegfile.hpp"

namespace TimingModel {

    const char* PhyRegfile::name = "physical_regfile";

    PhyRegfile::PhyRegfile(sparta::TreeNode* node, const PhyRegfileParameter* p) :
        sparta::Unit(node),
        phy_regfile_(p->phy_reg_num),
        phy_regfile_num_(p->phy_reg_num)
    {
        uint8_t read_port_num = p->read_port_num;
        while(read_port_num--) {
            auto * phy_regfile_request_in = new sparta::DataInPort<PhyRegfileRequest>
                    (&unit_port_set_, "phy_regfile_request_in"+std::to_string(read_port_num));
            auto * phy_regfile_request_out = new sparta::DataOutPort<PhyRegfileRequest>
                    (&unit_port_set_, "phy_regfile_request_in"+std::to_string(read_port_num));
        }
    }

    void PhyRegfile::ReadRegfile_(const PhyRegfileRequest &phy_regfile_request) {
        PhyRegfileRequest phy_regfile_request_tmp;
        PhyRegId_t phy_reg_idx = phy_regfile_request.phy_reg_idx;
        sparta_assert(phy_reg_idx <= phy_regfile_num_, "physical regfile accessing is out of range");

        phy_regfile_request_tmp.data =  phy_regfile_[phy_reg_idx];
        phy_regfile_request_tmp.rob_idx = phy_regfile_request.rob_idx;

        read_phy_regfile_out.send(phy_regfile_request_tmp);
    }

    void PhyRegfile::WriteBackRegfile_(const TimingModel::PhyRegfileWriteBack &phy_regfile_writeback) {
        PhyRegId_t phy_reg_idx = phy_regfile_writeback.phy_reg_idx;
        sparta_assert(phy_reg_idx <= phy_regfile_num_, "physical regfile accessing is out of range");
        phy_regfile_[phy_reg_idx] = phy_regfile_writeback.data;
    }
}