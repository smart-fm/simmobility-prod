//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "MT_PersonLoader.hpp"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <map>
#include <sstream>
#include <stdint.h>
#include <utility>
#include <vector>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
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
std::string getRandomTimeInWindow(double mid, bool firstFifteenMins, const std::string pid = "") {
	int hour = int(std::floor(mid));
	int min = 0, max = 29;
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
	RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
	SubTrip aSubTripInTrip;
	aSubTripInTrip.setPersonID(r.get<string>(0));
	aSubTripInTrip.itemType = TripChainItem::IT_TRIP;
	aSubTripInTrip.tripID = parentTrip->tripID + "-" + boost::lexical_cast<string>(subTripNo);
	aSubTripInTrip.origin = WayPoint(rn.getNodeById(r.get<int>(10)));
	aSubTripInTrip.originType = TripChainItem::LT_NODE;
	aSubTripInTrip.destination = WayPoint(rn.getNodeById(r.get<int>(5)));
	aSubTripInTrip.destinationType = TripChainItem::LT_NODE;
	aSubTripInTrip.mode = r.get<string>(6);
	aSubTripInTrip.isPrimaryMode = r.get<int>(7);
	aSubTripInTrip.startTime = parentTrip->startTime;
	parentTrip->addSubTrip(aSubTripInTrip);
}

Activity* MT_PersonLoader::makeActivity(const soci::row& r, unsigned int seqNo)
{
	RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
	Activity* res = new Activity();
	res->setPersonID(r.get<string>(0));
	res->itemType = TripChainItem::IT_ACTIVITY;
	res->sequenceNumber = seqNo;
	res->description = r.get<string>(4);
	res->isPrimary = r.get<int>(7);
	res->isFlexible = false;
	res->isMandatory = true;
	res->destination = WayPoint(rn.getNodeById(r.get<int>(5)));
	res->destinationType = TripChainItem::LT_NODE;
	res->startTime = DailyTime(getRandomTimeInWindow(r.get<double>(8), true));
	res->endTime = DailyTime(getRandomTimeInWindow(r.get<double>(9), false));
	return res;
}

Trip* MT_PersonLoader::makeTrip(const soci::row& r, unsigned int seqNo)
{
	RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
	Trip* tripToSave = new Trip();
	tripToSave->sequenceNumber = seqNo;
	tripToSave->tripID = boost::lexical_cast<string>(r.get<int>(1) * 100 + r.get<int>(3)); //each row corresponds to 1 trip and 1 activity. The tour and stop number can be used to generate unique tripID
	tripToSave->setPersonID(r.get<string>(0));
	tripToSave->itemType = TripChainItem::IT_TRIP;
	tripToSave->origin = WayPoint(rn.getNodeById(r.get<int>(10)));
	tripToSave->originType = TripChainItem::LT_NODE;
	tripToSave->destination = WayPoint(rn.getNodeById(r.get<int>(5)));
	tripToSave->destinationType = TripChainItem::LT_NODE;
	tripToSave->startTime = DailyTime(getRandomTimeInWindow(r.get<double>(11), false, tripToSave->getPersonID()));
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
	//CBD specific processing of trip chain
	//RestrictedRegion::getInstance().processTripChains(tripchains);//todo, plan changed, we are not chopping off the trips here

	Print() << "PeriodicPersonLoader:: activities loaded from " << nextLoadStart << " to " << end << ": " << actCtr << " | new persons loaded: " << personsLoaded << endl;
	Print() << "active_agents: " << activeAgents.size() << " | pending_agents: " << pendingAgents.size() << endl;

	//update next load start
	nextLoadStart = end + DEFAULT_LOAD_INTERVAL;
	if(nextLoadStart > LAST_30MIN_WINDOW_OF_DAY)
	{
		nextLoadStart = nextLoadStart - TWENTY_FOUR_HOURS; //next day starts at 3.25
	}
}


