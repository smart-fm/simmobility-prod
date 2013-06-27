//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "conf/settings/DisableMPI.h"

#include "util/LangHelpers.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "entities/Agent.hpp"
#include "entities/signal/Signal.hpp"
#include "entities/Person.hpp"

#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"

#include "geospatial/Node.hpp"
#include "entities/misc/TripChain.hpp"
#include "partitions/ParitionDebugOutput.hpp"

/*
 * \author Xu Yan
 */

namespace sim_mob {

/**
 * Serialization of Class Agent
 */
void sim_mob::Agent::pack(PackageUtils& packageUtil)
{
	//std::cout << "Agent package Called" <<this->getId()<< std::endl;

	packageUtil<<(id);
//	packageUtil<<(isSubscriptionListBuilt);
	packageUtil<<(startTime);

	sim_mob::Node::pack(packageUtil, originNode);
	sim_mob::Node::pack(packageUtil, destNode);

	packageUtil<<(xPos.get());
	packageUtil<<(yPos.get());
	packageUtil<<(fwdVel.get());
	packageUtil<<(latVel.get());
	packageUtil<<(xAcc.get());
	packageUtil<<(yAcc.get());

	packageUtil<<(toRemoved);
	packageUtil<<(dynamic_seed);
}

void sim_mob::Agent::unpack(UnPackageUtils& unpackageUtil) {

	unpackageUtil >> id;
//	id = unpackageUtil.unpackBasicData<int> ();
	//std::cout << "Agent unpackage Called:" <<this->getId() << std::endl;
	//isSubscriptionListBuilt = unpackageUtil.unpackBasicData<bool> ();
//	startTime = unpackageUtil.unpackBasicData<int> ();
	unpackageUtil >> startTime;

	originNode = Node::unpack(unpackageUtil);
	destNode = Node::unpack(unpackageUtil);

	int x_pos, y_pos;
	double x_acc, y_acc;
	double x_vel, y_vel;

	unpackageUtil >> x_pos;
	unpackageUtil >> y_pos;
	unpackageUtil >> x_acc;
	unpackageUtil >> y_acc;
	unpackageUtil >> x_vel;
	unpackageUtil >> y_vel;

//	x_pos = unpackageUtil.unpackBasicData<int> ();
//	y_pos = unpackageUtil.unpackBasicData<int> ();
//	x_acc = unpackageUtil.unpackBasicData<double> ();
//	y_acc = unpackageUtil.unpackBasicData<double> ();
//	x_vel = unpackageUtil.unpackBasicData<double> ();
//	y_vel = unpackageUtil.unpackBasicData<double> ();

	xPos.force(x_pos);
	yPos.force(y_pos);
	xAcc.force(x_acc);
	yAcc.force(y_acc);
	fwdVel.force(x_vel);
	latVel.force(y_vel);

	unpackageUtil >> toRemoved;
	unpackageUtil >> dynamic_seed;

//	toRemoved = unpackageUtil.unpackBasicData<bool> ();
//	dynamic_seed = unpackageUtil.unpackBasicData<int> ();
}

void sim_mob::Agent::packProxy(PackageUtils& packageUtil)
{
	packageUtil<<(id);
	//packageUtil<<(isSubscriptionListBuilt);
	packageUtil<<(startTime);

	packageUtil<<(xPos.get());
	packageUtil<<(yPos.get());
	packageUtil<<(fwdVel.get());
	packageUtil<<(latVel.get());
	packageUtil<<(xAcc.get());
	packageUtil<<(yAcc.get());

	packageUtil<<(toRemoved);
	packageUtil<<(dynamic_seed);
}

void sim_mob::Agent::unpackProxy(UnPackageUtils& unpackageUtil) {
//	id = unpackageUtil.unpackBasicData<int> ();
	unpackageUtil >> id;
	//isSubscriptionListBuilt = unpackageUtil.unpackBasicData<bool> ();
//	startTime = unpackageUtil.unpackBasicData<int> ();
	unpackageUtil >> startTime;

	int x_pos, y_pos;
	double x_acc, y_acc;
	double x_vel, y_vel;

	unpackageUtil >> x_pos;
	unpackageUtil >> y_pos;
	unpackageUtil >> x_acc;
	unpackageUtil >> y_acc;
	unpackageUtil >> x_vel;
	unpackageUtil >> y_vel;

//	x_pos = unpackageUtil.unpackBasicData<int> ();
//	y_pos = unpackageUtil.unpackBasicData<int> ();
//	x_acc = unpackageUtil.unpackBasicData<double> ();
//	y_acc = unpackageUtil.unpackBasicData<double> ();
//	x_vel = unpackageUtil.unpackBasicData<double> ();
//	y_vel = unpackageUtil.unpackBasicData<double> ();

	xPos.force(x_pos);
	yPos.force(y_pos);
	xAcc.force(x_acc);
	yAcc.force(y_acc);
	fwdVel.force(x_vel);
	latVel.force(y_vel);

	unpackageUtil >> toRemoved;
	unpackageUtil >> dynamic_seed;

//	toRemoved = unpackageUtil.unpackBasicData<bool> ();
//	dynamic_seed = unpackageUtil.unpackBasicData<int> ();
}

/**
 * Serialization of Class Person
 */
void sim_mob::Person::pack(PackageUtils& packageUtil) {
	//package Entity
	//std::cout << "Person package Called" << this->getId() << std::endl;
	sim_mob::Agent::pack(packageUtil);

	//package person
	packageUtil<<(specialStr);
//	sim_mob::TripChain::pack(packageUtil, currTripChain);
//	packageUtil<<(firstFrameTick);
}

void sim_mob::Person::unpack(UnPackageUtils& unpackageUtil) {

	sim_mob::Agent::unpack(unpackageUtil);
	//std::cout << "Person unpackage Called" << this->getId() << std::endl;

	unpackageUtil >> specialStr;
//	specialStr = unpackageUtil.unpackBasicData<std::string> ();
//	currTripChain = sim_mob::TripChain::unpack(unpackageUtil);

//	unpackageUtil >> firstFrameTick;
//	firstFrameTick = unpackageUtil.unpackBasicData<bool> ();
}

void sim_mob::Person::packProxy(PackageUtils& packageUtil) {
	//package Entity
	sim_mob::Agent::packProxy(packageUtil);

	//package person
	packageUtil<<(specialStr);
//	packageUtil<<(firstFrameTick);
}

void sim_mob::Person::unpackProxy(UnPackageUtils& unpackageUtil) {
	sim_mob::Agent::unpackProxy(unpackageUtil);

	unpackageUtil >> specialStr;
//	unpackageUtil >> firstFrameTick;

//	specialStr = unpackageUtil.unpackBasicData<std::string> ();
//	firstFrameTick = unpackageUtil.unpackBasicData<bool> ();
}

/**
 * Serialization of Signal
 */

//Note: The new signal class will require packing, but don't do it yet
//       ---we will remove the old Signal class anyway. ~Seth
//void Signal::packProxy(PackageUtils& packageUtil) {}
//void Signal::unpackProxy(UnPackageUtils& unpackageUtil) {}

}

#endif
