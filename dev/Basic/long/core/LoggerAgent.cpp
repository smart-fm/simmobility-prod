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
#include "conf/ConfigParams.hpp"
#include "conf/ConfigManager.hpp"


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

	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	bool bids = config.ltParams.outputFiles.bids;
    bool expectations = config.ltParams.outputFiles.expectations;
    bool parcels = config.ltParams.outputFiles.parcels;
    bool units = config.ltParams.outputFiles.units;
    bool projects = config.ltParams.outputFiles.projects;
    bool hh_pc = config.ltParams.outputFiles.hh_pc;
    bool units_in_market = config.ltParams.outputFiles.units_in_market;
    bool log_taxi_availability = config.ltParams.outputFiles.log_taxi_availability;
    bool log_vehicle_ownership = config.ltParams.outputFiles.log_vehicle_ownership;
    bool log_taz_level_logsum = config.ltParams.outputFiles.log_taz_level_logsum;
    bool log_householdgrouplogsum = config.ltParams.outputFiles.log_householdgrouplogsum;
    bool log_individual_hits_logsum = config.ltParams.outputFiles.log_individual_hits_logsum;
    bool log_householdbidlist = config.ltParams.outputFiles.log_householdbidlist;
    bool log_individual_logsum_vo = config.ltParams.outputFiles.log_individual_logsum_vo;
    bool log_screeningprobabilities = config.ltParams.outputFiles.log_screeningprobabilities;
    bool log_hhchoiceset = config.ltParams.outputFiles.log_hhchoiceset;
    bool log_error = config.ltParams.outputFiles.log_error;
    bool log_school_assignment = config.ltParams.outputFiles.log_school_assignment;
    bool log_pre_school_assignment = config.ltParams.outputFiles.log_pre_school_assignment;
    bool log_hh_awakening = config.ltParams.outputFiles.log_hh_awakening;
    bool log_hh_exit = config.ltParams.outputFiles.log_hh_exit;
    bool log_random_nums = config.ltParams.outputFiles.log_random_nums;
    bool log_dev_roi = config.ltParams.outputFiles.log_dev_roi;
    bool log_household_statistics = config.ltParams.outputFiles.log_household_statistics;


    PrintOutV(">>>>>>>>>>>>>>>" << std::endl);
    PrintOutV("Output CSV generation. " << std::endl);
	PrintOutV("Output CSV generation. bids: " << bids << std::endl);
	PrintOutV("Output CSV generation. expectations: " << expectations << std::endl);
	PrintOutV("Output CSV generation. parcels: " << parcels << std::endl);
	PrintOutV("Output CSV generation. units: " << units << std::endl);
	PrintOutV("Output CSV generation. projects " << projects << std::endl);
	PrintOutV("Output CSV generation. hh_pc: " << hh_pc << std::endl);
	PrintOutV("Output CSV generation. units_in_market: " << units_in_market << std::endl);
	PrintOutV("Output CSV generation. log_taxi_availability: " << log_taxi_availability << std::endl);
	PrintOutV("Output CSV generation. log_vehicle_ownersip: " << log_vehicle_ownership << std::endl);
	PrintOutV("Output CSV generation. log_taz_level_logsum: " << log_taz_level_logsum << std::endl);
	PrintOutV("Output CSV generation. log_householdgrouplogsum: " << log_householdgrouplogsum << std::endl);
	PrintOutV("Output CSV generation. log_individual_hits_logsum: " << log_individual_hits_logsum << std::endl);
	PrintOutV("Output CSV generation. log_householdbidlist: " << log_householdbidlist << std::endl);
	PrintOutV("Output CSV generation. log_individual_logsum_vo: " << log_individual_logsum_vo << std::endl);
	PrintOutV("Output CSV generation. log_screeningprobabilities: " << log_screeningprobabilities << std::endl);
	PrintOutV("Output CSV generation. log_hhchoiceset: " << log_hhchoiceset << std::endl);
	PrintOutV("Output CSV generation. log_error: " << log_error << std::endl);
	PrintOutV("Output CSV generation. log_school_assignment: " << log_school_assignment << std::endl);
	PrintOutV("Output CSV generation. log_pre_school_assignment: " << log_pre_school_assignment << std::endl);
	PrintOutV("Output CSV generation. log_hh_awakening: " << log_hh_awakening << std::endl);
	PrintOutV("Output CSV generation. log_hh_exit: " << log_hh_exit << std::endl);
	PrintOutV("Output CSV generation. log_random_nums: " << log_random_nums << std::endl);
	PrintOutV("Output CSV generation. log_dev_roi: " << log_dev_roi << std::endl);
	PrintOutV("Output CSV generation. log_household_statistics: " << log_household_statistics << std::endl);
	PrintOutV(">>>>>>>>>>>>>>>" << std::endl);



    if(bids)
    {
		//bids
		std::ofstream* bidsFile = new std::ofstream("bids.csv");
		streams.insert(std::make_pair(BIDS, bidsFile));
    }

    if(expectations)
    {
		//expectations;
		std::ofstream* expectationsFile = new std::ofstream("expectations.csv");
		streams.insert(std::make_pair(EXPECTATIONS, expectationsFile));
    }

    if(parcels)
    {
		//eligible parcels
		std::ofstream* parcelsFile = new std::ofstream("parcels.csv");
		streams.insert(std::make_pair(PARCELS, parcelsFile));
    }

    if(units)
    {
		//units
		std::ofstream* unitsFile = new std::ofstream("units.csv");
		streams.insert(std::make_pair(UNITS, unitsFile));
    }

    if(projects)
    {
		//projects
		std::ofstream* projectsFile = new std::ofstream("projects.csv");
		streams.insert(std::make_pair(PROJECTS, projectsFile));
    }

    if(hh_pc)
    {
		//hhpc postcodes
		std::ofstream* hhpcFile = new std::ofstream("HouseholdPostcodes.csv");
		streams.insert(std::make_pair(HH_PC, hhpcFile));
    }

    if(units_in_market)
    {
		//new units in the market
		std::ofstream* unitsInMarketFile = new std::ofstream("unitsInMarket.csv");
		streams.insert(std::make_pair(UNITS_IN_MARKET, unitsInMarketFile));
    }

    if(log_taxi_availability)
    {
		//hh with taxi availability
		std::ofstream* taxiAvailabilityFile = new std::ofstream("hhWithtaxiAvailability.csv");
		streams.insert(std::make_pair(LOG_TAXI_AVAILABILITY, taxiAvailabilityFile));
    }

    if(log_vehicle_ownership)
    {
		//vehicle ownership options of hh
		std::ofstream* vehicleOwnershipOptionFile = new std::ofstream("hhWithVehicleOwnership.csv");
		streams.insert(std::make_pair(LOG_VEHICLE_OWNERSIP, vehicleOwnershipOptionFile));
    }

    if(log_taz_level_logsum)
    {
		//Tas level group logsum
		std::ofstream* tazLevelLogsumFile = new std::ofstream("tazLevelLogsum.csv");
		streams.insert(std::make_pair(LOG_TAZ_LEVEL_LOGSUM, tazLevelLogsumFile));
    }

    if(log_householdgrouplogsum)
    {
		//household group logsum
		std::ofstream* householdGroupLogsumFile = new std::ofstream("householdGroupLogsum.csv");
		streams.insert(std::make_pair(LOG_HOUSEHOLDGROUPLOGSUM, householdGroupLogsumFile));
    }

    if(log_individual_hits_logsum)
    {
		//individual hits logsum
		std::ofstream* individualHitsLogsumFile = new std::ofstream("IndividualHitsLogsum.csv");
		streams.insert(std::make_pair(LOG_INDIVIDUAL_HITS_LOGSUM, individualHitsLogsumFile));
    }

    if(log_householdbidlist)
    {
		//household bid list
		std::ofstream* householdBidFile = new std::ofstream("householdBidList.csv");
		streams.insert(std::make_pair(LOG_HOUSEHOLDBIDLIST, householdBidFile));
    }

    if(log_individual_logsum_vo)
    {
		//individual hits logsum for vehicle ownership
		std::ofstream* individualHitsLogsumForVOFile = new std::ofstream("IndividualHitsLogsum4VO.csv");
		streams.insert(std::make_pair(LOG_INDIVIDUAL_LOGSUM_VO, individualHitsLogsumForVOFile));
    }

    if(log_screeningprobabilities)
    {
		//screening probabilities
		std::ofstream* screeningProbabilitiesFile = new std::ofstream("ScreeningProbabilities.csv");
		streams.insert(std::make_pair(LOG_SCREENINGPROBABILITIES, screeningProbabilitiesFile));
    }

    if(log_hhchoiceset)
    {
		//HH Choice set
		std::ofstream* hhChoiceSetFile = new std::ofstream("HHChoiceSet.csv");
		streams.insert(std::make_pair(LOG_HHCHOICESET, hhChoiceSetFile));
    }

    if(log_error)
    {
		//errors
		std::ofstream* errorFile = new std::ofstream("Errors.csv");
		streams.insert(std::make_pair(LOG_ERROR, errorFile));
    }

    if(log_school_assignment)
    {
		//school assignment
		std::ofstream* schoolAssignmentFile = new std::ofstream("schools.csv");
		streams.insert(std::make_pair(LOG_SCHOOL_ASSIGNMENT, schoolAssignmentFile));
    }

    if(log_pre_school_assignment)
    {
		//pre school assignment
		std::ofstream* preSchoolAssignmentFile = new std::ofstream("preSchools.csv");
		streams.insert(std::make_pair(LOG_PRE_SCHOOL_ASSIGNMENT, preSchoolAssignmentFile));
    }

    if(log_hh_awakening)
    {
		//awakenings
		std::ofstream* hhawakeningFile = new std::ofstream("HH_Awakenings.csv");
		streams.insert(std::make_pair(LOG_HH_AWAKENING, hhawakeningFile));
    }

    if(log_hh_exit)
    {
		//Exits
		std::ofstream* hhexitsFile = new std::ofstream("HH_Exits.csv");
		streams.insert(std::make_pair(LOG_HH_EXIT, hhexitsFile));
    }

    if(log_random_nums)
    {
		//random nums
		std::ofstream* randomNumFile = new std::ofstream("randomNums.csv");
		streams.insert(std::make_pair(LOG_RANDOM_NUMS, randomNumFile));
    }

    if(log_dev_roi)
    {
		//dev roi
		std::ofstream* roiFile = new std::ofstream("rdevROI.csv");
		streams.insert(std::make_pair(LOG_DEV_ROI, roiFile));
    }

    if(log_household_statistics)
    {
		//household statistics
		std::ofstream* householdStatisticsFile = new std::ofstream("householdStatistics.csv");
		streams.insert(std::make_pair(LOG_HOUSEHOLD_STATISTICS, householdStatisticsFile));
    }

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

std::vector<sim_mob::BufferedBase*> LoggerAgent::buildSubscriptionList() 
{
	return std::vector<sim_mob::BufferedBase*>();
}

void LoggerAgent::onWorkerEnter() {}

void LoggerAgent::onWorkerExit() {}

Entity::UpdateStatus LoggerAgent::update(timeslice now)
{
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}

void LoggerAgent::log(LogFile outputType, const std::string& logMsg)
{
	boost::mutex::scoped_lock lock( mtx );

    	// entry will be available only on the next tick
    	//MessageBus::PostMessage(this, LTMID_LOG, MessageBus::MessagePtr(new LogMsg(logMsg, outputType)));
	
	if (outputType == STDOUT)
	{
		PrintOut(logMsg << std::endl);
	}
	else
	{
	    (*streams[outputType]) << logMsg << std::endl;
		(*streams[outputType]).flush();
	}
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
