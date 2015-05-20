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
#include <utility>
#include <vector>
#include <soci.h>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "logging/Log.hpp"
#include "misc/TripChain.hpp"
#include "Person.hpp"
#include "util/DailyTime.hpp"
#include "util/Utils.hpp"

using namespace std;
using namespace sim_mob;

namespace
{
	// for convenience. see loadActivitySchedule function
	const double DEFAULT_LOAD_INTERVAL = 0.5; // 0.5, when added to the 30 min representation explained below, will span for 1 hour in our query

	const double LAST_30MIN_WINDOW_OF_DAY = 26.75;
	const double TWENTY_FOUR_HOURS = 24.0;
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

}//namespace

/*************************************************************************************
 * 						Restricted Region Tripchain processing
 * ***********************************************************************************
 */
boost::shared_ptr<sim_mob::RestrictedRegion> sim_mob::RestrictedRegion::instance;
sim_mob::RestrictedRegion::RestrictedRegion():Impl(new TagSearch(*this)),zoneSegmentsStr(""),zoneNodesStr(""),inStr(""), outStr("")
{}
sim_mob::RestrictedRegion::~RestrictedRegion(){safe_delete_item(Impl);}
void sim_mob::RestrictedRegion::populate()
{
	//skip if already populated
	if(!populated.check())
	{
		return;
	}
	sim_mob::aimsun::Loader::getCBD_Border(in,out);
	sim_mob::aimsun::Loader::getCBD_Segments(zoneSegments);
	//zone nodes = start and end nodes of zoneSegments  minus border-in&out->from_section->toNode()
	BOOST_FOREACH(const sim_mob::RoadSegment*rs,zoneSegments)
	{
		zoneNodes[boost::lexical_cast<std::string>(rs->getStart()->getID())] = rs->getStart();
		zoneNodes[boost::lexical_cast<std::string>(rs->getEnd()->getID())] = rs->getEnd();
	}

	BOOST_FOREACH(SegPair item, in)
	{
		zoneNodes.erase(boost::lexical_cast<std::string>(item.first->getEnd()->getID()));
	}
	BOOST_FOREACH(SegPair item, out)
	{
		zoneNodes.erase(boost::lexical_cast<std::string>(item.first->getEnd()->getID()));
	}

	typedef std::map<std::string, const Node*>::value_type Pair;
//	cout << "CBD Entering border Sections size: " << in.size() << "\n";
//	cout << "CBD Exitting border Sections size: " << out.size() << "\n";
//	cout << "Total segments in CBD Area: " << zoneSegments.size() << "\n";
//	cout << "Total nodes in CBD Area: " << zoneNodes.size() << "\n";
	/********************************************************
	 * ********** String representations & Tagging **********
	 * ******************************************************/
	std::stringstream out_("");
	BOOST_FOREACH(const sim_mob::RoadSegment*rs,zoneSegments)
	{
		out_ << rs->getId() << ",";
		rs->CBD = true;
	}
	zoneSegmentsStr = out_.str();
	std::cout << "zoneSegmentsStr :" << zoneSegmentsStr << std::endl;
	out_.str("");
	BOOST_FOREACH(Pair node, zoneNodes)
	{
		zoneNodesStr += node.first + ",";
		node.second->CBD = true;
	}
	std::cout << "zoneNodesStr: " << zoneNodesStr << std::endl;
	out_.str("");
	BOOST_FOREACH(SegPair item, in)
	{
		out_ << item.first->getId() << ":" << item.second->getId() << ",";
	}
	inStr = out_.str();
	out_.str("");
	BOOST_FOREACH(SegPair item, out)
	{
		out_ << item.first->getId() << ":" << item.second->getId() << ",";
	}
	outStr = out_.str();
	out_.str("");

}
//obsolete
void sim_mob::RestrictedRegion::processTripChains(map<string, vector<TripChainItem*> > &tripchains)
{
	typedef std::map<string, vector<TripChainItem*> >::value_type TCI;
	//iterate person
	BOOST_FOREACH(TCI &item, tripchains)
	{
		//iterate person's tripchain
		vector<TripChainItem*> &tcs = item.second;
		BOOST_FOREACH(TripChainItem* tci, tcs)
		{
			if(tci->itemType != sim_mob::TripChainItem::IT_TRIP)
			{
				continue;
			}
			std::vector<sim_mob::SubTrip>& subTrips = static_cast<Trip*>(tci)->getSubTripsRW();
			processSubTrips(subTrips);
		}
	}
}
//obsolete
void sim_mob::RestrictedRegion::processSubTrips(std::vector<sim_mob::SubTrip>& subTrips)
{
	for(int i = 0; i < subTrips.size(); i++)
	{
		bool o = isInRestrictedZone(subTrips[i].fromLocation);
		bool d = isInRestrictedZone(subTrips[i].toLocation);
		//	if either origin OR destination lie in the restricted zone (not both)
		if((o || d) && !(o && d))
		{
			//create an extra subtrip for the restricted zone travel(restrictedSubTrip)
			sim_mob::SubTrip restrictedSubTrip(subTrips[i]);
			//origin in the restricted area
			if(o != false)
			{
//				WayPoint wpo(o);
				restrictedSubTrip.toLocation = subTrips[i].fromLocation/*wpo*/;
//				subTrips[i].fromLocation = wpo;
				restrictedSubTrip.tripID += "-sb";//subtrip before the current subtrip
				//insert restrictedSubTrip 'before' the current subtrip
				subTrips.insert(subTrips.begin() + i , restrictedSubTrip);
				i++;
			}
			//destination in the restricted area
			else
			{
				WayPoint wpd(subTrips[i].toLocation);
				subTrips[i].toLocation = wpd;
				restrictedSubTrip.fromLocation = wpd;
				restrictedSubTrip.tripID += "-sa";//split after the current subtrip
				//insert restrictedSubTrip 'after' the current subtrip
				subTrips.insert(subTrips.begin() + i + 1, restrictedSubTrip);
				i++;
			}
		}
	}

}

