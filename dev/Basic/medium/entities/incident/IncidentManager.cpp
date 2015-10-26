#include "IncidentManager.hpp"


#include "entities/conflux/Conflux.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/Person_MT.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "config/MT_Config.hpp"
#include "path/PathSetManager.hpp"
#include "message/MessageBus.hpp"
#include "conf/ConfigManager.hpp"
#include <boost/tokenizer.hpp>

using namespace sim_mob;
using namespace sim_mob::medium;

namespace{
//BasicLogger & logger = Logger::log("pathset.log");
}

namespace {
//todo put this in the utils(and code style!)
boost::mt19937 myOwngen;

int roll_dice(int l,int r) {
    boost::uniform_int<> dist(l,r);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > dice(myOwngen, dist);
    return dice();
}
}

IncidentManager * IncidentManager::instance = 0;
std::map<const RoadSegment*, double> IncidentManager::currIncidents = std::map<const RoadSegment*, double>();
//Logger IncidentManager::profiler;
IncidentManager::IncidentManager(const std::string inputFile) :
		Agent(ConfigManager::GetInstance().FullConfig().mutexStategy()),inputFile(inputFile)/*,distribution(Utils::initDistribution(std::pair<float,float>(0.0, 1.0)))*/
{
	readFromFile("private/incidents.csv");
}

void IncidentManager::setSourceFile(const std::string inputFile_){
	inputFile = inputFile_;
}

/**
 * read the incidents from a csv file, each line having format sectionId,newFlowRate,tick
 */
void IncidentManager::readFromFile(std::string inputFile){
	std::ifstream in(inputFile.c_str());
	if (!in.is_open()){
//		std::ostringstream out("");
//		out << "Incident File " << inputFile << " not found";
//		throw std::runtime_error(out.str());
		return;
	}
	typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
	std::string line;
	std::vector< std::string > vec;
	while (getline(in,line))
	{
		Tokenizer record(line);
		vec.clear();
		vec.assign(record.begin(),record.end());
		unsigned int sectionId = boost::lexical_cast<unsigned int>(vec[0]);//first element
		double newFlowRate = boost::lexical_cast<double>(vec[1]);//second element
		uint32_t tick =  boost::lexical_cast<uint32_t>(vec[2]);//second element
		incidents.insert(std::make_pair(tick,Incident(sectionId,newFlowRate,tick)));
		//logger << "Incident inserted for tick:" <<tick << " sectionId:" << sectionId << "\n" ;
	}
	in.close();
}

void IncidentManager::insertTickIncidents(uint32_t tick){
	//find the incidents in this tick
	TickIncidents tickIncident = incidents.equal_range(tick);
	if(tickIncident.first == tickIncident.second){
		//no incidents for this tick
		return;
	}

	StreetDirectory & stDir = StreetDirectory::instance();
	std::pair<uint32_t,Incident> incident;
	//inserting and informing: 1-Conflux 2-pathsetmanager 3-person
	for(std::multimap<uint32_t,Incident>::iterator incident = tickIncident.first; incident != tickIncident.second; incident++){
		//get the conflux
		const RoadSegment* rs = stDir.getRoadSegment(incident->second.get<0>());
		Conflux* incidentConflux = Conflux::getConflux(rs);
		if(!incidentConflux)
		{
			throw std::runtime_error("No conflux found for segment");
		}
		//send a message to conflux to insert an incident to itseld
		messaging::MessageBus::PostMessage(incidentConflux, MSG_INSERT_INCIDENT,
							messaging::MessageBus::MessagePtr(new InsertIncidentMessage(rs, incident->second.get<1>())));
		PrivateTrafficRouteChoice::getInstance()->insertIncidentList(rs);
		std::vector <const Person_MT*> persons;
		identifyAffectedDrivers(rs,persons);
		//logger << " INCIDENT  segment:"<< rs->getSegmentAimsunId() << " affected:" << persons.size() << "\n" ;

		//find affected Drivers (only active agents for now)
		//inform the drivers about the incident
		BOOST_FOREACH(const Person_MT* person, persons) {
			//send the same type of message
			messaging::MessageBus::PostMessage(const_cast<Person_MT*>(person), MSG_INSERT_INCIDENT,
								messaging::MessageBus::MessagePtr(new InsertIncidentMessage(rs, incident->second.get<1>())));
		}

		//and finally, you have an incident
		currIncidents[rs] = incident->second.get<1>();
	}
	*/
}

