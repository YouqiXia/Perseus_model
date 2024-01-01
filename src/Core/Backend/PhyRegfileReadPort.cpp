//
// Created by yzhang on 1/1/24.
//

#include "PhyRegfileReadPort.hpp"

namespace TimingModel {

    PhyRegfileReadPort::PhyRegfileReadPort(const std::string& name,
                                           PhysicalReg *phy_reg,
                                           sparta::log::MessageSource   & info_logger,
                                           sparta::DataInPort<PhyRegfileRequest> *read_port_in,
                                           sparta::DataOutPort<PhyRegfileRequest> *read_port_out) :
            name_(name),
            info_logger_(info_logger),
            read_phy_regfile_out_(read_port_out),
            phy_regfile_(phy_reg)
    {
        read_port_in->registerConsumerHandler
        (CREATE_SPARTA_HANDLER_WITH_DATA(PhyRegfileReadPort, ReadRegfile_, PhyRegfileRequest));
    }

    const std::string& PhyRegfileReadPort::GetName() const {
        return name_;
    }

    void PhyRegfileReadPort::ReadRegfile_(const TimingModel::PhyRegfileRequest &phy_regfile_request) {
        PhyRegfileRequest phy_regfile_request_tmp;
        PhyRegId_t phy_reg_idx = phy_regfile_request.phy_reg_idx;

        phy_regfile_request_tmp.data = phy_regfile_->at(phy_reg_idx);
        phy_regfile_request_tmp.rob_idx = phy_regfile_request.rob_idx;

        ILOG("read from physical register, reg_idx = " << phy_reg_idx
            << " , data = " << phy_regfile_request_tmp.data);

        read_phy_regfile_out_->send(phy_regfile_request_tmp);
    }




}