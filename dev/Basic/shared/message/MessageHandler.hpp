//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   MessageHandler.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Aug 15, 2013, 9:30 PM
 */
#pragma once
#include "Message.hpp"
#include <set>

namespace sim_mob {

    namespace messaging {

        /**
         * Interface for all classes which are capable to receive messages from 
         * MessageBus. 
         * @see MessageBus to get more details.
         */
        class MessageHandler {
        	/// The already used numbers
        	static std::set<unsigned int> used;
        	/// The first available free number
        	static unsigned int freeCounter ;

        	void add(unsigned int i);

        	unsigned int getNewId();

        public:
            MessageHandler(unsigned int id);
            MessageHandler();

            virtual ~MessageHandler();
            /**
             * Handles all messages sent to the MessageHandler implementation.
             * @param type of the message.
             * @param message data received.
             */
            virtual void HandleMessage(Message::MessageType type, const Message& message) = 0;

            /**
             * Gets the id associated with this handler.
             * NOTE: used only for debug but it will be necessary on the future.
             * @return id value associated.
             */
            unsigned int GetId() const;

            void* GetContext() const{
            	return context;
            }

        private:
            friend class MessageBus;
            unsigned int id;
            //MessageBus exclusive
            void* context;
        };
    }
}
