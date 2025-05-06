/**
 * @file DoAllRegister.cpp
 * @author your name (you@domain.com)
 * @brief The self-defined sparta::unit header file should never be included here.
 * @version 0.1
 * @date 2025-04-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "UnitRegister.hpp"

#include "example.hpp"
#include "FE.hpp"
#include "BE.hpp"

namespace TimingModel {

int UnitRegister::doAllRegister() {
    int retval = 0;
    auto &the_instance = UnitRegister::instance();

    retval = registerExampleUnit(the_instance);
    if (retval != 0) {
        return retval;
    }

    retval = registerPerfectFE(the_instance);
    if (retval != 0) {
        return retval;
    }

    retval = registerBE(the_instance);
    if (retval != 0) {
        return retval;
    }
    
    return retval;
}

}