bool sim_mob::RestrictedRegion::isInRestrictedZone(const sim_mob::Node* target) const
{
	return Impl->isInRestrictedZone(target);
}

bool sim_mob::RestrictedRegion::isInRestrictedZone(const sim_mob::WayPoint& target) const
{
	if(target.type_ != WayPoint::NODE)
	{
		throw std::runtime_error ("Checking a Waypoint whose type is not Node");
	}
	return isInRestrictedZone(target.node_);
}


bool sim_mob::RestrictedRegion::isInRestrictedSegmentZone(const std::vector<WayPoint> & target) const
{
	BOOST_FOREACH(WayPoint wp, target)
	{
		if(wp.type_ != WayPoint::ROAD_SEGMENT)
		{
			throw std::runtime_error("Invalid WayPoint type Supplied\n");
		}
		if(Impl->isInRestrictedSegmentZone(wp.roadSegment_))
		{
			std::cout << wp.roadSegment_->getId() << " in the CBD zone\n";
			return true;
		}
	}
	return false;
}
bool sim_mob::RestrictedRegion::isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const
{
	return Impl->isInRestrictedSegmentZone(target);
}

bool sim_mob::RestrictedRegion::isEnteringRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg)
{
	return in.find(std::make_pair(curSeg,nxtSeg)) != in.end();
}

bool sim_mob::RestrictedRegion::isExittingRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg)
{
	return out.find(std::make_pair(curSeg,nxtSeg)) != out.end();
}

/*********  Search Implementaiont *****************/
sim_mob::RestrictedRegion::StrSearch::StrSearch(sim_mob::RestrictedRegion & instance):Search(instance){}
bool sim_mob::RestrictedRegion::StrSearch::isInRestrictedZone(const Node* target) const
{
	return
			(instance.zoneNodesStr.find(boost::lexical_cast<string>(target->getID())) != std::string::npos ?
					true :
					false);
}

bool sim_mob::RestrictedRegion::StrSearch::isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const
{
	return
			(instance.zoneSegmentsStr.find(boost::lexical_cast<string>(target->getId())) != std::string::npos ?
					true :
					false);

}
///////////////////
sim_mob::RestrictedRegion::ObjSearch::ObjSearch(sim_mob::RestrictedRegion & instance):Search(instance){}
bool sim_mob::RestrictedRegion::ObjSearch::isInRestrictedZone(const Node* target) const
{
	std::map<std::string,const Node*>::const_iterator it(instance.zoneNodes.find(boost::lexical_cast<std::string>(target->getID())));
	return (instance.zoneNodes.end() == it ? false : true);
}

bool sim_mob::RestrictedRegion::ObjSearch::isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const
{
	std::set<const sim_mob::RoadSegment*>::iterator itDbg;
	if ((itDbg = instance.zoneSegments.find(target)) != instance.zoneSegments.end())
	{
		std::cout << "segment " << (*itDbg)->getId() << " is in Zone Segments\n";
		return true;
	}
	return false;
}
///////////////////////
sim_mob::RestrictedRegion::TagSearch::TagSearch(sim_mob::RestrictedRegion & instance):Search(instance){}
bool sim_mob::RestrictedRegion::TagSearch::isInRestrictedZone(const Node* target) const
{
	return target->CBD;
}

bool sim_mob::RestrictedRegion::TagSearch::isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const
{
	return target->CBD;
}

///////////////////////
//const Node* sim_mob::RestrictedRegion::StrSearch::isInRestrictedZone(const WayPoint& target) const;
//bool sim_mob::RestrictedRegion::StrSearch::isInRestrictedSegmentZone(const std::vector<WayPoint> & target) const;
//bool sim_mob::RestrictedRegion::StrSearch::isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const;
//bool sim_mob::RestrictedRegion::StrSearch::isEnteringRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg);
//bool sim_mob::RestrictedRegion::StrSearch::isExittingRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg);





