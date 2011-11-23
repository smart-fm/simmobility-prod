/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "Base.hpp"

namespace sim_mob
{

namespace aimsun   // This namespace should be called "database" to be more generic.
{
    struct Signal : public Base
    {
        int id;
        int nodeId;
        double xPos;
        double yPos;
        std::string typeCode;
    };

}

}
