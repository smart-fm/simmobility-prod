//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "MT_PersonLoader.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <stdint.h>
#include <utility>
#include <vector>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/Log.hpp"
#include "Person_MT.hpp"
#include "util/DailyTime.hpp"
#include "util/Utils.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
// for convenience. see loadActivitySchedule function
const double DEFAULT_LOAD_INTERVAL = 0.5; // 0.5, when added to the 30 min representation explained below, will span for 1 hour in our query
const unsigned int SECONDS_IN_ONE_HOUR = 3600;
const double TWENTY_FOUR_HOURS = 24.0;
const double LAST_30MIN_WINDOW_OF_DAY = 26.75;
const string HOME_ACTIVITY_TYPE = "Home";

/**
 * a verbose, dangerous and yet fast helper function to get characters corresponding to decimal numbers 0 through 59
 * buildStringRepr() uses this function to quickly get the string representation for a time value in milliseconds
 */
inline char* timeDecimalDigitToChar(int num, char* c)
{
	switch(num)
	{
		case 0: { *c='0'; c++; *c='0'; break; }
		case 1: { *c='0'; c++; *c='1'; break; }
		case 2: { *c='0'; c++; *c='2'; break; }
		case 3: { *c='0'; c++; *c='3'; break; }
		case 4: { *c='0'; c++; *c='4'; break; }
		case 5: { *c='0'; c++; *c='5'; break; }
		case 6: { *c='0'; c++; *c='6'; break; }
		case 7: { *c='0'; c++; *c='7'; break; }
		case 8: { *c='0'; c++; *c='8'; break; }
		case 9: { *c='0'; c++; *c='9'; break; }
		case 10: { *c='1'; c++; *c='0'; break; }
		case 11: { *c='1'; c++; *c='1'; break; }
		case 12: { *c='1'; c++; *c='2'; break; }
		case 13: { *c='1'; c++; *c='3'; break; }
		case 14: { *c='1'; c++; *c='4'; break; }
		case 15: { *c='1'; c++; *c='5'; break; }
		case 16: { *c='1'; c++; *c='6'; break; }
		case 17: { *c='1'; c++; *c='7'; break; }
		case 18: { *c='1'; c++; *c='8'; break; }
		case 19: { *c='1'; c++; *c='9'; break; }
		case 20: { *c='2'; c++; *c='0'; break; }
		case 21: { *c='2'; c++; *c='1'; break; }
		case 22: { *c='2'; c++; *c='2'; break; }
		case 23: { *c='2'; c++; *c='3'; break; }
		case 24: { *c='2'; c++; *c='4'; break; }
		case 25: { *c='2'; c++; *c='5'; break; }
		case 26: { *c='2'; c++; *c='6'; break; }
		case 27: { *c='2'; c++; *c='7'; break; }
		case 28: { *c='2'; c++; *c='8'; break; }
		case 29: { *c='2'; c++; *c='9'; break; }
		case 30: { *c='3'; c++; *c='0'; break; }
		case 31: { *c='3'; c++; *c='1'; break; }
		case 32: { *c='3'; c++; *c='2'; break; }
		case 33: { *c='3'; c++; *c='3'; break; }
		case 34: { *c='3'; c++; *c='4'; break; }
		case 35: { *c='3'; c++; *c='5'; break; }
		case 36: { *c='3'; c++; *c='6'; break; }
		case 37: { *c='3'; c++; *c='7'; break; }
		case 38: { *c='3'; c++; *c='8'; break; }
		case 39: { *c='3'; c++; *c='9'; break; }
		case 40: { *c='4'; c++; *c='0'; break; }
		case 41: { *c='4'; c++; *c='1'; break; }
		case 42: { *c='4'; c++; *c='2'; break; }
		case 43: { *c='4'; c++; *c='3'; break; }
		case 44: { *c='4'; c++; *c='4'; break; }
		case 45: { *c='4'; c++; *c='5'; break; }
		case 46: { *c='4'; c++; *c='6'; break; }
		case 47: { *c='4'; c++; *c='7'; break; }
		case 48: { *c='4'; c++; *c='8'; break; }
		case 49: { *c='4'; c++; *c='9'; break; }
		case 50: { *c='5'; c++; *c='0'; break; }
		case 51: { *c='5'; c++; *c='1'; break; }
		case 52: { *c='5'; c++; *c='2'; break; }
		case 53: { *c='5'; c++; *c='3'; break; }
		case 54: { *c='5'; c++; *c='4'; break; }
		case 55: { *c='5'; c++; *c='5'; break; }
		case 56: { *c='5'; c++; *c='6'; break; }
		case 57: { *c='5'; c++; *c='7'; break; }
		case 58: { *c='5'; c++; *c='8'; break; }
		case 59: { *c='5'; c++; *c='9'; break; }
	}
	return c;
}

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
std::string getRandomTimeInWindow(double mid, bool firstFifteenMins)
{
	int hour = int(std::floor(mid));
	int min = 0, max = 29;
	if(firstFifteenMins) { min = 0; max = 14; }
	int minute = Utils::generateInt(min,max) + ((mid - hour - 0.25)*60);
	int second = Utils::generateInt(0,59);

	//construct string representation
	std::string random_time;
	random_time.resize(8); //hh:mi:ss - 8 characters
	char* c = &random_time[0];
	c = timeDecimalDigitToChar(hour, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(minute, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(second, c);
	return random_time; //HH24:MI:SS format
}

void setActivityStartEnd(sim_mob::Activity* activity, double startInterval, double endInterval)
{
	int hour = int(std::floor(startInterval));
	int min = 0, max = 29;
	int minute = Utils::generateInt(min,max) + ((startInterval - hour - 0.25)*60);
	int second = Utils::generateInt(0,59);

	//construct string representation
	std::string randomStartTime;
	randomStartTime.resize(8); //hh:mi:ss - 8 characters
	char* c = &randomStartTime[0];
	c = timeDecimalDigitToChar(hour, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(minute, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(second, c);
	activity->startTime = sim_mob::DailyTime(randomStartTime);

	if(endInterval <= startInterval)
	{
		min = minute - ((startInterval - hour - 0.25)*60); //max is still 29
	}
	hour = int(std::floor(endInterval));
	minute = Utils::generateInt(min,max) + ((endInterval - hour - 0.25)*60);
	second = Utils::generateInt(0,59);

	//construct string representation
	std::string randomEndTime;
	randomEndTime.resize(8); //hh:mi:ss - 8 characters
	c = &randomEndTime[0];
	c = timeDecimalDigitToChar(hour, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(minute, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(second, c);
	activity->endTime = sim_mob::DailyTime(randomEndTime);
}

}//anon namespace

/**
 * Parallel DAS loader
 *
 * \author Zhang Huai Peng
 * \author Harish Loganathan
 */
class CellLoader
{
public:
	CellLoader() {}

	/**
	 * function executed by each CellLoader thread
	 */
	void operator()(void)
	{
		id = boost::this_thread::get_id();
		for (size_t i = 0; i < tripChainList.size(); i++)
		{
			std::vector<TripChainItem*>& personTripChain = tripChainList[i];
			if (personTripChain.empty()) { continue; }
			ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
			Person_MT* person = new Person_MT("DAS_TripChain", cfg.mutexStategy(), personTripChain);
			if (!person->getTripChain().empty())
			{
				persons.push_back(person);
			}
			else
			{
				delete person;
			}
		}
		Print() << "Thread " <<  id << " loaded "<< persons.size() << " persons" << std::endl;
	}

	static int load(std::map<std::string, std::vector<TripChainItem*> >& tripChainMap, std::vector<Person_MT*>& outPersonsLoaded)
	{
		int personsPerThread = tripChainMap.size() / numThreads;
		CellLoader thread[numThreads];
		boost::thread_group threadGroup;
		int thIdx = 0;
		for(std::map<std::string, std::vector<TripChainItem*> >::iterator tcMapIt=tripChainMap.begin(); tcMapIt!=tripChainMap.end(); tcMapIt++, thIdx=(thIdx+1)%numThreads)
		{
			thread[thIdx].tripChainList.push_back(tcMapIt->second);
		}
		for (int i = 0; i < numThreads; i++)
		{
			threadGroup.add_thread(new boost::thread(boost::ref(thread[i])));
		}
		threadGroup.join_all();
		for (int i = 0; i < numThreads; i++)
		{
			outPersonsLoaded.insert(outPersonsLoaded.end(), thread[i].persons.begin(), thread[i].persons.end());
		}
		return outPersonsLoaded.size();
	}

private:
	std::vector<Person_MT*> persons;
	std::vector<std::vector<TripChainItem*> > tripChainList;
	static const int numThreads = 20;
	boost::thread::id id;
};

MT_PersonLoader::MT_PersonLoader(std::set<sim_mob::Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents)
	: PeriodicPersonLoader(activeAgents, pendinAgents)
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	dataLoadInterval = SECONDS_IN_ONE_HOUR; //1 hour by default. TODO: must be configurable.
	elapsedTimeSinceLastLoad = cfg.baseGranSecond(); // initializing to base gran second so that all subsequent loads will happen 1 tick before the actual start of the interval
	nextLoadStart = getHalfHourWindow(cfg.simulation.simStartTime.getValue()/1000);
	storedProcName = cfg.getDatabaseProcMappings().procedureMappings["day_activity_schedule"];
}

MT_PersonLoader::~MT_PersonLoader()
{
}

void MT_PersonLoader::makeSubTrip(const soci::row& r, Trip* parentTrip, unsigned short subTripNo)
{
	const RoadNetwork* rn = RoadNetwork::getInstance();
	SubTrip aSubTripInTrip;
	aSubTripInTrip.setPersonID(r.get<string>(0));
	aSubTripInTrip.itemType = TripChainItem::IT_TRIP;
	aSubTripInTrip.tripID = parentTrip->tripID + "-" + boost::lexical_cast<string>(subTripNo);
	aSubTripInTrip.origin = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(10)));
	aSubTripInTrip.originType = TripChainItem::LT_NODE;
	aSubTripInTrip.destination = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(5)));
	aSubTripInTrip.destinationType = TripChainItem::LT_NODE;
	aSubTripInTrip.mode = r.get<string>(6);
	aSubTripInTrip.isPrimaryMode = r.get<int>(7);
	aSubTripInTrip.startTime = parentTrip->startTime;
	parentTrip->addSubTrip(aSubTripInTrip);
}

