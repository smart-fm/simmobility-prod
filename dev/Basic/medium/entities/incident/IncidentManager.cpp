#include "IncidentManager.hpp"


//#include <entities/roles/Role.hpp>
//#include <entities/roles/RoleFacets.hpp>
#include <entities/roles/driver/DriverFacets.hpp>
#include <entities/conflux/Conflux.hpp>
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/PathSetManager.hpp"
#include "message/MessageBus.hpp"
#include "conf/ConfigManager.hpp"
//may needed when you move definition to cpp
//#include <boost/random.hpp>
//#include <boost/random/normal_distribution.hpp>
#include <boost/tokenizer.hpp>

sim_mob::IncidentManager * sim_mob::IncidentManager::instance = 0;
std::map<const sim_mob::RoadSegment*, double> sim_mob::IncidentManager::currIncidents = std::map<const sim_mob::RoadSegment*, double>();
//sim_mob::Logger sim_mob::IncidentManager::profiler;
sim_mob::IncidentManager::IncidentManager(const std::string inputFile) :
		Agent(ConfigManager::GetInstance().FullConfig().mutexStategy()),inputFile(inputFile)/*,distribution(Utils::initDistribution(std::pair<float,float>(0.0, 1.0)))*/
{}

void sim_mob::IncidentManager::setSourceFile(const std::string inputFile_){
	inputFile = inputFile_;
}

/**
 * read the incidents from a csv file, each line having format sectionId,newFlowRate,tick
 */
void sim_mob::IncidentManager::readFromFile(std::string inputFile){
	std::ifstream in(inputFile.c_str());
	if (!in.is_open()){
		std::ostringstream out("");
		out << "Incident File " << inputFile << " not found";
//		throw std::runtime_error(out.str());
		//debug
		return;
	}
	sim_mob::StreetDirectory & stDir = sim_mob::StreetDirectory::instance();
	typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
	std::string line;
	std::vector< std::string > vec;
	while (getline(in,line))
	{
		Tokenizer record(line);
		Tokenizer::iterator it = record.begin();
		vec.clear();
		vec.assign(record.begin(),record.end());
		unsigned int sectionId = boost::lexical_cast<unsigned int>(vec[0]);//first element
		double newFlowRate = boost::lexical_cast<double>(vec[1]);//second element
		uint32_t tick =  boost::lexical_cast<uint32_t>(vec[2]);//second element
		incidents.insert(std::make_pair(tick,Incident(sectionId,newFlowRate,tick)));
		sim_mob::Logger::log["path_set"] << "Incident inserted for tick:" <<tick << " sectionId:" << sectionId << sim_mob::BasicLogger::newLine;
	}
	in.close();
}

void sim_mob::IncidentManager::insertTickIncidents(uint32_t tick){
	/*find the incidents in this tick*/
	TickIncidents tickIncident = incidents.equal_range(tick);
	if(tickIncident.first == tickIncident.second){
		//no incidents for this tick
		return;
	}

	sim_mob::StreetDirectory & stDir = sim_mob::StreetDirectory::instance();
	std::pair<uint32_t,Incident> incident;
	/*inserting and informing: 1-Conflux 2-pathsetmanager 3-person*/
	for(std::multimap<uint32_t,Incident>::iterator incident = tickIncident.first; incident != tickIncident.second; incident++){
		//get the conflux
		const sim_mob::RoadSegment* rs = stDir.getRoadSegment(incident->second.get<0>());

		const std::vector<sim_mob::SegmentStats*>& stats = rs->getParentConflux()->findSegStats(rs);
		//send a message to conflux to insert an incident to itseld
		messaging::MessageBus::PostMessage(rs->getParentConflux(), MSG_INSERT_INCIDENT,
							messaging::MessageBus::MessagePtr(new InsertIncidentMessage(stats, incident->second.get<1>())));
		sim_mob::PathSetManager::getInstance()->inserIncidentList((*stats.begin())->getRoadSegment());
		std::vector <const sim_mob::Person*> persons;
		identifyAffectedDrivers(rs,persons);
		sim_mob::Logger::log["path_set"] << " INCIDENT  segment:"<< rs->getSegmentAimsunId() << " affected:" << persons.size() << sim_mob::BasicLogger::newLine;
//		/**
//		 * DEBUG
//		 */
//		return;//dont inform any driver, let them go into incident section
//		/**
//		 * DEBUG...
//		 */
		//find affected Drivers (only active agents for now)
		//inform the drivers about the incident
		BOOST_FOREACH(const sim_mob::Person * person, persons) {
			//send the same type of message
			messaging::MessageBus::PostMessage(const_cast<sim_mob::MovementFacet*>(person->getRole()->Movement()), MSG_INSERT_INCIDENT,
								messaging::MessageBus::MessagePtr(new InsertIncidentMessage(stats, incident->second.get<1>())));
		}

		//and finally, you have an incident
		currIncidents[rs] = incident->second.get<1>();
	}
}

