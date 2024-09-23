//
// Created by yzhang on 9/12/24.
//

#include "GlobalParamUnit.hpp"

namespace TimingModel {

    const char* GlobalParamUnit::name = "global_param";

    GlobalParamUnit::GlobalParamUnit(sparta::TreeNode *node, const TimingModel::GlobalParamUnit::GlobalParameter *p) :
            sparta::Unit(node) {
        ParseDispatchMap_(p);
        ParseWriteBackMap_(p);
    }

    void GlobalParamUnit::ParseDispatchMap_(const TimingModel::GlobalParamUnit::GlobalParameter *p) {
        enum class State {NAME, WIDTH, FU_TYPE};
        State state = State::NAME;
        std::set<FuncType> func_types;
        std::string following_unit_name;
        uint64_t width = 0;
        for (std::string dispatch_map_info: p->dispatch_map) {
            if (dispatch_map_info == "|") {
                if (state == State::NAME) {
                    state = State::WIDTH;
                } else if (state == State::WIDTH) {
                    state = State::FU_TYPE;
                } else {
                    dispatch_following_map_[following_unit_name] = func_types;
                    dispatch_issue_width_map_[following_unit_name] = width;
                    func_types.clear();
                    state = State::NAME;
                }
            } else {
                if (state == State::NAME) {
                    following_unit_name = dispatch_map_info;
                } else if (state == State::WIDTH) {
                   width = std::atoi(dispatch_map_info.c_str());
                } else {
                    func_types.emplace(stringToFuncType(dispatch_map_info));
                }
            }
        }
    }

    void GlobalParamUnit::ParseWriteBackMap_(const TimingModel::GlobalParamUnit::GlobalParameter *p) {
        enum class State {NAME, WIDTH};
        State state = State::NAME;
        std::string following_unit_name;
        uint32_t width = 0;
        for (std::string dispatch_map_info: p->write_back_map) {
            if (dispatch_map_info == "|") {
                if (state == State::NAME) {
                    state = State::WIDTH;
                } else {
                    write_back_map_[following_unit_name] = width;
                    state = State::NAME;
                }
            } else {
                if (state == State::NAME) {
                    following_unit_name = dispatch_map_info;
                } else {
                   width = std::atoi(dispatch_map_info.c_str());
                //    width = std::stoi(dispatch_map_info);
                }
            }
        }
    }

    GlobalParamUnit* getGlobalParams(sparta::TreeNode *node) {
        GlobalParamUnit * global_param = nullptr;
        if (node) {
            if (node->hasChild(GlobalParamUnit::name)) {
                global_param = node->getChild(GlobalParamUnit::name)->getResourceAs<GlobalParamUnit>();
            } else {
                return getGlobalParams(node->getParent());
            }
        }
        sparta_assert(global_param != nullptr, "Global param was not found");

        return global_param;
    }

}

