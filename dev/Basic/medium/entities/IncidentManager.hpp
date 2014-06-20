#pragma once
#include <sstream>
#include <vector>
#include <stdint.h>

#include "message/Message.hpp"
#include "entities/Agent.hpp"
#include "util/Profiler.hpp"

#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

namespace sim_mob {
//
//struct Distribution
//{
//		//normal distribution
//	boost::mt19937 rng; // I don't seed it on purpouse (it's not relevant)
//	boost::normal_distribution<> nd(0.0, 1.0);
//	boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > gen(rng, nd);
//};
/**
 * A class designed to manage the incidents.
 * It can read the load the incidents for preday/test planning as well as en-route incidents
 */
class IncidentManager : public sim_mob::Agent {
	typedef boost::tuples::tuple<unsigned int, double, uint32_t> Incident; //<sectionId, flowrate, start_tick>
	std::multimap<uint32_t,Incident> incidents;
	typedef std::pair<std::multimap<uint32_t,Incident>::iterator, std::multimap<uint32_t,Incident>::iterator> TickIncidents;
	std::string inputFile;
	boost::shared_ptr<boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > > distribution;
	static IncidentManager * instance;
public:
	//debug
	static sim_mob::Profiler profiler;
	IncidentManager(const std::string inputFile = "");
	/**
	 * Set the source file name
	 * \param inputFile_ the file containing the incident information
	 */
	void setSourceFile(const std::string inputFile_);

	/**
	 * read the incidents from a file
	 */
	void readFromFile(std::string inputFile);
	/**
	 * insert incidents into simulation starting in a given tick.
	 * This can be done via defining a new flow rate to the lanes of the given segment.
	 * since it using the message bus, it will be available in the next tick
	 * \param tick starting tick
	 */
	void insertTickIncidents(uint32_t tick);

	/**
	 *	step-1: find those who used the target rs in their path
	 *	step-2: for each person, iterate through the path(meso path for now) to see if the agent's current segment is before, on or after the target path.
	 *	step-3: if agent's current segment is before the target path, then inform him.
	 * \param in targetRS the target road segment that is having an incident
	 * \param out filteredPersons persons to be notified of the incident in the target roadsegment
	 */
	void identifyAffectedDrivers(const sim_mob::RoadSegment * targetRS,std::vector <const sim_mob::Person*> & filteredPersons);
	/**
	 * given a list of drivers who are affected by incident,
	 * use a distribution to keep those who will react to the incident and filter out the rest
	 * let's for simplicity, just toss a coin to find out who will react and who won't.
	 * \param persons ,inout, candidate drivers
	 */
	void findReactingDrivers(std::vector <const sim_mob::Person*> & persons);
	/**
	 * given a driver who is affected by incident,
	 * use a distribution to see if the driver should react to incident or not.
	 * let's for simplicity, just toss a coin to find out who will react and who won't.
	 * \param person ,inout, candidate driver
	 */
	bool shouldDriverReact(const sim_mob::Person* person);

	bool frame_init(timeslice now);

	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now){}
	bool isNonspatial();
	void load(const std::map<std::string, std::string>& configProps){}
	static sim_mob::IncidentManager * getInstance();
};

}