sim_mob::PeriodicPersonLoader::PeriodicPersonLoader(std::set<sim_mob::Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents)
	: activeAgents(activeAgents), pendingAgents(pendinAgents)
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	dataLoadInterval = SECONDS_IN_ONE_HOUR; //1 hour by default. TODO: must be configurable.
	elapsedTimeSinceLastLoad = cfg.baseGranSecond(); // initializing to base gran second so that all subsequent loads will happen 1 tick before the actual start of the interval

	nextLoadStart = getHalfHourWindow(cfg.system.simulation.simStartTime.getValue()/1000);

	storedProcName = cfg.getDatabaseProcMappings().procedureMappings["day_activity_schedule"];
}

sim_mob::PeriodicPersonLoader::~PeriodicPersonLoader()
{}

class CellLoader {
public:
	CellLoader():iStart(0),iEnd(0) {
	}

	void operator()(void) {
		id = boost::this_thread::get_id();
		for (int i = iStart; i < iEnd; i++) {
			vector<TripChainItem*>& item = trips[i];
			if (item.empty()) {
				continue;
			}
			ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
			Person* person = new Person("DAS_TripChain", cfg.mutexStategy(),
					item);
			if (!person->getTripChain().empty()) {
				persons.push_back(person);
			} else {
				delete person;
			}
		}
		Print()<<id<<" load "<<persons.size()<<std::endl;
	}

	static int Load(map<int, vector<TripChainItem*> >& trips,
			vector<Person*>& peoples) {
		int eachNum = trips.size() / LoaderNum;
		CellLoader workers[LoaderNum];
		boost::thread_group threads;
		for (int i = 0; i < LoaderNum; i++) {
			workers[i].trips = trips;
			workers[i].iStart = i * eachNum;
			if (i != LoaderNum - 1) {
				workers[i].iEnd = (i + 1) * eachNum;
			} else {
				workers[i].iEnd = trips.size();
			}
			threads.add_thread(new boost::thread(boost::ref(workers[i])));
		}
		threads.join_all();
		unsigned personsNum = 0;
		for (int i = 0; i < LoaderNum; i++) {
			peoples.insert(peoples.end(), workers[i].persons.begin(), workers[i].persons.end());
		}
		personsNum = peoples.size();
		return personsNum;
	}
private:
	vector<Person*> persons;
	map<int, vector<TripChainItem*> > trips;
	int iStart, iEnd;
	static const int LoaderNum = 20;
	boost::thread::id id;
};

void sim_mob::PeriodicPersonLoader::loadActivitySchedules()
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
	map<int, vector<TripChainItem*> > trips;
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
		trips[actCtr]=personTripChain;
		actCtr++;
	}

	vector<Person*> persons;
	int personsLoaded = CellLoader::Load(trips, persons);
	for(vector<Person*>::iterator i=persons.begin(); i!=persons.end(); i++){
		addOrStashPerson(*i);
	}
	//CBD specific processing of trip chain
	//RestrictedRegion::getInstance().processTripChains(tripchains);//todo, plan changed, we are not chopping off the trips here

	//add or stash new persons
	/*unsigned personsLoaded = 0;
	for(map<string, vector<TripChainItem*> >::iterator i=tripchains.begin(); i!=tripchains.end(); i++)
	{
		if(i->second.empty()) { continue; }
		Person* person = new Person("DAS_TripChain", cfg.mutexStategy(), i->second);
		if(!person->getTripChain().empty()) { addOrStashPerson(person); personsLoaded++; }
		else { delete person; }
	}*/

	Print() << "PeriodicPersonLoader:: activities loaded from " << nextLoadStart << " to " << end << ": " << actCtr
			<< " | new persons loaded: " << personsLoaded << endl;

	Print() << "active_agents: " << activeAgents.size() << " | pending_agents: " << pendingAgents.size() << endl;
	//update next load start
	nextLoadStart = end + DEFAULT_LOAD_INTERVAL;
	if(nextLoadStart > LAST_30MIN_WINDOW_OF_DAY)
	{
		nextLoadStart = nextLoadStart - TWENTY_FOUR_HOURS; //next day starts at 3.25
	}
}

void sim_mob::PeriodicPersonLoader::addOrStashPerson(Person* p)
{
	//Only agents with a start time of zero should start immediately in the all_agents list.
	if (p->getStartTime()==0)
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


void sim_mob::PeriodicPersonLoader::makeSubTrip(const soci::row& r, sim_mob::Trip* parentTrip, unsigned short subTripNo)
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

sim_mob::Activity* sim_mob::PeriodicPersonLoader::makeActivity(const soci::row& r, unsigned int seqNo)
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

sim_mob::Trip* sim_mob::PeriodicPersonLoader::makeTrip(const soci::row& r, unsigned int seqNo)
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
	tripToSave->startTime = sim_mob::DailyTime(getRandomTimeInWindow(r.get<double>(11), false, tripToSave->getPersonID()));
	//just a sanity check
	if(tripToSave->fromLocation == tripToSave->toLocation)
	{
		safe_delete_item(tripToSave);
		return nullptr;
	}
	makeSubTrip(r, tripToSave);
	return tripToSave;
}
