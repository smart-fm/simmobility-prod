//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   EventArgs.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 7, 2013, 3:26 PM
 */

#pragma once

namespace sim_mob {
    namespace event {

        typedef unsigned int EventId;

        ///Represents an event data.

        class EventArgs {
        public:
            EventArgs();
            EventArgs(const EventArgs& source);
            virtual ~EventArgs();

            virtual EventArgs& operator=(const EventArgs& source);
        };

    }
}
/**
 * Call this before you EventArgs implementation.
 */
#define DECLARE_CUSTOM_CALLBACK_TYPE(type) class type; \
        typedef void (sim_mob::event::EventListener::*type##ContextCallback)\
        (sim_mob::event::EventId id, sim_mob::event::Context ctxId, \
                sim_mob::event::EventPublisher* sender, const type& args);


