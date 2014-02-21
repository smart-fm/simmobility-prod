/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LoggerAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 21, 2014, 1:32 PM
 */

#include "LoggerAgent.hpp"
#include "message/MessageBus.hpp"
#include "Common.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;

namespace {

    class LogMsg : public Message {
    public:

        LogMsg(const std::string& logMsg)
        : logMsg(logMsg) {
            priority = INTERNAL_MESSAGE_PRIORITY;
        }
        std::string logMsg;
    };
}

LoggerAgent::LoggerAgent() : Entity(-1) {
}

LoggerAgent::~LoggerAgent() {
}

bool LoggerAgent::isNonspatial() {
    return false;
}

void LoggerAgent::buildSubscriptionList(std::vector<BufferedBase*>& subsList) {
}

void LoggerAgent::onWorkerEnter() {
}

void LoggerAgent::onWorkerExit() {
}

Entity::UpdateStatus LoggerAgent::update(timeslice now) {
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}

void LoggerAgent::log(const std::string& logMsg){
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_LOG,
            MessageBus::MessagePtr(new LogMsg(logMsg)));
}

void LoggerAgent::HandleMessage(messaging::Message::MessageType type,
                    const messaging::Message& message){
    switch (type) {
        case LTMID_LOG:
        {
            const LogMsg& msg = MSG_CAST(LogMsg, message);
            PrintOut(msg.logMsg << std::endl);
            break;
        }
        default:break;
    };
}
