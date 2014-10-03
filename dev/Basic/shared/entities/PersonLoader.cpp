//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonLoader.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <map>
#include <sstream>
#include <stdint.h>
#include <vector>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "Person.hpp"
#include "logging/Log.hpp"
#include "misc/TripChain.hpp"
#include "util/DailyTime.hpp"
#include "util/Utils.hpp"

using namespace std;
using namespace sim_mob;

namespace
{
	// for convenience. see loadActivitySchedule function
	const double DEFAULT_LOAD_INTERVAL = 0.5; // 0.5, when added to the 30 min representation explained below, will span for 1 hour in our query

	const double LAST_30MIN_WINDOW_OF_DAY = 26.75;
	const string HOME_ACTIVITY_TYPE = "Home";
	const unsigned int SECONDS_IN_ONE_HOUR = 3600;

	/**
	 * given a time value in seconds measured from 00:00:00 (12AM)
	 * this function returns a numeric representation of the half hour window of the day
	 * the time belongs to.
	 * The 48 numeric representations of the day are as follows
	 * 3.25 = (3:00 - 3:29)
	 * 3.75 = (3:30 - 3:59)
	 * 4.25 = (4:00 - 4:29)
	 * 4.75 = (4:30 - 4:59)
	 * .
	 * . and so on
	 * .
	 * 23.75 = (23:30 - 23:59)
	 * 24:25 = (0:00 - 0:29) of next day
	 * 24.75 = (0:30 - 0:59)
	 * 25.25 = (1:00 - 1:29)
	 * .
	 * . and so on
	 * .
	 * 26.75 = (2:30 - 2:59)
	 */
	double getHalfHourWindow(uint32_t time) //time is in seconds
	{
		uint32_t hour = time / SECONDS_IN_ONE_HOUR;
		uint32_t remainder = time % SECONDS_IN_ONE_HOUR;
		uint32_t minutes = remainder/60;
		if(hour < 3) { hour = hour+24; }
		if(minutes < 30) { return (hour + 0.25); }
		else { return (hour + 0.75); }
	}

	/**
	 * generates a random time within the time window passed in preday's representation.
	 *
	 * @param mid time window in preday format (E.g. 4.75 => 4:30 to 4:59 AM)
	 * @param firstFifteenMins flag to restrict random time to first fifteen minutes of 30 minute window.
	 * 							This is useful in case of activities which have the same arrival and departure window
	 * 							The arrival time can be chosen in the first 15 minutes and dep. time can be chosen in the 2nd 1 mins of the window
	 * @return a random time within the window in hh24:mm:ss format
	 */
	std::string getRandomTimeInWindow(double mid, bool firstFifteenMins) {
		int hour = int(std::floor(mid));
		int min = 15, max = 29;
		if(firstFifteenMins) { min = 0; max = 14; }
		int minute = Utils::generateInt(min,max) + ((mid - hour - 0.25)*60);
		int second = Utils::generateInt(0,60);
		std::stringstream random_time;
		hour = hour % 24;
		if (hour < 10) { random_time << "0"; }
		random_time << hour << ":";
		if (minute < 10) { random_time << "0"; }
		random_time << minute << ":";
		if(second < 10) { random_time << "0"; }
		random_time << second;
		return random_time.str(); //HH24:MI:SS format
	}

	/**
	 * makes a single sub trip for trip (for now)
	 * @param r row from database table
	 * @param parentTrip parent Trip for the subtrip to be constructed
	 * @param subTripNo the sub trip number
	 */
	void makeSubTrip(const soci::row& r, sim_mob::Trip* parentTrip, unsigned short subTripNo=1)
	{
		sim_mob::RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
		sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
		sim_mob::SubTrip aSubTripInTrip;
		aSubTripInTrip.setPersonID(r.get<string>(0));
		aSubTripInTrip.itemType = sim_mob::TripChainItem::IT_TRIP;
		aSubTripInTrip.tripID = parentTrip->tripID + "-" + boost::lexical_cast<string>(subTripNo);
		aSubTripInTrip.fromLocation = sim_mob::WayPoint(rn.getNodeById(r.get<int>(10)));
		aSubTripInTrip.fromLocationType = sim_mob::TripChainItem::LT_NODE;
		aSubTripInTrip.toLocation = sim_mob::WayPoint(rn.getNodeById(r.get<int>(5)));
		aSubTripInTrip.toLocationType = sim_mob::TripChainItem::LT_NODE;
		aSubTripInTrip.mode = r.get<string>(6);
		aSubTripInTrip.isPrimaryMode = r.get<int>(7);
		aSubTripInTrip.startTime = parentTrip->startTime;
		parentTrip->addSubTrip(aSubTripInTrip);
	}

	sim_mob::Activity* makeActivity(const soci::row& r, unsigned int seqNo)
	{
		sim_mob::RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
		sim_mob::Activity* res = new sim_mob::Activity();
		res->setPersonID(r.get<string>(0));
		res->itemType = sim_mob::TripChainItem::IT_ACTIVITY;
		res->sequenceNumber = seqNo;
		res->description = r.get<string>(4);
		res->isPrimary = r.get<int>(7);
		res->isFlexible = false;
		res->isMandatory = true;
		res->location = rn.getNodeById(r.get<int>(5));
		res->locationType = sim_mob::TripChainItem::LT_NODE;
		res->startTime = sim_mob::DailyTime(getRandomTimeInWindow(r.get<double>(8), true));
		res->endTime = sim_mob::DailyTime(getRandomTimeInWindow(r.get<double>(9), false));
		return res;
	}

