#include "FuncUnits.hpp"
#include "simulation/ExtensionsCfg.hpp"

namespace TimingModel {
    DispatchMap dispatch_map;

    FuMap fu_map;

    /* info from yaml configuration */

    DispatchMap& getDispatchMap() {
        return dispatch_map;
    }

    FuMap& getFuMap() {
        return fu_map;
    }

    void setDispatchMap(sparta::TreeNode * rootnode){
        auto topo_dispatch_map = TimingModel::getCommonTopology(rootnode, "dispatch_map");
        uint32_t topo_cnt = topo_dispatch_map.size();
        if(topo_cnt > 0){
            dispatch_map.clear();
        }else{
            return;
        }

        for(auto i=0u; i<topo_cnt; i++){
            uint32_t type_cnt = topo_dispatch_map[i].size()-1;
            std::set<FuncType> fu_types;
            for(auto t=0u; t<type_cnt; t++){
                FuncType type = stringToFuncType(topo_dispatch_map[i][t+1]);
                fu_types.insert(type);
            }
            dispatch_map[topo_dispatch_map[i][0]] = fu_types;
        }
    }

    void setFuMap(sparta::TreeNode * rootnode){
        auto topo_fu_map = TimingModel::getCommonTopology(rootnode, "fu_group");
        uint32_t topo_cnt = topo_fu_map.size();
        if(topo_cnt > 0){
            fu_map.clear();
        }else{
            return;
        }

        for(auto i=0u; i<topo_cnt; i++){
            fu_map.emplace_back(topo_fu_map[i][0]);
        }
    }

    void printDispatchMap(){
        std::cout << "fu map:" << std::endl;
        for(std::map<RsType, std::set<FuncType>>::iterator it=dispatch_map.begin(); it!=dispatch_map.end(); ++it){
            std::cout << it->first << ": ";
            for (std::set<FuncType>::iterator its =it->second.begin(); its!=it->second.end(); ++its)
                std::cout << funcTypeToString(*its) << ", ";
            std::cout << std::endl;
        }
    }

    void setFuCfgFromExtensions(sparta::TreeNode * rootnode){
        setDispatchMap(rootnode);
        setFuMap(rootnode);
    }
}

