#include "DebugFlags.hh"
namespace Emulator
{

std::set<std::string> debugFlagsEnableSet;

std::set<std::string> ObjectEnableSet;

extern void setFlagEnable(std::string flag) {
    debugFlagsEnableSet.insert(flag);
};

extern bool getFlagEnable(std::string flag) {
    return debugFlagsEnableSet.count(flag);
};

extern void setObjectEnable(std::string Name){
    ObjectEnableSet.insert(Name);
};

extern bool getObjectEnable(std::string Name){
    return ObjectEnableSet.count(Name);
};

} // namespace Emulator




