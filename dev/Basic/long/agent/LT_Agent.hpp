/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_Agent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 13, 2013, 6:36 PM
 */
#pragma once
#include "Common.hpp"
#include "entities/UpdateParams.hpp"
#include "conf/simpleconf.hpp"
#include "entities/Agent.hpp"
#include "event/EventManager.hpp"
#include "message/MessageReceiver.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents an Long-Term agent.
         * These agents can have different behaviors:
         * - Seller
         * - Bidder 
         * - Both
         * It will depend of the context.
         */
        class LT_Agent : public sim_mob::Agent, public messaging::MessageReceiver {
        public:
            LT_Agent(int id);
            virtual ~LT_Agent();

            /**
             * Inherited from Agent.
             */
            virtual void load(const std::map<std::string, std::string>& configProps);

            /**
             * Gets the EventManager reference from worker parent.
             * @return EventManager reference. 
             */
            sim_mob::event::EventManager& GetEventManager();

        protected:

            /**
             * Handler for frame_init method from agent.
             * @param now time.
             * @return true if the init ran well or false otherwise.
             */
            virtual bool OnFrameInit(timeslice now) = 0;

            /**
             * Handler for frame_tick method from agent.
             * Attention: this method can be called N 
             * times depending on the messages contained in the message queue.
             * 
             * messageCounter field has the counter of 
             * the read messages on the current tick. 
             * 0- indicates that is the first time that 
             *    this method is called on this tick.
             * N- indicates that (N+1) message were loaded in this tick.  
             * 
             * @param now time.
             * @param messageCounter has the counter.
             * @return update status.
             */
            virtual sim_mob::Entity::UpdateStatus OnFrameTick(timeslice now,
                    int messageCounter) = 0;

            /**
             * Handler for frame_output method from agent.
             * @param now time.
             */
            virtual void OnFrameOutput(timeslice now) = 0;

            /**
             * Inherited from MessageReceiver.
             */
            virtual void HandleMessage(messaging::MessageReceiver::MessageType type,
                    messaging::MessageReceiver& sender, const messaging::Message& message);

            /**
             * Inherited from Agent.
             */
            bool frame_init(timeslice now);
            sim_mob::Entity::UpdateStatus frame_tick(timeslice now);
            void frame_output(timeslice now);
            bool isNonspatial();    
        };
    }
}

