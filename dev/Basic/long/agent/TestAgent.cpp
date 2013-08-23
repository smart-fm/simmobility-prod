/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_Agent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 13, 2013, 6:36 PM
 */

#include "TestAgent.hpp"
#include "conf/simpleconf.hpp"
#include "workers/Worker.hpp"
#include "message/MessageBus.hpp"


#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using std::vector;
using std::string;
using std::map;
using std::cout;

TestAgent::TestAgent(int id, messaging::MessageHandler* receiver)
: Agent(ConfigParams::GetInstance().mutexStategy, id), receiver(receiver) {
    isRegistered = false;
}

TestAgent::~TestAgent() {
}

void TestAgent::load(const map<string, string>& configProps) {
}

bool TestAgent::frame_init(timeslice now) {
    return true;
}

Entity::UpdateStatus TestAgent::frame_tick(timeslice now) {
    if (this->GetId() % 2 != 0 && receiver) {
        MessageBus::PostMessage(receiver, 11, MessageBus::MessagePtr(new Message()));
        //cout << "Message from:" << GetId() << " to: "<< receiver->GetId() << " Thread: "<< boost::lexical_cast<std::string>(boost::this_thread::get_id()) <<  std::endl;
    }
    return Entity::UpdateStatus::Continue;
}

void TestAgent::frame_output(timeslice now) {
}

bool TestAgent::isNonspatial() {
    return false;
}

void TestAgent::HandleMessage(Message::MessageType type, const Message& message) {
    //cout << "Hello Message, id:" << GetId() << " Type: "<< type << " Thread: "<< boost::lexical_cast<std::string>(boost::this_thread::get_id()) <<  std::endl;
}