Activity* MT_PersonLoader::makeActivity(const soci::row& r, unsigned int seqNo)
{
	const RoadNetwork* rn = RoadNetwork::getInstance();
	Activity* res = new Activity();
	res->setPersonID(r.get<string>(0));
	res->itemType = TripChainItem::IT_ACTIVITY;
	res->sequenceNumber = seqNo;
	res->description = r.get<string>(4);
	res->isPrimary = r.get<int>(7);
	res->isFlexible = false;
	res->isMandatory = true;
	res->destination = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(5)));
	res->destinationType = TripChainItem::LT_NODE;
	setActivityStartEnd(res, r.get<double>(8), r.get<double>(9));
	return res;
}

Trip* MT_PersonLoader::makeTrip(const soci::row& r, unsigned int seqNo)
{
	const RoadNetwork* rn = RoadNetwork::getInstance();
	Trip* tripToSave = new Trip();
	tripToSave->sequenceNumber = seqNo;
	tripToSave->tripID = boost::lexical_cast<string>(r.get<int>(1) * 100 + r.get<int>(3)); //each row corresponds to 1 trip and 1 activity. The tour and stop number can be used to generate unique tripID
	tripToSave->setPersonID(r.get<string>(0));
	tripToSave->itemType = TripChainItem::IT_TRIP;
	tripToSave->origin = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(10)));
	tripToSave->originType = TripChainItem::LT_NODE;
	tripToSave->destination = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(5)));
	tripToSave->destinationType = TripChainItem::LT_NODE;
	tripToSave->startTime = DailyTime(getRandomTimeInWindow(r.get<double>(11), false));
	//just a sanity check
	if(tripToSave->origin == tripToSave->destination)
	{
		safe_delete_item(tripToSave);
		return nullptr;
	}
	makeSubTrip(r, tripToSave);
	return tripToSave;
}