std::map<const sim_mob::RoadSegment*, double> & sim_mob::IncidentManager::getCurrIncidents(){
	return currIncidents;
}

//step-1: find those who used the target rs in their path
//step-2: for each person, iterate through the path(meso path for now) to see if the agent's current segment is before, on or after the target path.
//step-3: if agent's current segment is before the target path, then he can be informed(if the probability function allows that).
void sim_mob::IncidentManager::identifyAffectedDrivers(const sim_mob::RoadSegment * targetRS,
		std::vector <const sim_mob::Person*> & filteredPersons){
	int affected = 0;
	int ignored = 0;
	int reacting = 0;
	int ignorant = 0;
	//step-1: find those who used the target rs in their path
	const std::pair <RPOD::const_iterator,RPOD::const_iterator > range(sim_mob::PathSetManager::getInstance()->getODbySegment(targetRS));
	for(RPOD::const_iterator it(range.first); it != range.second; it++){
		const sim_mob::Person *per = it->second.per;
		//get his,meso, path...//todo: you need to dynamic_cast!
		const sim_mob::medium::DriverMovement *dm = dynamic_cast<sim_mob::medium::DriverMovement*>(per->getRole()->Movement());
		const std::vector<const sim_mob::SegmentStats*> path = dm->getMesoPathMover().getPath();
		const sim_mob::SegmentStats* curSS = dm->getMesoPathMover().getCurrSegStats();
		//In the following steps, we try to select only those who are before the current segment
		//todo, increase the criteria , add some min distance restriction
		std::vector<const sim_mob::SegmentStats*>::const_iterator itSS;//segStat iterator
		//a.is the incident before driver's current segment?
		bool res = false;
		for(itSS = path.begin(); (*itSS) != curSS ; itSS++){
			if(targetRS == (*itSS)->getRoadSegment()){
				res = true;
//				sim_mob::Logger::log["path_set"] << "This incident is happening on a segments 'before' this driver" << std::endl;
				break;
			}
		}
		//Same check for case (*itSS) == curSS i.e you are currently 'on' the incident segment
		if(itSS != path.end() && (targetRS == (*itSS)->getRoadSegment())){
//			sim_mob::Logger::log["path_set"] << "This incident is happening on the driver's current segment" << std::endl;
			res = true;
		}

		if(res){
			ignored++;
//			sim_mob::Logger::log["path_set"] << "ignoring this driver" <<std::endl;
			//person passed, or currently on the target path. So, not interested in this person
			continue;
		}
		//b.So the incident is 'after' driver's current segment. still, Check it.
		for(; itSS != path.end() ; itSS++){
			if(targetRS == (*itSS)->getRoadSegment()){
				res = true;
				break;
			}
		}
		if(!res){
			//can't be! this means we have been searching for a target road segment that is not in the path!
			throw std::runtime_error("searching for a roadsegment which was not in the path!");
		}
		affected ++;
		//should react to incident or not?
		if(shouldDriverReact(per)){
			filteredPersons.push_back(per);
			reacting ++;
		}
		else{
			ignorant ++;
		}
	}//RPOD
	sim_mob::Logger::log["path_set"] << "Number of Affected Driver's Paths: " << affected << std::endl;
	sim_mob::Logger::log["path_set"] << "Number of Affected Drivers: " << affected - ignored << std::endl;
	sim_mob::Logger::log["path_set"] << "Number of Drivers Reacting to Incident: " << reacting << std::endl;
	sim_mob::Logger::log["path_set"] << "Number of Drivers Ignoring the incident : " << ignorant << std::endl;
}

//probability function(for now, just behave like tossing a coin
void sim_mob::IncidentManager::findReactingDrivers(std::vector<const sim_mob::Person*> & res) {
	std::vector<const sim_mob::Person*>::iterator it(res.begin());
	for ( ; it != res.end(); ) {
	  if (roll_dice(0,1)) {
	    it = res.erase(it);
	  } else {
	    ++it;
	  }
	}
}

//probability function(for now, just behave like tossing a coin
bool sim_mob::IncidentManager::shouldDriverReact(const sim_mob::Person* per){
	return roll_dice(0,1);
}

bool sim_mob::IncidentManager::frame_init(timeslice now){
	return true;
}

sim_mob::Entity::UpdateStatus sim_mob::IncidentManager::frame_tick(timeslice now){
	insertTickIncidents(now.frame());
	return UpdateStatus::Continue;
}

bool sim_mob::IncidentManager::isNonspatial(){
	return true;
}


sim_mob::IncidentManager * sim_mob::IncidentManager::getInstance()
{
//	static IncidentManager instance;
	if(!instance){
		instance = new sim_mob::IncidentManager();
	}
	return instance;
}