	sim_mob::Trip* makeTrip(const soci::row& r, unsigned int seqNo)
	{
		sim_mob::RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
		sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
		sim_mob::Trip* tripToSave = new sim_mob::Trip();
		tripToSave->sequenceNumber = seqNo;
		tripToSave->tripID = boost::lexical_cast<string>(r.get<int>(1) * 100 + r.get<int>(3)); //each row corresponds to 1 trip and 1 activity. The tour and stop number can be used to generate unique tripID
		tripToSave->setPersonID(r.get<string>(0));
		tripToSave->itemType = sim_mob::TripChainItem::IT_TRIP;
		tripToSave->fromLocation = sim_mob::WayPoint(rn.getNodeById(r.get<int>(10)));
		tripToSave->fromLocationType = sim_mob::TripChainItem::LT_NODE;
		tripToSave->toLocation = sim_mob::WayPoint(rn.getNodeById(r.get<int>(5)));
		tripToSave->toLocationType = sim_mob::TripChainItem::LT_NODE;
		tripToSave->startTime = sim_mob::DailyTime(getRandomTimeInWindow(r.get<double>(11), true));
		makeSubTrip(r, tripToSave);
		return tripToSave;
	}
}

sim_mob::PeriodicPersonLoader::PeriodicPersonLoader(std::set<sim_mob::Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents)
	: activeAgents(activeAgents), pendingAgents(pendinAgents),
	  sql_(soci::postgresql, ConfigManager::GetInstanceRW().FullConfig().getDatabaseConnectionString(false))
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	dataLoadInterval = SECONDS_IN_ONE_HOUR; //1 hour by default. TODO: must be configurable.
	elapsedTimeSinceLastLoad = cfg.baseGranSecond(); // initializing to base gran second so that all subsequent loads will happen 1 tick before the actual start of the interval

	//TODO:  This is rigid. Must do something about relating this variable to simulation time and extracting the 30 min representation when we actually load.
	//we assume the simulation does not start before 3AM (the start of day for Preday)
	nextLoadStart = getHalfHourWindow(cfg.system.simulation.simStartTime.getValue()/1000);

	storedProcName = cfg.getDatabaseProcMappings().procedureMappings["day_activity_schedule"];
}

sim_mob::PeriodicPersonLoader::~PeriodicPersonLoader()
{}

void sim_mob::PeriodicPersonLoader::loadActivitySchedules()
{
	if (storedProcName.empty()) { return; }
	//Our SQL statement
	stringstream query;
	double end = nextLoadStart + DEFAULT_LOAD_INTERVAL;
	query << "select * from " << storedProcName << "(" << nextLoadStart << "," << end << ")";
	std::string sql_str = query.str();
	soci::rowset<soci::row> rs = (sql_.prepare << sql_str);
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	unsigned actCtr = 0;
	map<string, vector<TripChainItem*> > tripchains;
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		const soci::row& r = (*it);
		std::string personId = r.get<string>(0);
		bool isLastInSchedule = (r.get<double>(9)==LAST_30MIN_WINDOW_OF_DAY) && (r.get<string>(4)==HOME_ACTIVITY_TYPE);
		std::vector<TripChainItem*>& personTripChain = tripchains[personId];
		//add trip and activity
		unsigned int seqNo = personTripChain.size(); //seqNo of last trip chain item
		personTripChain.push_back(makeTrip(r, ++seqNo));
		if(!isLastInSchedule) { personTripChain.push_back(makeActivity(r, ++seqNo)); }
		actCtr++;
	}

	//add or stash new persons
	for(map<string, vector<TripChainItem*> >::iterator i=tripchains.begin(); i!=tripchains.end(); i++)
	{
		Person* person = new Person("DAS_TripChain", cfg.mutexStategy(), i->second);
		addOrStashPerson(person);
	}

	Print() << "PeriodicPersonLoader:: activities loaded from " << nextLoadStart << " to " << end << ": " << actCtr
			<< " | new persons loaded: " << tripchains.size() << endl;

	Print() << "active_agents: " << activeAgents.size() << " | pending_agents: " << pendingAgents.size() << endl;
	//update next load start
	nextLoadStart = end + DEFAULT_LOAD_INTERVAL;
}

void sim_mob::PeriodicPersonLoader::addOrStashPerson(Person* p)
{
	//Only agents with a start time of zero should start immediately in the all_agents list.
	if (p->getStartTime()==0) //TODO: Check if this condition will suffice here
	{
		p->load(p->getConfigProperties());
		p->clearConfigProperties();
		activeAgents.insert(p);
	}
	else
	{
		//Start later.
		pendingAgents.push(p);
	}
}

bool sim_mob::PeriodicPersonLoader::checkTimeForNextLoad()
{
	elapsedTimeSinceLastLoad += ConfigManager::GetInstance().FullConfig().baseGranSecond();
	if(elapsedTimeSinceLastLoad >= dataLoadInterval)
	{
		elapsedTimeSinceLastLoad = 0;
		return true;
	}
	return false;
}
