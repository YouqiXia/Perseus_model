//
// Created by yzhang on 1/1/24.
//

#include "DispatchStage.hpp"

namespace TimingModel {

    DispatchStage::DispatchStage(sparta::TreeNode *node, const DispatchStageParameter *p) :
            sparta::Unit(node),
            phy_regfile_num_(p->phy_reg_num),
            phy_regfile_(p->phy_reg_num)
    {
        // construct the inport and outport of the Read Port
        uint8_t reg_read_port_num = p->reg_read_port_num;
        while(reg_read_port_num--) {
            auto * phy_reg_read_port_in = new sparta::DataInPort<PhyRegfileRequest>
                    (&unit_port_set_, "phy_reg_read_port_in_"+std::to_string(reg_read_port_num));
            phy_reg_read_ports_[]
        }
        // construct the inport of write port
    }

}