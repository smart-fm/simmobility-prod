//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "MT_PersonLoader.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <functional>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <stdint.h>
#include <utility>
#include <vector>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/MT_Config.hpp"
#include "conf/Constructs.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Connection.hpp"
#include "database/DB_Config.hpp"
#include "database/predaydao/DatabaseHelper.hpp"
#include "database/predaydao/PopulationSqlDao.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/Log.hpp"
#include "Person_MT.hpp"
#include "util/DailyTime.hpp"
#include "util/Utils.hpp"
#include "util/CSVReader.hpp"
#include "entities/TrainController.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;
using sim_mob::db::DB_Config;
using sim_mob::db::DB_Connection;

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

	//hour ranges from 3 to 26... wrap around to 0 after 23.
	if(hour >= 24)
	{
		hour = hour - 24;
	}
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
	int startHour = int(std::floor(startInterval));
	int min = 0, max = 29;
	int startMinute = Utils::generateInt(min,max) + ((startInterval - startHour - 0.25)*60);
	int startSecond = Utils::generateInt(0,59);

	//construct string representation
	std::string randomStartTime;
	randomStartTime.resize(8); //hh:mi:ss - 8 characters
	char* c = &randomStartTime[0];
	c = timeDecimalDigitToChar(startHour, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(startMinute, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(startSecond, c);
	activity->startTime = sim_mob::DailyTime(randomStartTime);

	if(endInterval <= startInterval)
	{
		min = startMinute - ((startInterval - startHour - 0.25)*60); //max is still 29
	}
	int endHour = int(std::floor(endInterval));
	int endMinute = Utils::generateInt(min,max) + ((endInterval - endHour - 0.25)*60);
	int endSecond = 0;
	if(endHour == startHour && endMinute == startMinute)
	{
		endSecond = Utils::generateInt(startSecond,59);
	}
	else
	{
		endSecond = Utils::generateInt(0,59);
	}

	//construct string representation
	std::string randomEndTime;
	randomEndTime.resize(8); //hh:mi:ss - 8 characters
	c = &randomEndTime[0];
	c = timeDecimalDigitToChar(endHour, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(endMinute, c);
	c++; *c=':'; c++;
	c = timeDecimalDigitToChar(endSecond, c);
	activity->endTime = sim_mob::DailyTime(randomEndTime);
}

DB_Connection getDB_Connection(const DatabaseDetails& dbInfo)
{
	const std::string& dbId = dbInfo.database;
	Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at(dbId);
	const std::string& cred_id = dbInfo.credentials;
	Credential cred = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(cred_id);
	std::string username = cred.getUsername();
	std::string password = cred.getPassword(false);
	DB_Config dbConfig(db.host, db.port, db.dbName, username, password);
	return DB_Connection(sim_mob::db::POSTGRES, dbConfig);
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
	CellLoader():isLoadPersonInfo(false) {}

	/**
	 * function executed by each CellLoader thread
	 */
	void operator()(void)
	{
		id = boost::this_thread::get_id();
		ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();

		for (size_t i = 0; i < tripChainList.size(); i++)
		{
			std::vector<TripChainItem*>& personTripChain = tripChainList[i];
			if (personTripChain.empty()) { continue; }
			Person_MT* person = new Person_MT("DAS_TripChain", cfg.mutexStategy(), personTripChain);
			if (!person->getTripChain().empty())
			{
				//Set the usage of in-simulation travel times
				//Generate random number between 0 and 100 (indicates percentage)
				int randomInt = Utils::generateInt(0, 100);
				if(randomInt <= cfg.simulation.inSimulationTTUsage)
				{
					person->setUseInSimulationTravelTime(true);
				}

				if(isLoadPersonInfo)
				{
					if(person->getDatabaseId().find_first_not_of("0123456789-") == std::string::npos)
					{   // to eliminate any dummy persons we include for background traffic
						// person ids from long-term population are numeric
						std::string::size_type sz;
						DB_Connection populationConn = getDB_Connection(cfg.populationDatabase);
						populationConn.connect();
						if (!populationConn.isConnected())
						{
							throw std::runtime_error("connection to LT population database failed");
						}
						PopulationSqlDao populationDao(populationConn);
						long long personId = std::stol(person->getDatabaseId(), &sz); //gets the numerical part before '-' from the person id
						PersonParams personInfo;
						populationDao.getOneById(personId, personInfo);
						person->setPersonInfo(personInfo);
					}
				}
				persons.push_back(person);
			}
			else
			{
				safe_delete_item(person);
			}
		}
	}

	static int load(std::map<std::string, std::vector<TripChainItem*> >& tripChainMap, std::vector<Person_MT*>& outPersonsLoaded)
	{
		unsigned int numThreads = MT_Config::getInstance().getThreadsNumInPersonLoader();
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
	boost::thread::id id;
	bool isLoadPersonInfo;
};

MT_PersonLoader::MT_PersonLoader(std::set<sim_mob::Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents)
	: PeriodicPersonLoader(activeAgents, pendinAgents),isLoadPersonInfo(false)
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	dataLoadInterval = SECONDS_IN_ONE_HOUR; //1 hour by default. TODO: must be configurable.
	elapsedTimeSinceLastLoad = cfg.baseGranSecond(); // initializing to base gran second so that all subsequent loads will happen 1 tick before the actual start of the interval
	nextLoadStart = getHalfHourWindow(cfg.simulation.simStartTime.getValue()/1000);
	storedProcName = cfg.getDatabaseProcMappings().procedureMappings["day_activity_schedule"];
	freightStoredProcName = cfg.getDatabaseProcMappings().procedureMappings["freight_trips"];

	if(isLoadPersonInfo){
		DB_Connection populationConn = getDB_Connection(cfg.populationDatabase);
		populationConn.connect();
		if (!populationConn.isConnected())
		{
			throw std::runtime_error("connection to LT population database failed");
		}
		PopulationSqlDao populationDao(populationConn);
		populationDao.getIncomeCategories(PersonParams::getIncomeCategoryLowerLimits());
		populationDao.getAddresses(PersonParams::getAddressLookup(), PersonParams::getZoneAddresses());
	}
}

MT_PersonLoader::~MT_PersonLoader()
{
}

void MT_PersonLoader::makeSubTrip(const soci::row& r, Trip* parentTrip, unsigned short subTripNo)
{
	const RoadNetwork* rn = RoadNetwork::getInstance();
	SubTrip subTrip;
	subTrip.setPersonID(r.get<string>(0));
	subTrip.itemType = TripChainItem::IT_TRIP;
	subTrip.tripID = parentTrip->tripID + "-" + boost::lexical_cast<string>(subTripNo);
	subTrip.origin = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(10)));
	subTrip.originType = TripChainItem::LT_NODE;
	subTrip.destination = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(5)));
	subTrip.destinationType = TripChainItem::LT_NODE;
	subTrip.travelMode = r.get<string>(6);
	subTrip.startTime = parentTrip->startTime;
	parentTrip->addSubTrip(subTrip);
}

