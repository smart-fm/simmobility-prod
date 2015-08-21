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

namespace
{
    class LogMsg : public Message
    {
    public:

        LogMsg(const std::string& logMsg, LoggerAgent::LogFile fileType) : logMsg(logMsg), fileType(fileType)
    	{
            priority = INTERNAL_MESSAGE_PRIORITY;
        }

        std::string logMsg;
        LoggerAgent::LogFile fileType;
    };
}

LoggerAgent::LoggerAgent() : Entity(-1)
{
    //bids
    std::ofstream* bidsFile = new std::ofstream("bids.csv");
    streams.insert(std::make_pair(BIDS, bidsFile));

    //expectations;
    std::ofstream* expectationsFile = new std::ofstream("expectations.csv");
    streams.insert(std::make_pair(EXPECTATIONS, expectationsFile));

    //eligible parcels
    std::ofstream* parcelsFile = new std::ofstream("parcels.csv");
    streams.insert(std::make_pair(PARCELS, parcelsFile));

    //units
    std::ofstream* unitsFile = new std::ofstream("units.csv");
    streams.insert(std::make_pair(UNITS, unitsFile));

    //projects
    std::ofstream* projectsFile = new std::ofstream("projects.csv");
    streams.insert(std::make_pair(PROJECTS, projectsFile));

    //projects
    std::ofstream* projectsDBFile = new std::ofstream("projectsDB.csv");
    streams.insert(std::make_pair(PROJECTS_DB, projectsDBFile));

    //hhpc postcodes
    std::ofstream* hhpcFile = new std::ofstream("HouseholdPostcodes.csv");
    streams.insert(std::make_pair(HH_PC, hhpcFile));

    //new units in the market
    std::ofstream* unitsInMarketFile = new std::ofstream("unitsInMarket.csv");
    streams.insert(std::make_pair(UNITS_IN_MARKET, unitsInMarketFile));

    //hh with taxi availability
    std::ofstream* taxiAvailabilityFile = new std::ofstream("hhWithtaxiAvailability.csv");
    streams.insert(std::make_pair(LOG_TAXI_AVAILABILITY, taxiAvailabilityFile));

    //vehicle ownership options of hh
    std::ofstream* vehicleOwnershipOptionFile = new std::ofstream("hhWithVehicleOwnership.csv");
    streams.insert(std::make_pair(LOG_VEHICLE_OWNERSIP, vehicleOwnershipOptionFile));

    //Tas level group logsum
    std::ofstream* tazLevelLogsumFile = new std::ofstream("tazLevelLogsum.csv");
    streams.insert(std::make_pair(LOG_TAZ_LEVEL_LOGSUM, tazLevelLogsumFile));

    //household group logsum
    std::ofstream* householdGroupLogsumFile = new std::ofstream("householdGroupLogsum.csv");
    streams.insert(std::make_pair(LOG_HOUSEHOLDGROUPLOGSUM, householdGroupLogsumFile));

    //individual hits logsum
    std::ofstream* individualHitsLogsumFile = new std::ofstream("IndividualHitsLogsum.csv");
    streams.insert(std::make_pair(LOG_INDIVIDUAL_HITS_LOGSUM, individualHitsLogsumFile));

    //household bid list
    std::ofstream* householdBidFile = new std::ofstream("householdBidList.csv");
    streams.insert(std::make_pair(LOG_HOUSEHOLDBIDLIST, householdBidFile));
}

LoggerAgent::~LoggerAgent()
{
    typename Files::iterator it;
    for (it = streams.begin(); it != streams.end(); it++)
    {
        if (it->second)
        {
            it->second->close();
            delete (it->second);
        }
    }
    streams.clear();
}

bool LoggerAgent::isNonspatial()
{
    return false;
}

void LoggerAgent::buildSubscriptionList(std::vector<BufferedBase*>& subsList) {}

void LoggerAgent::onWorkerEnter() {}

void LoggerAgent::onWorkerExit() {}

Entity::UpdateStatus LoggerAgent::update(timeslice now)
{
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}

void LoggerAgent::log(LogFile outputType, const std::string& logMsg)
{
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_LOG, MessageBus::MessagePtr(new LogMsg(logMsg, outputType)));
}

void LoggerAgent::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
    switch (type)
    {
        case LTMID_LOG:
        {
            const LogMsg& msg = MSG_CAST(LogMsg, message);

            if (msg.fileType == STDOUT)
            {
                PrintOut(msg.logMsg << std::endl);
            }
            else
            {
                (*streams[msg.fileType]) << msg.logMsg << std::endl;
            }
            break;
        }
        default:break;
    };
}
