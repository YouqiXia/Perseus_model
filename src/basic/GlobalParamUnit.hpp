//
// Created by yzhang on 9/12/24.
//

#ifndef PERSEUS_GLOBALPARAMUNIT_HPP
#define PERSEUS_GLOBALPARAMUNIT_HPP

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/simulation/TreeNode.hpp"

#include "Inst.hpp"

namespace TimingModel {

    inline FuncType stringToFuncType(std::string & str){
        FuncType type = FuncType::NO_TYPE;
        if(str == "ALU")       return FuncType::ALU;
        else if(str == "MUL")  return FuncType::MUL;
        else if(str == "DIV")  return FuncType::DIV;
        else if(str == "BRU")  return FuncType::BRU;
        else if(str == "CSR")  return FuncType::CSR;
        else if(str == "LDU")  return FuncType::LDU;
        else if(str == "STU")  return FuncType::STU;
        else if(str == "FPU")  return FuncType::FPU;
        else                                        return FuncType::NO_TYPE;
    }

    inline std::string funcTypeToString(FuncType type){
        switch(type){
            case FuncType::ALU:     return "ALU";
            case FuncType::MUL:     return "MUL";
            case FuncType::DIV:     return "DIV";
            case FuncType::BRU:     return "BRU";
            case FuncType::CSR:     return "CSR";
            case FuncType::LDU:     return "LDU";
            case FuncType::STU:     return "STU";
            case FuncType::FPU:     return "FPU";
            default:
                break;
        }
        return nullptr;
    }

    class GlobalParamUnit : public sparta::Unit {
    public:
        class GlobalParameter : public sparta::ParameterSet {
        public:
            GlobalParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
        {}

            PARAMETER(std::vector<std::string>, dispatch_map,
                  std::vector<std::string>({"following_unit_name", "|", "bandwidth", "|", "fu_type_n", "|"}), "the dispatch map to multiple following units")
            PARAMETER(std::vector<std::string>, write_back_map,
                  std::vector<std::string>({"following_unit_name", "|", "bandwidth", "|"}), "the write map from multiple following units")

    };
        typedef std::map<std::string, std::set<FuncType>> DispatchMap;
        typedef std::map<std::string, uint32_t> DispatchIssueWidthMap;
        typedef std::map<std::string, uint32_t> WriteBackMap;

        static const char* name;

        GlobalParamUnit(sparta::TreeNode* node, const GlobalParameter* p);

        ~GlobalParamUnit() {}

        DispatchMap& getDispatchMap() { return dispatch_following_map_; };

        DispatchIssueWidthMap& getDispatchIssueWidthMap() { return dispatch_issue_width_map_; };

        WriteBackMap& getWriteBackMap() { return write_back_map_; };

    private:
        void ParseDispatchMap_(const TimingModel::GlobalParamUnit::GlobalParameter *p);

        void ParseWriteBackMap_(const TimingModel::GlobalParamUnit::GlobalParameter *p);

    private:
        DispatchMap dispatch_following_map_;
        DispatchIssueWidthMap dispatch_issue_width_map_;
        WriteBackMap write_back_map_;
    };

    GlobalParamUnit* getGlobalParams(sparta::TreeNode *);

}


#endif //PERSEUS_GLOBALPARAMUNIT_HPP