Activity* MT_PersonLoader::makeActivity(const soci::row& r, unsigned int seqNo)
{
	const RoadNetwork* rn = RoadNetwork::getInstance();
	Activity* activity = new Activity();
	activity->setPersonID(r.get<string>(0));
	activity->itemType = TripChainItem::IT_ACTIVITY;
	activity->sequenceNumber = seqNo;
	activity->purpose = TripChainItem::getItemPurpose(r.get<string>(4));
	activity->isPrimary = r.get<int>(7);
	activity->isFlexible = false;
	activity->isMandatory = true;
	activity->destination = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(5)));
	activity->destinationType = TripChainItem::LT_NODE;
	activity->destinationZoneCode = r.get<int>(13);
	setActivityStartEnd(activity, r.get<double>(8), r.get<double>(9));
	return activity;
}

Trip* MT_PersonLoader::makeTrip(const soci::row& r, unsigned int seqNo)
{
	const RoadNetwork* rn = RoadNetwork::getInstance();
	Trip* trip = new Trip();
	trip->sequenceNumber = seqNo;
	trip->tripID = boost::lexical_cast<string>(r.get<int>(1) * 100 + r.get<int>(3)); //each row corresponds to 1 trip and 1 activity. The tour and stop number can be used to generate unique tripID
	trip->setPersonID(r.get<string>(0));
	trip->itemType = TripChainItem::IT_TRIP;
	trip->purpose = TripChainItem::getItemPurpose(r.get<string>(4));
	trip->origin = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(10)));
	trip->originType = TripChainItem::LT_NODE;
	trip->originZoneCode = r.get<int>(12);
	trip->destination = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<int>(5)));
	trip->destinationType = TripChainItem::LT_NODE;
	trip->destinationZoneCode = r.get<int>(13);
	trip->startTime = DailyTime(getRandomTimeInWindow(r.get<double>(11), false));
	trip->travelMode = r.get<string>(6);
	//just a sanity check
	if(trip->origin == trip->destination)
	{
		Warn() << "Person " << trip->getPersonID() << " has trip " << trip->tripID
			   << " with the same origin and destination. This trip is not loaded.\n";
		safe_delete_item(trip);
		return nullptr;
	}
	makeSubTrip(r, trip);
	return trip;
}

