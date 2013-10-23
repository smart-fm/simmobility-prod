//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LT_Agent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 13, 2013, 6:36 PM
 */
#pragma once
#include "Common.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/Agent.hpp"
#include "event/EventManager.hpp"
#include "message/MessageHandler.hpp"

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
        class LT_Agent : public sim_mob::Agent{
        public:
            LT_Agent(int id);
            virtual ~LT_Agent();

            /**
             * Inherited from Agent.
             */
            virtual void load(const std::map<std::string, std::string>& configProps);

        protected:
        
            /**
             * Method called when agent runs the method Update for the first time.
             * @param now time.
             */
            virtual void onStart(timeslice now) = 0;
            
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
             * @return update status.
             */
            virtual sim_mob::Entity::UpdateStatus OnFrameTick(timeslice now) = 0;

            /**
             * Handler for frame_output method from agent.
             * @param now time.
             */
            virtual void OnFrameOutput(timeslice now) = 0;

            /**
             * Inherited from MessageReceiver.
             */
            virtual void HandleMessage(messaging::Message::MessageType type,
                const messaging::Message& message);

            /**
             * Inherited from Agent.
             */
            bool frame_init(timeslice now);
            sim_mob::Entity::UpdateStatus frame_tick(timeslice now);
            void frame_output(timeslice now);
            bool isNonspatial();    
            bool isRegistered;
        };
    }
}

