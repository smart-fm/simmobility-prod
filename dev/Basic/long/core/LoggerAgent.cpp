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
#include "util/HelperFunctions.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;

namespace {

    class LogMsg : public Message {
    public:

        LogMsg(const std::string& logMsg, LoggerAgent::LogFile fileType)
        : logMsg(logMsg), fileType(fileType) {
            priority = INTERNAL_MESSAGE_PRIORITY;
        }
        std::string logMsg;
        LoggerAgent::LogFile fileType;
    };
}

LoggerAgent::LoggerAgent() : Entity(-1) {

    //bids
    std::ofstream* bidsFile = new std::ofstream("bids.csv");
    streams.insert(std::make_pair(BIDS, bidsFile));

    //expectations;
    std::ofstream* expectationsFile = new std::ofstream("expectations.csv");
    streams.insert(std::make_pair(EXPECTATIONS, expectationsFile));
}

LoggerAgent::~LoggerAgent() {
    typename Files::iterator it;
    for (it = streams.begin(); it != streams.end(); it++) {
        if (it->second) {
            it->second->close();
            delete (it->second);
        }
    }
    streams.clear();
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

void LoggerAgent::log(LogFile outputType, const std::string& logMsg) {
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_LOG,
            MessageBus::MessagePtr(new LogMsg(logMsg, outputType)));
}

void LoggerAgent::HandleMessage(messaging::Message::MessageType type,
        const messaging::Message& message) {
    switch (type) {
        case LTMID_LOG:
        {
            const LogMsg& msg = MSG_CAST(LogMsg, message);
            if (msg.fileType == STDOUT) {
                PrintOut(msg.logMsg << std::endl);
            } else {
                (*streams[msg.fileType]) << msg.logMsg << std::endl;
            }
            break;
        }
        default:break;
    };
}