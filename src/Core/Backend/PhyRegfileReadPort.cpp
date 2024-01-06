//
// Created by yzhang on 1/1/24.
//

#include "PhyRegfileReadPort.hpp"

namespace TimingModel {

    PhyRegfileReadPort::PhyRegfileReadPort(const std::string& name,
                                           PhysicalReg *phy_reg,
                                           sparta::log::MessageSource   & info_logger,
                                           sparta::DataInPort<PhyRegfileRequestPtr> *read_port_in,
                                           sparta::DataOutPort<PhyRegfileRequestPtr> *read_port_out) :
            name_(name),
            info_logger_(info_logger),
            read_phy_regfile_out_(read_port_out),
            phy_regfile_(phy_reg)
    {
        read_port_in->registerConsumerHandler
        (CREATE_SPARTA_HANDLER_WITH_DATA(PhyRegfileReadPort, ReadRegfile_, PhyRegfileRequestPtr));
    }

    const std::string& PhyRegfileReadPort::GetName() const {
        return name_;
    }

    void PhyRegfileReadPort::ReadRegfile_(const TimingModel::PhyRegfileRequestPtr &phy_regfile_request_ptr) {
        PhyRegfileRequestPtr phy_regfile_request_ptr_tmp;
        PhyRegId_t phy_reg_idx = phy_regfile_request_ptr->phy_reg_idx;

        phy_regfile_request_ptr_tmp->data = 0;
        if (phy_reg_idx != 0) {
            phy_regfile_request_ptr_tmp->data = phy_regfile_->at(phy_reg_idx);
        }
        phy_regfile_request_ptr_tmp->func_unit = phy_regfile_request_ptr->func_unit;
        phy_regfile_request_ptr_tmp->is_rs1 = phy_regfile_request_ptr->is_rs1;
        phy_regfile_request_ptr_tmp->is_rs2 = phy_regfile_request_ptr->is_rs2;

        ILOG("read from physical register, reg_idx = " << phy_reg_idx
            << " , data = " << phy_regfile_request_ptr_tmp->data);

        read_phy_regfile_out_->send(phy_regfile_request_ptr_tmp);
    }




}