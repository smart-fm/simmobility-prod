//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   SystemEvents.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on August 29, 2013, 11:30 AM
 */

#pragma once
#include "args/EventArgs.hpp"
#include "EventListener.hpp"

namespace sim_mob {

    namespace event {
        const unsigned int EVT_CORE_START    = 100000;
        const unsigned int EVT_SHORT_START   = 200000;
        const unsigned int EVT_MEDIUM_START  = 300000;
        const unsigned int EVT_LONG_START    = 400000;
        
        enum CoreEvent {
            EVT_CORE_SYTEM_START = EVT_CORE_START,
            //agent life cycle.
            EVT_CORE_AGENT_DIED,
        };

        /**
         * Arguments for agent life-cycle events.
         */
        DECLARE_CUSTOM_CALLBACK_TYPE (AgentLifeCycleEventArgs)
        class AgentLifeCycleEventArgs : public EventArgs {
        public:
            AgentLifeCycleEventArgs(unsigned int agentId);
            virtual ~AgentLifeCycleEventArgs();
            virtual AgentLifeCycleEventArgs& operator=(const AgentLifeCycleEventArgs& source);
            unsigned int GetAgentId() const;
        private:
            unsigned int agentId;
        };
    }
}
