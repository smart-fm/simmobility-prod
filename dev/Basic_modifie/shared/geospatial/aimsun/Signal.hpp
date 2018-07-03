//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
