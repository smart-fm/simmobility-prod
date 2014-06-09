#pragma once
#include <sstream>
#include <vector>
#include <stdint.h>

#include <entities/conflux/Conflux.hpp>
#include "geospatial/streetdir/StreetDirectory.hpp"

#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

namespace sim_mob {

/**
 * Subclasses message, This is to allow it to function as an message callback parameter.
 */
class InsertIncident : public messaging::Message {
public:
	InsertIncident(const std::vector<sim_mob::SegmentStats*>& stats, double newFlowRate):stats(stats), newFlowRate(newFlowRate){;}
	virtual ~InsertIncident() {}
	const std::vector<sim_mob::SegmentStats*>& stats;
	double newFlowRate;
};

/**
 * A class designed to manage the incidents.
 * It can read the load the incidents for preday/test planning as well as en-route incidents
 */
class IncidentManager : sim_mob::Agent {
	typedef boost::tuples::tuple<unsigned int, double, uint32_t> Incident; //<sectionId, flowrate, start_tick>
	std::multimap<uint32_t,Incident> incidents;
	typedef std::pair<std::multimap<uint32_t,Incident>::iterator, std::multimap<uint32_t,Incident>::iterator> TickIncidents;
	std::string fileName;
public:
	/**
	 * Set the source file name
	 * \param fileName_ the file containing the incident information
	 */
	void setFileSource(const std::string fileName_){
		fileName = fileName_;
	}

	/**
	 * read the incidents from a file
	 */
	void readFromFile(std::string fileName){
		std::ifstream in(fileName.c_str());
		if (!in.is_open()){
			std::ostringstream out("");
			out << "File " << fileName << " not found";
			throw std::runtime_error(out.str());
			//return false;
		}
		sim_mob::StreetDirectory & stDir = sim_mob::StreetDirectory::instance();
		typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
		std::string line;

		while (getline(in,line))
		{
			Tokenizer record(line);
			Tokenizer::iterator it = record.begin();
			unsigned int sectionId = boost::lexical_cast<unsigned int>(*(it));//first element
			double newFlowRate = boost::lexical_cast<double>(*(++it));//second element
			uint32_t tick =  boost::lexical_cast<uint32_t>(*(++it));//second element
			incidents.insert(std::make_pair(tick,Incident(sectionId,newFlowRate,tick)));
		}
	}
	/**
	 * insert incidents into a conflux starting in a given tick
	 * since it using the message bus, it will be available in the next tick
	 * \param tick starting tick
	 */
	void insertTickIncidents(uint32_t tick){
		TickIncidents res = incidents.equal_range(tick);
		if(res.first == res.second){
			//no incidents for this tick
			return;
		}
		sim_mob::StreetDirectory & stDir = sim_mob::StreetDirectory::instance();
		std::pair<uint32_t,Incident> element;
		BOOST_FOREACH(element, res){
			//get the conflux
			const sim_mob::RoadSegment* rs = stDir.getRoadSegment(element.second.get<0>());
			const std::vector<sim_mob::SegmentStats*>& stats = rs->getParentConflux()->findSegStats(rs);
			//send a message to conflux to change its flow rate for the next tick
			messaging::MessageBus::PostMessage(rs->getParentConflux(), MSG_INSERT_INCIDENT,
								messaging::MessageBus::MessagePtr(new InsertIncident(stats, element.second.get<1>())));
			//Identify the to-be-affected Drivers (at present, only the active agents are considered)
			//contact the path set manager(via your own method). he should already have the paths in its cache
//			identifyAffectedDrivers();
		}
	}



	bool frame_init(timeslice now){
		if(fileName.size()){
			readFromFile(fileName);
		}
	}

	virtual Entity::UpdateStatus frame_tick(timeslice now){
		insertTickIncidents(now.frame());
	}




};

}