std::map<const RoadSegment*, double> & IncidentManager::getCurrIncidents(){
	return currIncidents;
}

//step-1: find those who used the target rs in their path
//step-2: for each person, iterate through the path(meso path for now) to see if the agent's current segment is before, on or after the target path.
//step-3: if agent's current segment is before the target path, then he can be informed(if the probability function allows that).
//TODO: this method should be changed/scrabbed after this file was moved from medium to shared folder. const DriverMovement is not functional here!!!-vahid
void IncidentManager::identifyAffectedDrivers(const RoadSegment * targetRS, std::vector <const Person_MT*> & filteredPersons)
{
//	int affected = 0;
//	int ignored = 0;
//	int reacting = 0;
//	int ignorant = 0;
//	//step-1: find those who used the target rs in their path
//	const std::pair <SGPER::const_iterator,SGPER::const_iterator > range(PathSetManager::getInstance()->getODbySegment(targetRS));
//	for(SGPER::const_iterator it(range.first); it != range.second; it++){
//		const Person *per = it->second;
//		//TODO: this is currently not working-vahid
//		const DriverMovement *dm = dynamic_cast<DriverMovement*>(per->getRole()->Movement());
//		const std::vector<const SegmentStats*> path = dm->getMesoPathMover().getPath();
//		const SegmentStats* curSS = dm->getMesoPathMover().getCurrSegStats();
//		//In the following steps, we try to select only those who are before the current segment
//		//todo, increase the criteria , add some min distance restriction
//		std::vector<const SegmentStats*>::const_iterator itSS;//segStat iterator
//		//a.is the incident before driver's current segment?
//		bool res = false;
//		for(itSS = path.begin(); (*itSS) != curSS ; itSS++){
//			if(targetRS == (*itSS)->getRoadSegment()){
//				res = true;
////				logger << "This incident is happening on a segments 'before' this driver" << std::endl;
//				break;
//			}
//		}
//		//Same check for case (*itSS) == curSS i.e you are currently 'on' the incident segment
//		if(itSS != path.end() && (targetRS == (*itSS)->getRoadSegment())){
////			logger << "This incident is happening on the driver's current segment" << std::endl;
//			res = true;
//		}
//
//		if(res){
//			ignored++;
////			logger << "ignoring this driver" <<std::endl;
//			//person passed, or currently on the target path. So, not interested in this person
//			continue;
//		}
//		//b.So the incident is 'after' driver's current segment. still, Check it.
//		for(; itSS != path.end() ; itSS++){
//			if(targetRS == (*itSS)->getRoadSegment()){
//				res = true;
//				break;
//			}
//		}
//		if(!res){
//			//can't be! this means we have been searching for a target road segment that is not in the path!
//			throw std::runtime_error("searching for a roadsegment which was not in the path!");
//		}
//		affected ++;
//		//should react to incident or not?
//		if(shouldDriverReact(per)){
//			filteredPersons.push_back(per);
//			reacting ++;
//		}
//		else{
//			ignorant ++;
//		}
//	}//SGPER
//	logger << "Number of Affected Driver's Paths: " << affected << "\n";
//	logger << "Number of Affected Drivers: " << affected - ignored << "\n";
//	logger << "Number of Drivers Reacting to Incident: " << reacting << "\n";
//	logger << "Number of Drivers Ignoring the incident : " << ignorant << "\n";
}

//probability function(for now, just behave like tossing a coin
void IncidentManager::findReactingDrivers(std::vector<const Person_MT*> & res) {
	std::vector<const Person_MT*>::iterator it(res.begin());
	for ( ; it != res.end(); ) {
	  if (roll_dice(0,1)) {
	    it = res.erase(it);
	  } else {
	    ++it;
	  }
	}
}

//probability function(for now, just behave like tossing a coin
bool IncidentManager::shouldDriverReact(const Person_MT* per){
	return roll_dice(0,1);
}

bool IncidentManager::frame_init(timeslice now){
	return true;
}

Entity::UpdateStatus IncidentManager::frame_tick(timeslice now){
	insertTickIncidents(now.frame());
	return UpdateStatus::Continue;
}

bool IncidentManager::isNonspatial(){
	return true;
}


IncidentManager * IncidentManager::getInstance()
{
//	static IncidentManager instance;
	if(!instance){
		instance = new IncidentManager();
	}
	return instance;
}
