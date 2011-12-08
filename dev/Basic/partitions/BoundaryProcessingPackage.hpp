/*
 * BoundaryProcessingPackage.hpp
 * Target:
 * 1. Each time, one computer should package data in boundary and transmit package to other computers.
 * 2. Each package can contain many road segment conditions.
 * 3. This Package Implement serialization
 */

#pragma once

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "partitions/RoadNetworkPackageManager.hpp"
#include "partitions/AgentPackageManager.hpp"

namespace sim_mob {

class OneRoadSegmentAgentList;

class BoundaryProcessingPackage {

public:
	std::vector<Agent const*> cross_agents;
	std::vector<Agent const*> feedback_agents;
	std::vector<Signal const*> boundary_signals;

public:
	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	BOOST_SERIALIZATION_SPLIT_MEMBER()
};

}

template<class Archive>
void sim_mob::BoundaryProcessingPackage::save(Archive & ar, const unsigned int version) const {
	sim_mob::AgentPackageManager& packageImpl = sim_mob::AgentPackageManager::instance();
	int cross_size = cross_agents.size();
	ar & cross_size;
	//std::cout << "packageOneCrossDriver 1:" << std::endl;

	std::vector<Agent const*>::const_iterator cross_it = cross_agents.begin();
	for (; cross_it != cross_agents.end(); cross_it++) {
		role_modes type = packageImpl.getAgentRoleType(*cross_it);
		ar & type;

		if (type == role_modes(Driver_Role) || type == role_modes(Pedestrian_Role))
			packageImpl.packageOneCrossAgent(ar, (*cross_it));
	}

	//std::cout << "packageOneCrossDriver 2:" << std::endl;
	int feed_size = feedback_agents.size();
	ar & feed_size;

	std::vector<Agent const*>::const_iterator feed_it = feedback_agents.begin();
	for (; feed_it != feedback_agents.end(); feed_it++) {
		int type = packageImpl.getAgentRoleType(*feed_it);
		ar & type;

		//std::cout << "packageOneCrossDriver 2.1:" << std::endl;
		if (type == role_modes(Driver_Role) || type == role_modes(Pedestrian_Role))
			packageImpl.packageOneFeedbackAgent(ar, (*feed_it));
	}

	//std::cout << "packageOneCrossDriver 12:" << std::endl;
	int signal_size = boundary_signals.size();
	ar & signal_size;

	sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();

	for (int i = 0; i < signal_size; i++) {
		rnpackageImpl.packageSignalContent(ar, boundary_signals[i]);
	}

	//std::cout << "packageOneCrossDriver 13:" << std::endl;
}

template<class Archive>
void sim_mob::BoundaryProcessingPackage::load(Archive & ar, const unsigned int version) {
	sim_mob::AgentPackageManager& packageImpl = sim_mob::AgentPackageManager::instance();

	int cross_size = 0;
	ar & cross_size;

	//std::cout << "unpackageOneCrossDriver 2:" << std::endl;
	for (int i = 0; i < cross_size; i++) {
		role_modes type;
		ar & type;

		if (type == role_modes(Driver_Role) || type == role_modes(Pedestrian_Role))
		//if (type == role_modes(Driver_Role))
		{
			Agent* one = packageImpl.rebuildOneCrossAgent(ar, type);
			cross_agents.push_back(one);
		}
	}

	//std::cout << "unpackageOneCrossDriver 3:" << std::endl;
	int feed_size = 0;
	ar & feed_size;

	for (int i = 0; i < feed_size; i++) {
		role_modes type;
		ar & type;

		if (type == role_modes(Driver_Role) || type == role_modes(Pedestrian_Role))
		//if (type == role_modes(Driver_Role))
		{
			Agent * one = packageImpl.rebuildOneFeedbackAgent(ar, type);
			feedback_agents.push_back(one);
		}
	}
	//std::cout << "unpackageOneCrossDriver 4:" << std::endl;

	int signal_size = 0;
	ar & signal_size;

	sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();

	for (int i = 0; i < signal_size; i++) {
		rnpackageImpl.unpackageSignalContent(ar);
	}
	//std::cout << "packageOneCrossDriver 5:" << std::endl;
}

