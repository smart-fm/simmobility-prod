/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "Base.hpp"
//#include "Phase.hpp"

namespace sim_mob
{

namespace aimsun   // This namespace should be called "database" to be more generic.
{
    /// \author LIM Fung Chai
    struct Signal : public Base
    {
        int id;
        int nodeId;
        double xPos;
        double yPos;
        double bearing;
        std::string typeCode;
    };

}

}
