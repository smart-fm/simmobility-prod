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
#include "event/SystemEvents.hpp"


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
: Agent(ConfigParams::GetInstance().mutexStategy(), id), receiver(receiver) {
    isRegistered = false;
}

TestAgent::~TestAgent() {
}

void TestAgent::load(const map<string, string>& configProps) {
}

bool TestAgent::frame_init(timeslice now) {
    if (!isRegistered){
        MessageBus::SubscribeEvent(event::EVT_CORE_SYTEM_START, this);
        MessageBus::SubscribeEvent(9999999, this);
        MessageBus::SubscribeEvent(9999998, receiver, this);
        MessageBus::SubscribeEvent(event::EVT_CORE_AGENT_DIED, receiver, this);
        isRegistered = true;
    }
    return true;
}

Entity::UpdateStatus TestAgent::frame_tick(timeslice now) {
    if (this->GetId() % 2 != 0 && receiver) {
        MessageBus::PostMessage(receiver, 11, MessageBus::MessagePtr(new Message()));
        //cout << "Message from:" << GetId() << " to: "<< receiver->GetId() << " Thread: "<< boost::lexical_cast<std::string>(boost::this_thread::get_id()) <<  std::endl;
        //MessageBus::PublishEvent(9999998, this);
    }
    /*if (this->GetId() == 0 && now.ms() > 100){
        return Entity::UpdateStatus::Done;
    }*/
    return Entity::UpdateStatus::Continue;
}

void TestAgent::frame_output(timeslice now) {
}

bool TestAgent::isNonspatial() {
    return false;
}

void TestAgent::HandleMessage(Message::MessageType type, const Message& message) {
    MessageBus::PublishEvent(9999998, this, MessageBus::EventArgsPtr(new event::EventArgs()));
    //cout << "Hello Message, id:" << GetId() << " Type: "<< type << " Thread: "<< boost::lexical_cast<std::string>(boost::this_thread::get_id()) <<  std::endl;
}

void TestAgent::OnEvent(sim_mob::event::EventId id, 
                                 sim_mob::event::EventPublisher* sender, 
                                 const sim_mob::event::EventArgs& args){
      switch(id){
        case event::EVT_CORE_SYTEM_START:{
            //cout << "System Start event" <<  std::endl;
            break;
        }
        default:break;
    }

}

void TestAgent::OnEvent(sim_mob::event::EventId id,
        sim_mob::event::Context ctxId,
        sim_mob::event::EventPublisher* sender,
        const sim_mob::event::EventArgs& args) {
    switch(id){
        case event::EVT_CORE_AGENT_DIED:{
           const AgentLifeCycleEventArgs& lagrs = MSG_CAST(AgentLifeCycleEventArgs, args);
           cout << "Agent died:"<< lagrs.GetAgentId() <<  std::endl;
           receiver = NULL;
           break;
        }
        default:break;
    }
    //cout << "ContextEvent Id:"<< (int)id <<  std::endl;
}