Trip* MT_PersonLoader::makeFreightTrip(const soci::row& r)
{
	const RoadNetwork* rn = RoadNetwork::getInstance();
	Trip* trip = new Trip();
	trip->sequenceNumber = 1;
	trip->tripID = r.get<string>(0);
	trip->setPersonID(r.get<string>(0));
	trip->itemType = TripChainItem::IT_TRIP;
	trip->origin = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<unsigned int>(2)));
	trip->originType = TripChainItem::LT_NODE;
	trip->originZoneCode = r.get<int>(1);
	trip->destination = WayPoint(rn->getById(rn->getMapOfIdvsNodes(), r.get<unsigned int>(4)));
	trip->destinationType = TripChainItem::LT_NODE;
	trip->destinationZoneCode = r.get<int>(3);
	trip->startTime = DailyTime(getRandomTimeInWindow(r.get<double>(5), false));
	trip->travelMode = r.get<string>(6);
	//just a sanity check
	if(trip->origin == trip->destination)
	{
		safe_delete_item(trip);
		return nullptr;
	}

	SubTrip subtrip;
	subtrip.setPersonID(r.get<string>(0));
	subtrip.itemType = TripChainItem::IT_TRIP;
	subtrip.tripID = trip->tripID + "-" + boost::lexical_cast<string>(1);
	subtrip.origin = trip->origin;
	subtrip.originType = TripChainItem::LT_NODE;
	subtrip.destination = trip->destination;
	subtrip.destinationType = TripChainItem::LT_NODE;
	subtrip.travelMode = trip->travelMode;
	subtrip.startTime = trip->startTime;
	trip->addSubTrip(subtrip);

	return trip;
}
void MT_PersonLoader::loadMRT_Demand()
{
	static bool isLoaded = false;
	if (isLoaded)
	{
		return;
	}
	isLoaded = true;
	std::string filename = "MRTTrips.csv";
	CSV_Reader variablesReader(filename, true);
	boost::unordered_map<std::string, std::string> variableRow;
	variablesReader.getNextRow(variableRow, false);
	map<string, vector<TripChainItem*> > tripchains;
	TrainController<Person_MT>* trainController = TrainController<Person_MT>::getInstance();
	while (!variableRow.empty())
	{
		try
		{
			std::string personId = variableRow.at("person_id");
			std::string sStartPlatform = variableRow.at("start_platform");
			std::string sEndPlatform = variableRow.at("end_platform");
			std::string sStartTime = variableRow.at("start_time");
			std::string lineId = variableRow.at("line_id");
			sim_mob::Platform* sPlatform = trainController->getPlatformFromId(sStartPlatform);
			sim_mob::Platform* ePlatform = trainController->getPlatformFromId(sEndPlatform);
			if (!sPlatform || !ePlatform)
			{
				throw std::runtime_error("csv do not include correct platform");
			}
			if (tripchains.count(personId) > 0)
			{
				throw std::runtime_error("csv include duplicated person id");
			}

			Trip* trip = new Trip();
			int startTime = boost::lexical_cast<int>(sStartTime);
			trip->startTime = DailyTime(startTime * 1000);
			trip->itemType = TripChainItem::IT_TRIP;
			trip->travelMode = "MRT";
			trip->serviceLine = lineId;
			trip->setPersonID(personId);
			trip->origin = WayPoint(trainController->getStationFromId(sPlatform->getStationNo()));
			trip->originType = TripChainItem::LT_PUBLIC_TRANSIT_STOP;
			trip->startLocationId = sPlatform->getStationNo();
			trip->destination = WayPoint(trainController->getStationFromId(ePlatform->getStationNo()));
			trip->destinationType = TripChainItem::LT_PUBLIC_TRANSIT_STOP;
			trip->endLocationId = ePlatform->getStationNo();

			SubTrip subTrip;
			subTrip.itemType = TripChainItem::IT_TRIP;
			subTrip.origin = WayPoint(sPlatform);
			subTrip.destination = WayPoint(ePlatform);
			subTrip.originType = TripChainItem::LT_PUBLIC_TRANSIT_STOP;
			subTrip.destinationType = TripChainItem::LT_PUBLIC_TRANSIT_STOP;
			subTrip.travelMode = "WaitingTrainActivity";
			subTrip.startTime = trip->startTime;
			subTrip.serviceLine = lineId;
			subTrip.startLocationType = "PT";
			subTrip.endLocationType = "PT";
			subTrip.startLocationId = sPlatform->getPlatformNo();
			subTrip.endLocationId = ePlatform->getPlatformNo();
			trip->addSubTrip(subTrip);

			subTrip.travelMode = "MRT";
			trip->addSubTrip(subTrip);
			tripchains[personId].push_back(trip);
		} catch (const std::out_of_range& oor) {
			throw std::runtime_error("Header mis-match while reading MRT demand csv");
		} catch (boost::bad_lexical_cast const&) {
			throw std::runtime_error("Invalid value found in MRT demand csv");
		}

		variableRow.clear();
		variablesReader.getNextRow(variableRow, false);
	}

	vector<Person_MT*> persons;
	int personsLoaded = CellLoader::load(tripchains, persons);
	for (vector<Person_MT*>::iterator i = persons.begin(); i != persons.end();i++)
	{
		addOrStashPerson(*i);
	}

	Print() << "PersonLoader:: MRT loaded " << personsLoaded << endl;
	Print() << "active_agents: " << activeAgents.size() << " | pending_agents: "<< pendingAgents.size() << endl;
}
void MT_PersonLoader::loadPersonDemand()
{
	if(storedProcName.empty())
	{
		loadMRT_Demand();
		return;
	}
	//Our SQL statement
	stringstream query;
	double end = nextLoadStart + DEFAULT_LOAD_INTERVAL;
	query << "select * from " << storedProcName << "(" << nextLoadStart << "," << end << ")";
	std::string sql_str = query.str();
	soci::session sql_(soci::postgresql, ConfigManager::GetInstanceRW().FullConfig().getDatabaseConnectionString(false));

	soci::rowset<soci::row> rs = (sql_.prepare << sql_str);
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	unsigned actCtr = 0, tripsNotConstructed = 0;
	map<string, vector<TripChainItem*> > tripchains;

	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		const soci::row &r = (*it);
		std::string personId = r.get<string>(0);
		bool isLastInSchedule =
				(r.get<double>(9) == LAST_30MIN_WINDOW_OF_DAY) && (r.get<string>(4) == HOME_ACTIVITY_TYPE);
		std::vector<TripChainItem *> &personTripChain = tripchains[personId];
		//add trip and activity
		unsigned int seqNo = personTripChain.size(); //seqNo of last trip chain item
		sim_mob::Trip *constructedTrip = makeTrip(r, ++seqNo);

		if (constructedTrip)
		{
			personTripChain.push_back(constructedTrip);
		}
		else
		{
			tripsNotConstructed++;
			continue;
		}

		if (!isLastInSchedule)
		{
			personTripChain.push_back(makeActivity(r, ++seqNo));
		}

		actCtr++;
	}

	//Record the total number of persons loaded and the trips not loaded from the day activity schedule
	ConfigParams &configParams = ConfigManager::GetInstanceRW().FullConfig();
	configParams.numTripsLoaded += actCtr;
	configParams.numTripsNotLoaded += tripsNotConstructed;
	configParams.numPersonsLoaded += tripchains.size();


	if (!freightStoredProcName.empty())
	{
		//Our SQL statement
		stringstream freightQuery;
		freightQuery << "select * from " << freightStoredProcName << "(" << nextLoadStart << "," << end << ")";
		std::string freightSql_str = freightQuery.str();

		soci::rowset<soci::row> rsFreight = (sql_.prepare << freightSql_str);
		for (soci::rowset<soci::row>::const_iterator it=rsFreight.begin(); it!=rsFreight.end(); ++it)
		{
			const soci::row& r = (*it);
			std::string freightTripId = r.get<string>(0);
			std::vector<TripChainItem*>& personTripChain = tripchains[freightTripId];
			//add trip and activity
			sim_mob::Trip* constructedTrip = makeFreightTrip(r);
			if(constructedTrip)
			{
				personTripChain.push_back(constructedTrip);
			}
			else
			{
				continue;
			}
		}
	}

	vector<Person_MT*> persons;
	int personsLoaded = CellLoader::load(tripchains, persons);

	for(vector<Person_MT*>::iterator i=persons.begin(); i!=persons.end(); i++)
	{
		addOrStashPerson(*i);
	}

	//update next load start
	nextLoadStart = end + DEFAULT_LOAD_INTERVAL;
	if(nextLoadStart > LAST_30MIN_WINDOW_OF_DAY)
	{
		nextLoadStart = nextLoadStart - TWENTY_FOUR_HOURS; //next day starts at 3.25
	}
}

