//
// Created by yzhang on 1/1/24.
//

#include "PhyRegfileWritePort.hpp"

namespace TimingModel {
    PhyRegfileWritePort::PhyRegfileWritePort(const std::string &name,
                                             TimingModel::PhysicalReg *phy_reg,
                                             sparta::log::MessageSource   & info_logger,
                                             sparta::DataInPort<PhyRegfileWriteBackPtr> *read_port_in) :
            name_(name),
            info_logger_(info_logger),
            phy_regfile_(phy_reg)
    {
        read_port_in->registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(PhyRegfileWritePort, WriteBackRegfile_, PhyRegfileWriteBackPtr));
    }

    const std::string& PhyRegfileWritePort::GetName() {
        return name_;
    }

    void PhyRegfileWritePort::WriteBackRegfile_(const TimingModel::PhyRegfileWriteBackPtr &phy_regfile_writeback_ptr) {
        PhyRegId_t phy_reg_idx = phy_regfile_writeback_ptr->phy_reg_idx;
        ILOG("Write Physical Register idx = " << phy_reg_idx << " , data = " << phy_regfile_writeback_ptr->data);
        if (phy_reg_idx != 0) {
            phy_regfile_->at(phy_reg_idx) = phy_regfile_writeback_ptr->data;
        }
    }
}