void MT_PersonLoader::loadPersonDemand()
{
	if (storedProcName.empty()) { return; }
	//Our SQL statement
	stringstream query;
	double end = nextLoadStart + DEFAULT_LOAD_INTERVAL;
	query << "select * from " << storedProcName << "(" << nextLoadStart << "," << end << ")";
	std::string sql_str = query.str();
	soci::session sql_(soci::postgresql, ConfigManager::GetInstanceRW().FullConfig().getDatabaseConnectionString(false));

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
		sim_mob::Trip* constructedTrip = makeTrip(r, ++seqNo);
		if(constructedTrip) { personTripChain.push_back(constructedTrip); }
		else { continue; }
		if(!isLastInSchedule) { personTripChain.push_back(makeActivity(r, ++seqNo)); }
		actCtr++;
	}

	vector<Person_MT*> persons;
	int personsLoaded = CellLoader::load(tripchains, persons);
	for(vector<Person_MT*>::iterator i=persons.begin(); i!=persons.end(); i++)
	{
		addOrStashPerson(*i);
	}

	Print() << "PeriodicPersonLoader:: activities loaded from " << nextLoadStart << " to " << end << ": " << actCtr << " | new persons loaded: " << personsLoaded << endl;
	Print() << "active_agents: " << activeAgents.size() << " | pending_agents: " << pendingAgents.size() << endl;

	//update next load start
	nextLoadStart = end + DEFAULT_LOAD_INTERVAL;
	if(nextLoadStart > LAST_30MIN_WINDOW_OF_DAY)
	{
		nextLoadStart = nextLoadStart - TWENTY_FOUR_HOURS; //next day starts at 3.25
	}
}


