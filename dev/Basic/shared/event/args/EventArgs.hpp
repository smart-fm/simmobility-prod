/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventArgs.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 7, 2013, 3:26 PM
 */

#pragma once

namespace sim_mob {

    /**
     * Represents an event data.
     */
    class EventArgs {
    public:
        EventArgs();
        EventArgs(const EventArgs& source);
        virtual ~EventArgs();
        virtual EventArgs& operator=(const EventArgs& source);
    };
}

