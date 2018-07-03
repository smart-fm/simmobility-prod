//
//  DemandEstimator.h
//  AMODBase
//
//  Created by Harold Soh on 18/4/15.
//  Copyright (c) 2015 Harold Soh. All rights reserved.
//

#pragma once

#include <utility>
#include "Types.hpp"
#include "World.hpp"

namespace sim_mob {

namespace amod {

class DemandEstimator {
public:
    /**
     * Constructor
     */
    DemandEstimator() { }

    /**
     * Destructor
     */
    virtual ~DemandEstimator() {}

    /**
     * predict
     * predicts the demand (number of customer bookings) at a given position pos or location id
     * at a given time t
     * @param pos - Position
     * @param worldState - Pointer to amod world
     * @param t - time
     * @return a std::pair with the mean prediction and the uncertainty
     */
    virtual std::pair<double, double> predict(const amod::Position &pos, const amod::World &worldState, double t) = 0;

    /**
     * predict
     * predicts the demand (number of customer bookings) at a given position pos or location id
     * at a given time t
     * @param locId - location id
     * @param worldState - Pointer to amod world
     * @param t - time
     * @return a std::pair with the mean prediction and the uncertainty
     */
    virtual std::pair<double, double> predict(int locId, const amod::World &worldState, double t) = 0;
};
}

}

