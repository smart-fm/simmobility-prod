/**
 * AgentPackageManager.hpp
 * Target:
 * 1. package agents (focus on person)
 * 2. The modeler should define what to package and send;
 * 3. The modeler should define how to process the received data;
 */

#pragma once

#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include "partitions/RoadNetworkPackageManager.hpp"
#include "perception/FixedDelayed.hpp"

namespace sim_mob {

/**
 * for receiver to know what is the class structure
 */
enum role_modes
{
	No_Role = 0, Driver_Role, Pedestrian_Role, Passenger_Role
};

/**
 * for receiver to know what is the class structure
 */
enum message_type
{
	No_MESSAGE_CLASS = 0, ACCIDENT_SEND, ACCIDENT_RECV
};

/**
 * for receiver to know what is the target of communication
 */
enum message_tags
{
	No_MESSAGE = 1000,
	TYPE_TAGE,
	BOUNDARY_PROCESS,
	P2P_COMMUNICATION_MESSAGE,
	P2P_COMMUNICATION_MESSAGES,
	P2P_COMMUNICATION_IMEDIATE_RESULT
};

class AgentPackageManager
{
public:
	/**
	 *many classes need the help of AgentPackageManager, just for easy access
	 */
	static AgentPackageManager &
	instance()
	{
		return instance_;
	}

private:
	static AgentPackageManager instance_;
	AgentPackageManager()
	{
	}

public:

	template<class Archive>
	void packageOneCrossAgent(Archive & ar, Agent const* agent);

	template<class Archive>
	void packageOneFeedbackAgent(Archive & ar, Agent const* agent);

	template<class Archive>
	Agent * rebuildOneCrossAgent(Archive & ar, role_modes agent_type);

	template<class Archive>
	Agent * rebuildOneFeedbackAgent(Archive & ar, role_modes agent_type);

	void updateOneFeedbackAgent(Agent* new_agent, Agent * old_agent);

	//check the role type of one agent
	role_modes getAgentRoleType(Agent const* agent);

private:
	template<class Archive>
	bool checkSerilizationNotNull(Archive & ar, Agent const* agent);

	/**
	 * For package cross agents issues
	 */
	template<class Archive>
	void packageOneCrossDriver(Archive & ar, Agent const* agent);

	template<class Archive>
	void packageOneCrossPedestrain(Archive & ar, Agent const* agent);

	template<class Archive>
	void packageOneCrossPassenger(Archive & ar, Agent const* agent);

	/**
	 * For package feedback agents issues
	 */
	template<class Archive>
	void packageOneFeedbackDriver(Archive & ar, Agent const* agent);

	template<class Archive>
	void packageOneFeedbackPedestrain(Archive & ar, Agent const* agent);

	template<class Archive>
	void packageOneFeedbackPassenger(Archive & ar, Agent const* agent);

	/**
	 * For rebuild cross agents issues
	 */
	template<class Archive>
	Agent* rebuildOneCrossDriver(Archive & ar);

	template<class Archive>
	Agent* rebuildOneCrossPedestrian(Archive & ar);

	template<class Archive>
	Agent* rebuildOneCrossPassenger(Archive & ar);

	/**
	 * For rebuild feedback agents issues
	 */
	template<class Archive>
	Agent* rebuildOneFeedbackDriver(Archive & ar);

	template<class Archive>
	Agent* rebuildOneFeedbackPedestrian(Archive & ar);

	template<class Archive>
	Agent* rebuildOneFeedbackPassenger(Archive & ar);

	/**
	 * Update fake agents
	 */
	void updateOneFeedbackDriver(Agent * new_agent, Agent * old_agent);
	void updateOneFeedbackPedestrian(Agent * new_agent, Agent * old_agent);
	void updateOneFeedbackPassenger(Agent * new_agent, Agent * old_agent);
};

template<class Archive>
bool sim_mob::AgentPackageManager::checkSerilizationNotNull(Archive & ar, Agent const* agent)
{
	if (agent == NULL)
	{
		bool isNULL = false;
		ar & isNULL;
		return false;
	}
	else
	{
		bool isNULL = true;
		ar & isNULL;
		return true;
	}
}

/**
 * Agent Processing
 */

template<class Archive>
void sim_mob::AgentPackageManager::packageOneCrossAgent(Archive & ar, Agent const* agent)
{
	//std::cout << "packageOneCrossDriver 2.2:" << std::endl;
	if (checkSerilizationNotNull(ar, agent) == false)
	{
		return;
	}

	//std::cout << "packageOneCrossDriver 2.3:" << std::endl;
	role_modes agent_type = getAgentRoleType(agent);

	/**
	 * The modeler should define the functions for each type of agent
	 */
	if (agent_type == Driver_Role)
	{
		packageOneCrossDriver(ar, agent);
	}
	else if (agent_type == Pedestrian_Role)
	{
		packageOneCrossPedestrain(ar, agent);
	}
	else if (agent_type == Passenger_Role)
	{
		//		packageOneCrossPassenger(ar, agent);
	}
}

template<class Archive>
void sim_mob::AgentPackageManager::packageOneFeedbackAgent(Archive & ar, Agent const* agent)
{

//	packageOneCrossAgent(ar, agent);

		if (checkSerilizationNotNull(ar, agent) == false) {
			return;
		}

		role_modes agent_type = getAgentRoleType(agent);

		/**
		 * The modeler should define the functions for each type of agent
		 */
		if (agent_type == Driver_Role) {
			packageOneFeedbackDriver(ar, agent);
		} else if (agent_type == Pedestrian_Role) {
			packageOneFeedbackPedestrain(ar, agent);
		} else if (agent_type == Passenger_Role) {
			//packageOneFeedbackDriver(ar, agent);
		}
}

template<class Archive>
Agent* sim_mob::AgentPackageManager::rebuildOneCrossAgent(Archive & ar, role_modes agent_type)
{
	bool isNotNULL;
	ar & isNotNULL;

	if (isNotNULL == false)
		return NULL;

	if (agent_type == Driver_Role)
	{
		return rebuildOneCrossDriver(ar);
	}
	else if (agent_type == Pedestrian_Role)
	{
		return rebuildOneCrossPedestrian(ar);
	}
	else if (agent_type == Passenger_Role)
	{
		return NULL; //rebuildOneCrossPassenger(ar);
	}
	else
	{
		std::cout << "Error: not found agent type" << std::endl;
		return 0;
	}
}

template<class Archive>
Agent* sim_mob::AgentPackageManager::rebuildOneFeedbackAgent(Archive & ar, role_modes agent_type)
{

//	return rebuildOneCrossAgent(ar, agent_type);

		bool isNotNULL;
		ar & isNotNULL;

		if (isNotNULL == false)
			return NULL;

		if (agent_type == Driver_Role) {
			//std::cout << "There 4.11" << std::endl;
			return rebuildOneFeedbackDriver(ar);
		} else if (agent_type == Pedestrian_Role) {
			//std::cout << "There 4.22" << std::endl;
			return rebuildOneFeedbackPedestrian(ar);
		} else if (agent_type == Passenger_Role) {
			//std::cout << "There 4.33" << std::endl;
			return NULL; //rebuildOneFeedbackPassenger(ar);
		} else {
			std::cout << "Error: not found agent type" << std::endl;
			return NULL;
		}
}

/**
 * Driver Processing
 */

template<class Archive>
void sim_mob::AgentPackageManager::packageOneCrossDriver(Archive & ar, Agent const* agent)
{
	const Person *person = dynamic_cast<const Person *> (agent);
	Person* one_person = const_cast<Person*> (person);

	const Driver *driver = dynamic_cast<const Driver *> (one_person->getRole());

	/**
	 *Step 0: package Entity-related attributes
	 *Step 1: package agent-related attributes
	 *Step 2: package person-related attributes
	 *Step 3: package driver-related attributes
	 */

	//std::cout << "packageOneCrossDriver 2.4:" << std::endl;
	ar & (agent->id);
	ar & (agent->isSubscriptionListBuilt);

	sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();
	rnpackageImpl.packageNode(ar, person->originNode);
	rnpackageImpl.packageNode(ar, person->destNode);

	ar & (agent->startTime);
	ar & (agent->xPos.get());
	ar & (agent->yPos.get());
	ar & (agent->xAcc.get());
	ar & (agent->yAcc.get());
	ar & (agent->xVel.get());
	ar & (agent->yVel.get());
	ar & (agent->toRemoved);

	//Step 2
	rnpackageImpl.packageTripChain(ar, person->currTripChain);

	//std::cout << "packageOneCrossDriver 3:" << std::endl;

	//Step 3
	rnpackageImpl.packageVehicle(ar, driver->vehicle);

	//		rnpackageImpl.packageFixedDelayed(ar, &(driver->perceivedVelocity));
	//		rnpackageImpl.packageFixedDelayed(ar, &(driver->perceivedVelocityOfFwdCar));
	//		rnpackageImpl.packageFixedDelayed(ar, &(driver->perceivedDistToFwdCar));

//	rnpackageImpl.packageFixedDelayedPointer2D(ar, driver->perceivedVelocity);
//	rnpackageImpl.packageFixedDelayedPointer2D(ar, driver->perceivedVelocityOfFwdCar);
//	rnpackageImpl.packageFixedDelayedInt(ar, driver->perceivedDistToFwdCar);

	//	driver->perceivedVelocity.delayMS;
	//	driver->perceivedVelocity.delayMS;

	ar & (driver->timeStep);
	ar & (driver->perceivedXVelocity_);
	ar & (driver->perceivedYVelocity_);
	ar & (driver->xPos_nextLink);
	ar & (driver->yPos_nextLink);
	ar & (driver->xPosCrossing_);
	ar & (driver->acc_);
	ar & (driver->xDirection);
	ar & (driver->yDirection);
	ar & (driver->crossingFarX);
	ar & (driver->crossingFarY);
	ar & (driver->crossingNearX);
	ar & (driver->crossingNearY);
	ar & (driver->origin);
	ar & (driver->goal);

	rnpackageImpl.packageNode(ar, driver->destNode);
	rnpackageImpl.packageNode(ar, driver->originNode);

	ar & (driver->isGoalSet);
	ar & (driver->isOriginSet);

	ar & (driver->maxAcceleration);
	ar & (driver->normalDeceleration);
	ar & (driver->maxDeceleration);
	ar & (driver->distanceToNormalStop);
	ar & (driver->maxLaneSpeed);

	/****************IN REAL NETWORK****************/
	int all_segments_size = driver->allRoadSegments.size();
	ar & all_segments_size;
	std::vector<const RoadSegment*>::const_iterator it = driver->allRoadSegments.begin();

	for (; it != driver->allRoadSegments.end(); it++)
	{
		rnpackageImpl.packageRoadSegment(ar, (*it));
	}

	//std::cout << "packageOneCrossDriver 5:" << std::endl;

	rnpackageImpl.packageLink(ar, driver->currLink);
	//std::cout << "packageOneCrossDriver 6.1:" << std::endl;
	//		rnpackageImpl.packageLink(ar, driver->nextLink);
	//std::cout << "packageOneCrossDriver 6.2:" << std::endl;
	rnpackageImpl.packageRoadSegment(ar, driver->currRoadSegment);
	//std::cout << "packageOneCrossDriver 6.3:" << std::endl;
	//		rnpackageImpl.packageLane(ar, currLane);
	rnpackageImpl.packageLane(ar, driver->nextLaneInNextLink);
	//std::cout << "packageOneCrossDriver 6.4:" << std::endl;
	rnpackageImpl.packageLane(ar, driver->leftLane);
	//std::cout << "packageOneCrossDriver 6.5:" << std::endl;
	rnpackageImpl.packageLane(ar, driver->rightLane);

	//		rnpackageImpl.packageLink(ar, driver->desLink);


	ar & (driver->currLaneOffset);
	ar & (driver->currLinkOffset);
	ar & (driver->traveledDis);
	ar & (driver->RSIndex);
	ar & (driver->polylineSegIndex);
	ar & (driver->currLaneIndex);
	ar & (driver->targetLaneIndex);

	//std::cout << "driver->RSIndex:" << driver->RSIndex << std::endl;

	rnpackageImpl.packageLaneAndIndexPair(ar, &(driver->laneAndIndexPair));

	//I will solve it later
	//		ar & laneAndIndexPair;
	ar & (*(driver->currLanePolyLine));
	//		ar & (*(driver->desLanePolyLine));

	//std::cout << "packageOneCrossDriver 7.0:" << std::endl;

	ar & (driver->currPolylineSegStart);
	ar & (driver->currPolylineSegEnd);
	ar & (driver->polylineSegLength);
	//std::cout << "packageOneCrossDriver 7.1:" << std::endl;

	ar & (driver->desPolyLineStart);
	ar & (driver->desPolyLineEnd);
	ar & (driver->entryPoint);
	ar & (driver->xTurningStart);
	ar & (driver->yTurningStart);

	//std::cout << "packageOneCrossDriver 7.2:" << std::endl;

	ar & (driver->currLaneLength);
	ar & (driver->xDirection_entryPoint);
	ar & (driver->yDirection_entryPoint);
	ar & (driver->disToEntryPoint);
	ar & (driver->isCrossingAhead);
	ar & (driver->closeToCrossing);
	ar & (driver->isForward);
	ar & (driver->nextIsForward);
	ar & (driver->isReachGoal);
	ar & (driver->lcEnterNewLane);
	ar & (driver->isTrafficLightStop);

	//std::cout << "packageOneCrossDriver 7.3:" << std::endl;

	//		ar & nearby_agents);
	ar & (driver->distanceInFront);
	ar & (driver->distanceBehind);

	//std::cout << "packageOneCrossDriver 8:" << std::endl;

	rnpackageImpl.packageLane(ar, driver->currLane_.get());

	ar & (driver->currLaneOffset_.get());
	ar & (driver->currLaneLength_.get());

	/***********SOMETHING BIG BROTHER CAN RETURN*************/
	ar & (driver->targetSpeed);
	ar & (driver->minCFDistance);
	ar & (driver->minCBDistance);
	ar & (driver->minLFDistance);
	ar & (driver->minLBDistance);
	ar & (driver->minRFDistance);
	ar & (driver->minRBDistance);
	ar & (driver->minPedestrianDis);
	ar & (driver->tsStopDistance);
	ar & (driver->space);
	ar & (driver->headway);
	ar & (driver->space_star);
	ar & (driver->dv);
	ar & (driver->a_lead);
	ar & (driver->v_lead);

	//std::cout << "packageOneCrossDriver 9:" << std::endl;

	//for lane changing decision
	ar & (driver->VelOfLaneChanging);
	ar & (driver->changeMode);
	ar & (driver->changeDecision);
	ar & (driver->isLaneChanging);
	ar & (driver->isback);
	ar & (driver->isWaiting);
	ar & (driver->fromLane);
	ar & (driver->toLane);
	ar & (driver->satisfiedDistance);
	ar & (driver->dis2stop);
	ar & (driver->avoidBadAreaDistance);

	/**************BEHAVIOR WHEN APPROACHING A INTERSECTION***************/

	//std::cout << "packageOneCrossDriver 10:" << std::endl;
	rnpackageImpl.packageSignal(ar, driver->trafficSignal);

	ar & (driver->angle);
	ar & (driver->inIntersection);

	/**************COOPERATION WITH PEDESTRIAN***************/
	ar & (driver->isPedestrianAhead);
	//std::cout << "packageOneCrossDriver 11:" << std::endl;
}

template<class Archive>
void sim_mob::AgentPackageManager::packageOneFeedbackDriver(Archive & ar, Agent const* agent)
{
	const Person *person = dynamic_cast<const Person *> (agent);
		Person* one_person = const_cast<Person*> (person);

		const Driver *driver = dynamic_cast<const Driver *> (one_person->getRole());

		/**
		 *Step 0: package Entity-related attributes
		 *Step 1: package agent-related attributes
		 *Step 2: package person-related attributes
		 *Step 3: package driver-related attributes
		 */

		//std::cout << "packageOneCrossDriver 2.4:" << std::endl;
		ar & (agent->id);
		ar & (agent->isSubscriptionListBuilt);

		sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();
		rnpackageImpl.packageNode(ar, person->originNode);
		rnpackageImpl.packageNode(ar, person->destNode);

		ar & (agent->startTime);
		ar & (agent->xPos.get());
		ar & (agent->yPos.get());
		ar & (agent->xAcc.get());
		ar & (agent->yAcc.get());
		ar & (agent->xVel.get());
		ar & (agent->yVel.get());
		ar & (agent->toRemoved);

		//Step 2
		rnpackageImpl.packageTripChain(ar, person->currTripChain);

		//std::cout << "packageOneCrossDriver 3:" << std::endl;

		//Step 3
		rnpackageImpl.packageVehicle(ar, driver->vehicle);

		//		rnpackageImpl.packageFixedDelayed(ar, &(driver->perceivedVelocity));
		//		rnpackageImpl.packageFixedDelayed(ar, &(driver->perceivedVelocityOfFwdCar));
		//		rnpackageImpl.packageFixedDelayed(ar, &(driver->perceivedDistToFwdCar));

	//	rnpackageImpl.packageFixedDelayedPointer2D(ar, driver->perceivedVelocity);
	//	rnpackageImpl.packageFixedDelayedPointer2D(ar, driver->perceivedVelocityOfFwdCar);
	//	rnpackageImpl.packageFixedDelayedInt(ar, driver->perceivedDistToFwdCar);

		//	driver->perceivedVelocity.delayMS;
		//	driver->perceivedVelocity.delayMS;

		ar & (driver->timeStep);
		ar & (driver->perceivedXVelocity_);
		ar & (driver->perceivedYVelocity_);
		ar & (driver->xPos_nextLink);
		ar & (driver->yPos_nextLink);
		ar & (driver->xPosCrossing_);
		ar & (driver->acc_);
		ar & (driver->xDirection);
		ar & (driver->yDirection);
		ar & (driver->crossingFarX);
		ar & (driver->crossingFarY);
		ar & (driver->crossingNearX);
		ar & (driver->crossingNearY);
		ar & (driver->origin);
		ar & (driver->goal);

		rnpackageImpl.packageNode(ar, driver->destNode);
		rnpackageImpl.packageNode(ar, driver->originNode);

		ar & (driver->isGoalSet);
		ar & (driver->isOriginSet);

		ar & (driver->maxAcceleration);
		ar & (driver->normalDeceleration);
		ar & (driver->maxDeceleration);
		ar & (driver->distanceToNormalStop);
		ar & (driver->maxLaneSpeed);

		/****************IN REAL NETWORK****************/
		int all_segments_size = driver->allRoadSegments.size();
		ar & all_segments_size;
		std::vector<const RoadSegment*>::const_iterator it = driver->allRoadSegments.begin();

		for (; it != driver->allRoadSegments.end(); it++)
		{
			rnpackageImpl.packageRoadSegment(ar, (*it));
		}

		//std::cout << "packageOneCrossDriver 5:" << std::endl;

		rnpackageImpl.packageLink(ar, driver->currLink);
		//std::cout << "packageOneCrossDriver 6.1:" << std::endl;
		//		rnpackageImpl.packageLink(ar, driver->nextLink);
		//std::cout << "packageOneCrossDriver 6.2:" << std::endl;
		rnpackageImpl.packageRoadSegment(ar, driver->currRoadSegment);
		//std::cout << "packageOneCrossDriver 6.3:" << std::endl;
		//		rnpackageImpl.packageLane(ar, currLane);
		rnpackageImpl.packageLane(ar, driver->nextLaneInNextLink);
		//std::cout << "packageOneCrossDriver 6.4:" << std::endl;
		rnpackageImpl.packageLane(ar, driver->leftLane);
		//std::cout << "packageOneCrossDriver 6.5:" << std::endl;
		rnpackageImpl.packageLane(ar, driver->rightLane);

		//		rnpackageImpl.packageLink(ar, driver->desLink);


		ar & (driver->currLaneOffset);
		ar & (driver->currLinkOffset);
		ar & (driver->traveledDis);
		ar & (driver->RSIndex);
		ar & (driver->polylineSegIndex);
		ar & (driver->currLaneIndex);
		ar & (driver->targetLaneIndex);

		//std::cout << "driver->RSIndex:" << driver->RSIndex << std::endl;

		rnpackageImpl.packageLaneAndIndexPair(ar, &(driver->laneAndIndexPair));

		//I will solve it later
		//		ar & laneAndIndexPair;
		ar & (*(driver->currLanePolyLine));
		//		ar & (*(driver->desLanePolyLine));

		//std::cout << "packageOneCrossDriver 7.0:" << std::endl;

		ar & (driver->currPolylineSegStart);
		ar & (driver->currPolylineSegEnd);
		ar & (driver->polylineSegLength);
		//std::cout << "packageOneCrossDriver 7.1:" << std::endl;

		ar & (driver->desPolyLineStart);
		ar & (driver->desPolyLineEnd);
		ar & (driver->entryPoint);
		ar & (driver->xTurningStart);
		ar & (driver->yTurningStart);

		//std::cout << "packageOneCrossDriver 7.2:" << std::endl;

		ar & (driver->currLaneLength);
		ar & (driver->xDirection_entryPoint);
		ar & (driver->yDirection_entryPoint);
		ar & (driver->disToEntryPoint);
		ar & (driver->isCrossingAhead);
		ar & (driver->closeToCrossing);
		ar & (driver->isForward);
		ar & (driver->nextIsForward);
		ar & (driver->isReachGoal);
		ar & (driver->lcEnterNewLane);
		ar & (driver->isTrafficLightStop);

		//std::cout << "packageOneCrossDriver 7.3:" << std::endl;

		//		ar & nearby_agents);
		ar & (driver->distanceInFront);
		ar & (driver->distanceBehind);

		//std::cout << "packageOneCrossDriver 8:" << std::endl;

		rnpackageImpl.packageLane(ar, driver->currLane_.get());

		ar & (driver->currLaneOffset_.get());
		ar & (driver->currLaneLength_.get());

		/***********SOMETHING BIG BROTHER CAN RETURN*************/
		ar & (driver->targetSpeed);
		ar & (driver->minCFDistance);
		ar & (driver->minCBDistance);
		ar & (driver->minLFDistance);
		ar & (driver->minLBDistance);
		ar & (driver->minRFDistance);
		ar & (driver->minRBDistance);
		ar & (driver->minPedestrianDis);
		ar & (driver->tsStopDistance);
		ar & (driver->space);
		ar & (driver->headway);
		ar & (driver->space_star);
		ar & (driver->dv);
		ar & (driver->a_lead);
		ar & (driver->v_lead);

		//std::cout << "packageOneCrossDriver 9:" << std::endl;

		//for lane changing decision
		ar & (driver->VelOfLaneChanging);
		ar & (driver->changeMode);
		ar & (driver->changeDecision);
		ar & (driver->isLaneChanging);
		ar & (driver->isback);
		ar & (driver->isWaiting);
		ar & (driver->fromLane);
		ar & (driver->toLane);
		ar & (driver->satisfiedDistance);
		ar & (driver->dis2stop);
		ar & (driver->avoidBadAreaDistance);

		/**************BEHAVIOR WHEN APPROACHING A INTERSECTION***************/

		//std::cout << "packageOneCrossDriver 10:" << std::endl;
		//rnpackageImpl.packageSignal(ar, driver->trafficSignal);

		ar & (driver->angle);
		ar & (driver->inIntersection);

		/**************COOPERATION WITH PEDESTRIAN***************/
		ar & (driver->isPedestrianAhead);
//		std::cout << "packageOneCrossDriver 11:" << std::endl;
}

template<class Archive>
Agent* sim_mob::AgentPackageManager::rebuildOneCrossDriver(Archive & ar)
{
	Person* one_person = new Person();

	Driver* onerole = new Driver(one_person);
	one_person->changeRole(onerole);

	/**
	 *Step 1: rebuild agent
	 *Step 2: rebuild person
	 *Step 3: rebuild driver
	 */

	ar & (one_person->id);
	ar & (one_person->isSubscriptionListBuilt);

	sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();
	one_person->originNode = const_cast<sim_mob::Node*> (rnpackageImpl.unpackageNode(ar));
	one_person->destNode = const_cast<sim_mob::Node*> (rnpackageImpl.unpackageNode(ar));

	ar & (one_person->startTime);

	int x_pos, y_pos;
	double x_acc, y_acc;
	double x_vel, y_vel;

	ar & x_pos;
	ar & y_pos;
	ar & x_acc;
	ar & y_acc;
	ar & x_vel;
	ar & y_vel;

	one_person->xPos.force(x_pos);
	one_person->yPos.force(y_pos);
	one_person->xAcc.force(x_acc);
	one_person->yAcc.force(y_acc);
	one_person->xVel.force(x_vel);
	one_person->yVel.force(y_vel);

	ar & (one_person->toRemoved);

	//std::cout << "Here 3.11" << std::endl;
	//Step 2
	one_person->currTripChain = rnpackageImpl.unpackageTripChain(ar);

	//Step 3
	onerole->vehicle = rnpackageImpl.unpackageVehicle(ar);

	//		FixedDelayed<Point2D*>* delay_object_1 = new FixedDelayed<Point2D*>();
	//		onerole->perceivedVelocity = *(rnpackageImpl.unpackageFixedDelayed<Point2D*>(ar));
	//		onerole->perceivedVelocityOfFwdCar = *(rnpackageImpl.unpackageFixedDelayed<Point2D*>(ar));
	//		onerole->perceivedDistToFwdCar = *(rnpackageImpl.unpackageFixedDelayed<centimeter_t>(ar));

//	onerole->perceivedVelocity = rnpackageImpl.unpackageFixedDelayedPointer2D(ar);
//	onerole->perceivedVelocityOfFwdCar = rnpackageImpl.unpackageFixedDelayedPointer2D(ar);
//	onerole->perceivedDistToFwdCar = rnpackageImpl.unpackageFixedDelayedInt(ar);

	ar & (onerole->timeStep);
	ar & (onerole->perceivedXVelocity_);
	ar & (onerole->perceivedYVelocity_);
	ar & (onerole->xPos_nextLink);
	ar & (onerole->yPos_nextLink);
	ar & (onerole->xPosCrossing_);
	ar & (onerole->acc_);
	ar & (onerole->xDirection);
	ar & (onerole->yDirection);
	ar & (onerole->crossingFarX);
	ar & (onerole->crossingFarY);
	ar & (onerole->crossingNearX);
	ar & (onerole->crossingNearY);
	ar & (onerole->origin);
	ar & (onerole->goal);

	onerole->destNode = rnpackageImpl.unpackageNode(ar);
	onerole->originNode = rnpackageImpl.unpackageNode(ar);

	ar & (onerole->isGoalSet);
	ar & (onerole->isOriginSet);
	ar & (onerole->maxAcceleration);
	ar & (onerole->normalDeceleration);
	ar & (onerole->maxDeceleration);
	ar & (onerole->distanceToNormalStop);
	ar & (onerole->maxLaneSpeed);

	//std::cout << "Here 3.12" << std::endl;
	/****************IN REAL NETWORK****************/
	/**********Load in Road Network based on Point2D********/
	int all_segment_size;
	ar & all_segment_size;

	for (int i = 0; i < all_segment_size; i++)
	{
		const RoadSegment* one_road_segment = rnpackageImpl.unpackageRoadSegment(ar);
		onerole->allRoadSegments.push_back(one_road_segment);
	}

	onerole->currLink = rnpackageImpl.unpackageLink(ar);
	//		onerole->nextLink = rnpackageImpl.unpackageLink(ar);
	onerole->currRoadSegment = rnpackageImpl.unpackageRoadSegment(ar);
	onerole->nextLaneInNextLink = rnpackageImpl.unpackageLane(ar);
	onerole->leftLane = rnpackageImpl.unpackageLane(ar);
	onerole->rightLane = rnpackageImpl.unpackageLane(ar);
	//onerole->desLink = rnpackageImpl.unpackageLink(ar);

	//std::cout << "Here 3.13" << std::endl;

	ar & (onerole->currLaneOffset);
	ar & (onerole->currLinkOffset);
	ar & (onerole->traveledDis);
	ar & (onerole->RSIndex);
	ar & (onerole->polylineSegIndex);
	ar & (onerole->currLaneIndex);
	ar & (onerole->targetLaneIndex);
	//		ar & laneAndIndexPair;

	onerole->laneAndIndexPair = *(rnpackageImpl.unpackageLaneAndIndexPair(ar));

	std::vector<sim_mob::Point2D>* buffer_currLanePolyLine = new std::vector<sim_mob::Point2D>();
	ar & (*buffer_currLanePolyLine);
	onerole->currLanePolyLine = buffer_currLanePolyLine;

	ar & (onerole->currPolylineSegStart);
	ar & (onerole->currPolylineSegEnd);
	ar & (onerole->polylineSegLength);

	ar & (onerole->desPolyLineStart);
	ar & (onerole->desPolyLineEnd);
	ar & (onerole->entryPoint);
	ar & (onerole->xTurningStart);
	ar & (onerole->yTurningStart);

	ar & (onerole->currLaneLength);
	ar & (onerole->xDirection_entryPoint);
	ar & (onerole->yDirection_entryPoint);
	ar & (onerole->disToEntryPoint);
	ar & (onerole->isCrossingAhead);
	ar & (onerole->closeToCrossing);
	ar & (onerole->isForward);
	ar & (onerole->nextIsForward);
	ar & (onerole->isReachGoal);
	ar & (onerole->lcEnterNewLane);
	ar & (onerole->isTrafficLightStop);
	//		ar & nearby_agents;
	ar & (onerole->distanceInFront);
	ar & (onerole->distanceBehind);

	//std::cout << "Here 3.15" << std::endl;

	sim_mob::Lane* buffer_Lane = const_cast<sim_mob::Lane*> (rnpackageImpl.unpackageLane(ar));
	onerole->currLane_.force(buffer_Lane);
	//std::cout << "receiver driver->currLane:" << onerole->currLane_.get()->getRoadSegment()->getId() << std::endl;

	double buffer_currLaneOffset;
	ar & buffer_currLaneOffset;
	onerole->currLaneOffset_.force(buffer_currLaneOffset);

	double buffer_currLaneLength;
	ar & buffer_currLaneLength;
	onerole->currLaneLength_.force(buffer_currLaneLength);

	/***********SOMETHING BIG BROTHER CAN RETURN*************/
	ar & (onerole->targetSpeed);
	ar & (onerole->minCFDistance);
	ar & (onerole->minCBDistance);
	ar & (onerole->minLFDistance);
	ar & (onerole->minLBDistance);
	ar & (onerole->minRFDistance);
	ar & (onerole->minRBDistance);
	ar & (onerole->minPedestrianDis);
	ar & (onerole->tsStopDistance);
	ar & (onerole->space);
	ar & (onerole->headway);
	ar & (onerole->space_star);
	ar & (onerole->dv);
	ar & (onerole->a_lead);
	ar & (onerole->v_lead);

	//for lane changing decision
	ar & (onerole->VelOfLaneChanging);
	ar & (onerole->changeMode);
	ar & (onerole->changeDecision);
	ar & (onerole->isLaneChanging);
	ar & (onerole->isback);
	ar & (onerole->isWaiting);
	ar & (onerole->fromLane);
	ar & (onerole->toLane);
	ar & (onerole->satisfiedDistance);
	ar & (onerole->dis2stop);
	ar & (onerole->avoidBadAreaDistance);

	/**************BEHAVIOR WHEN APPROACHING A INTERSECTION***************/
	onerole->trafficSignal = rnpackageImpl.unpackageSignal(ar);

	ar & (onerole->angle);
	ar & (onerole->inIntersection);

	/**************COOPERATION WITH PEDESTRIAN***************/
	ar & (onerole->isPedestrianAhead);

	//std::cout << "Rebuild FINISHED" << std::endl;

	return one_person;
}

template<class Archive>
Agent* sim_mob::AgentPackageManager::rebuildOneFeedbackDriver(Archive & ar)
{
	Person* one_person = new Person();

		Driver* onerole = new Driver(one_person);
		one_person->changeRole(onerole);

		/**
		 *Step 1: rebuild agent
		 *Step 2: rebuild person
		 *Step 3: rebuild driver
		 */

		ar & (one_person->id);
		ar & (one_person->isSubscriptionListBuilt);

		sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();
		one_person->originNode = const_cast<sim_mob::Node*> (rnpackageImpl.unpackageNode(ar));
		one_person->destNode = const_cast<sim_mob::Node*> (rnpackageImpl.unpackageNode(ar));

		ar & (one_person->startTime);

		int x_pos, y_pos;
		double x_acc, y_acc;
		double x_vel, y_vel;

		ar & x_pos;
		ar & y_pos;
		ar & x_acc;
		ar & y_acc;
		ar & x_vel;
		ar & y_vel;

		one_person->xPos.force(x_pos);
		one_person->yPos.force(y_pos);
		one_person->xAcc.force(x_acc);
		one_person->yAcc.force(y_acc);
		one_person->xVel.force(x_vel);
		one_person->yVel.force(y_vel);

		ar & (one_person->toRemoved);

		//std::cout << "Here 3.11" << std::endl;
		//Step 2
		one_person->currTripChain = rnpackageImpl.unpackageTripChain(ar);

		//Step 3
		onerole->vehicle = rnpackageImpl.unpackageVehicle(ar);

		//		FixedDelayed<Point2D*>* delay_object_1 = new FixedDelayed<Point2D*>();
		//		onerole->perceivedVelocity = *(rnpackageImpl.unpackageFixedDelayed<Point2D*>(ar));
		//		onerole->perceivedVelocityOfFwdCar = *(rnpackageImpl.unpackageFixedDelayed<Point2D*>(ar));
		//		onerole->perceivedDistToFwdCar = *(rnpackageImpl.unpackageFixedDelayed<centimeter_t>(ar));

	//	onerole->perceivedVelocity = rnpackageImpl.unpackageFixedDelayedPointer2D(ar);
	//	onerole->perceivedVelocityOfFwdCar = rnpackageImpl.unpackageFixedDelayedPointer2D(ar);
	//	onerole->perceivedDistToFwdCar = rnpackageImpl.unpackageFixedDelayedInt(ar);

		ar & (onerole->timeStep);
		ar & (onerole->perceivedXVelocity_);
		ar & (onerole->perceivedYVelocity_);
		ar & (onerole->xPos_nextLink);
		ar & (onerole->yPos_nextLink);
		ar & (onerole->xPosCrossing_);
		ar & (onerole->acc_);
		ar & (onerole->xDirection);
		ar & (onerole->yDirection);
		ar & (onerole->crossingFarX);
		ar & (onerole->crossingFarY);
		ar & (onerole->crossingNearX);
		ar & (onerole->crossingNearY);
		ar & (onerole->origin);
		ar & (onerole->goal);

		onerole->destNode = rnpackageImpl.unpackageNode(ar);
		onerole->originNode = rnpackageImpl.unpackageNode(ar);

		ar & (onerole->isGoalSet);
		ar & (onerole->isOriginSet);
		ar & (onerole->maxAcceleration);
		ar & (onerole->normalDeceleration);
		ar & (onerole->maxDeceleration);
		ar & (onerole->distanceToNormalStop);
		ar & (onerole->maxLaneSpeed);

		//std::cout << "Here 3.12" << std::endl;
		/****************IN REAL NETWORK****************/
		/**********Load in Road Network based on Point2D********/
		int all_segment_size;
		ar & all_segment_size;

		for (int i = 0; i < all_segment_size; i++)
		{
			const RoadSegment* one_road_segment = rnpackageImpl.unpackageRoadSegment(ar);
			onerole->allRoadSegments.push_back(one_road_segment);
		}

		onerole->currLink = rnpackageImpl.unpackageLink(ar);
		//		onerole->nextLink = rnpackageImpl.unpackageLink(ar);
		onerole->currRoadSegment = rnpackageImpl.unpackageRoadSegment(ar);
		onerole->nextLaneInNextLink = rnpackageImpl.unpackageLane(ar);
		onerole->leftLane = rnpackageImpl.unpackageLane(ar);
		onerole->rightLane = rnpackageImpl.unpackageLane(ar);
		//onerole->desLink = rnpackageImpl.unpackageLink(ar);

		//std::cout << "Here 3.13" << std::endl;

		ar & (onerole->currLaneOffset);
		ar & (onerole->currLinkOffset);
		ar & (onerole->traveledDis);
		ar & (onerole->RSIndex);
		ar & (onerole->polylineSegIndex);
		ar & (onerole->currLaneIndex);
		ar & (onerole->targetLaneIndex);
		//		ar & laneAndIndexPair;

		onerole->laneAndIndexPair = *(rnpackageImpl.unpackageLaneAndIndexPair(ar));

		std::vector<sim_mob::Point2D>* buffer_currLanePolyLine = new std::vector<sim_mob::Point2D>();
		ar & (*buffer_currLanePolyLine);
		onerole->currLanePolyLine = buffer_currLanePolyLine;

		ar & (onerole->currPolylineSegStart);
		ar & (onerole->currPolylineSegEnd);
		ar & (onerole->polylineSegLength);

		ar & (onerole->desPolyLineStart);
		ar & (onerole->desPolyLineEnd);
		ar & (onerole->entryPoint);
		ar & (onerole->xTurningStart);
		ar & (onerole->yTurningStart);

		ar & (onerole->currLaneLength);
		ar & (onerole->xDirection_entryPoint);
		ar & (onerole->yDirection_entryPoint);
		ar & (onerole->disToEntryPoint);
		ar & (onerole->isCrossingAhead);
		ar & (onerole->closeToCrossing);
		ar & (onerole->isForward);
		ar & (onerole->nextIsForward);
		ar & (onerole->isReachGoal);
		ar & (onerole->lcEnterNewLane);
		ar & (onerole->isTrafficLightStop);
		//		ar & nearby_agents;
		ar & (onerole->distanceInFront);
		ar & (onerole->distanceBehind);

		//std::cout << "Here 3.15" << std::endl;

		sim_mob::Lane* buffer_Lane = const_cast<sim_mob::Lane*> (rnpackageImpl.unpackageLane(ar));
		onerole->currLane_.force(buffer_Lane);
		//std::cout << "receiver driver->currLane:" << onerole->currLane_.get()->getRoadSegment()->getId() << std::endl;

		double buffer_currLaneOffset;
		ar & buffer_currLaneOffset;
		onerole->currLaneOffset_.force(buffer_currLaneOffset);

		double buffer_currLaneLength;
		ar & buffer_currLaneLength;
		onerole->currLaneLength_.force(buffer_currLaneLength);

		/***********SOMETHING BIG BROTHER CAN RETURN*************/
		ar & (onerole->targetSpeed);
		ar & (onerole->minCFDistance);
		ar & (onerole->minCBDistance);
		ar & (onerole->minLFDistance);
		ar & (onerole->minLBDistance);
		ar & (onerole->minRFDistance);
		ar & (onerole->minRBDistance);
		ar & (onerole->minPedestrianDis);
		ar & (onerole->tsStopDistance);
		ar & (onerole->space);
		ar & (onerole->headway);
		ar & (onerole->space_star);
		ar & (onerole->dv);
		ar & (onerole->a_lead);
		ar & (onerole->v_lead);

		//for lane changing decision
		ar & (onerole->VelOfLaneChanging);
		ar & (onerole->changeMode);
		ar & (onerole->changeDecision);
		ar & (onerole->isLaneChanging);
		ar & (onerole->isback);
		ar & (onerole->isWaiting);
		ar & (onerole->fromLane);
		ar & (onerole->toLane);
		ar & (onerole->satisfiedDistance);
		ar & (onerole->dis2stop);
		ar & (onerole->avoidBadAreaDistance);

		/**************BEHAVIOR WHEN APPROACHING A INTERSECTION***************/
		//onerole->trafficSignal = rnpackageImpl.unpackageSignal(ar);

		ar & (onerole->angle);
		ar & (onerole->inIntersection);

		/**************COOPERATION WITH PEDESTRIAN***************/
		ar & (onerole->isPedestrianAhead);

		//std::cout << "Rebuild FINISHED" << std::endl;

		return one_person;
}

/**
 * Pedestrian Processing
 */

template<class Archive>
void sim_mob::AgentPackageManager::packageOneCrossPedestrain(Archive & ar, Agent const* agent)
{

//	std::cout << "packageOneCrossDriver Pdesttrains 2.4:" << std::endl;
	const Person *person = dynamic_cast<const Person *> (agent);
	Person* one_person = const_cast<Person*> (person);

	const Pedestrian *pedestrian = dynamic_cast<const Pedestrian *> (one_person->getRole());

	//std::cout << "packageOneCrossDriver Pdesttrains 2.5:" << std::endl;
	//Step 1: Agent-related
	ar & (agent->id);
	ar & (agent->isSubscriptionListBuilt);

	sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();
	rnpackageImpl.packageNode(ar, person->originNode);
	rnpackageImpl.packageNode(ar, person->destNode);

	//std::cout << "packageOneCrossDriver Pdesttrains 2.6:" << std::endl;
	ar & (agent->startTime);
	ar & (agent->xPos.get());
	ar & (agent->yPos.get());
	ar & (agent->xAcc.get());
//	std::cout << "packageOneCrossDriver Pdesttrains 2.7:" << std::endl;
	ar & (agent->yAcc.get());
	ar & (agent->xVel.get());
	ar & (agent->yVel.get());

	//std::cout << "packageOneCrossDriver Pdesttrains 2.8:" << std::endl;

	ar & (agent->toRemoved);

	//std::cout << "packageOneCrossDriver Pdesttrains 2.8.2:" << std::endl;

	//Step 2 Person-related
	rnpackageImpl.packageTripChain(ar, person->currTripChain);

	//Step 3 Pedestrian-related

	//std::cout << "packageOneCrossDriver Pdesttrains 2.8.3:" << std::endl;

	ar & (pedestrian->speed);
	ar & (pedestrian->xVel);
	ar & (pedestrian->yVel);
	ar & (pedestrian->goal);
	ar & (pedestrian->goalInLane);
	ar & (pedestrian->currentStage);

	//std::cout << "packageOneCrossDriver Pdesttrains 2.8.4:" << std::endl;

	rnpackageImpl.packageSignal(ar, pedestrian->trafficSignal);

	//std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.1:" << std::endl;
	rnpackageImpl.packageCrossing(ar, pedestrian->currCrossing);
	//	rnpackageImpl.pac
	//std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.2:" << std::endl;

	ar & (pedestrian->sigColor);
	ar & (pedestrian->curCrossingID);
	ar & (pedestrian->startToCross);
	ar & (pedestrian->cStartX);
	ar & (pedestrian->cStartY);
	ar & (pedestrian->cEndX);
	ar & (pedestrian->cEndY);
	ar & (pedestrian->firstTimeUpdate);

	//std::cout << "packageOneCrossDriver Pdesttrains 2.8.5:" << std::endl;

	ar & (pedestrian->xCollisionVector);
	ar & (pedestrian->yCollisionVector);

	//std::cout << "packageOneCrossDriver Pdesttrains 2.9:" << std::endl;

}

template<class Archive>
void sim_mob::AgentPackageManager::packageOneFeedbackPedestrain(Archive & ar, Agent const* agent)
{
	//std::cout << "packageOneCrossDriver Pdesttrains 2.4:" << std::endl;
		const Person *person = dynamic_cast<const Person *> (agent);
		Person* one_person = const_cast<Person*> (person);

		const Pedestrian *pedestrian = dynamic_cast<const Pedestrian *> (one_person->getRole());

		//std::cout << "packageOneCrossDriver Pdesttrains 2.5:" << std::endl;
		//Step 1: Agent-related
		ar & (agent->id);
		ar & (agent->isSubscriptionListBuilt);

		sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();
		rnpackageImpl.packageNode(ar, person->originNode);
		rnpackageImpl.packageNode(ar, person->destNode);

		//std::cout << "packageOneCrossDriver Pdesttrains 2.6:" << std::endl;
		ar & (agent->startTime);
		ar & (agent->xPos.get());
		ar & (agent->yPos.get());
		ar & (agent->xAcc.get());
		//std::cout << "packageOneCrossDriver Pdesttrains 2.7:" << std::endl;
		ar & (agent->yAcc.get());
		ar & (agent->xVel.get());
		ar & (agent->yVel.get());

		//std::cout << "packageOneCrossDriver Pdesttrains 2.8:" << std::endl;

		ar & (agent->toRemoved);

		//std::cout << "packageOneCrossDriver Pdesttrains 2.8.2:" << std::endl;

		//Step 2 Person-related
		rnpackageImpl.packageTripChain(ar, person->currTripChain);

		//Step 3 Pedestrian-related

		//std::cout << "packageOneCrossDriver Pdesttrains 2.8.3:" << std::endl;

		ar & (pedestrian->speed);
		ar & (pedestrian->xVel);
		ar & (pedestrian->yVel);
		ar & (pedestrian->goal);
		ar & (pedestrian->goalInLane);
		ar & (pedestrian->currentStage);

		//std::cout << "packageOneCrossDriver Pdesttrains 2.8.4:" << std::endl;

		//rnpackageImpl.packageSignal(ar, pedestrian->trafficSignal);

		//std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.1:" << std::endl;
		//rnpackageImpl.packageCrossing(ar, pedestrian->currCrossing);
		//	rnpackageImpl.pac
		//std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.2:" << std::endl;

		ar & (pedestrian->sigColor);
		ar & (pedestrian->curCrossingID);
		ar & (pedestrian->startToCross);
		ar & (pedestrian->cStartX);
		ar & (pedestrian->cStartY);
		ar & (pedestrian->cEndX);
		ar & (pedestrian->cEndY);
		ar & (pedestrian->firstTimeUpdate);

	//	std::cout << "packageOneCrossDriver Pdesttrains 2.8.5:" << std::endl;

		ar & (pedestrian->xCollisionVector);
		ar & (pedestrian->yCollisionVector);

		//std::cout << "packageOneCrossDriver Pdesttrains 2.9:" << std::endl;
}

template<class Archive>
Agent* sim_mob::AgentPackageManager::rebuildOneCrossPedestrian(Archive & ar)
{
	Person* one_person = new Person();

	Pedestrian* onerole = new Pedestrian(one_person);
	one_person->changeRole(onerole);

	/**
	 *Step 1: rebuild agent
	 *Step 2: rebuild person
	 *Step 3: rebuild driver
	 */

	ar & (one_person->id);
	ar & (one_person->isSubscriptionListBuilt);

	sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();
	one_person->originNode = const_cast<sim_mob::Node*> (rnpackageImpl.unpackageNode(ar));
	one_person->destNode = const_cast<sim_mob::Node*> (rnpackageImpl.unpackageNode(ar));

	ar & (one_person->startTime);

	int x_pos, y_pos;
	double x_acc, y_acc;
	double x_vel, y_vel;

	ar & x_pos;
	ar & y_pos;
	ar & x_acc;
	ar & y_acc;
	ar & x_vel;
	ar & y_vel;

	one_person->xPos.force(x_pos);
	one_person->yPos.force(y_pos);
	one_person->xAcc.force(x_acc);
	one_person->yAcc.force(y_acc);
	one_person->xVel.force(x_vel);
	one_person->yVel.force(y_vel);

	ar & (one_person->toRemoved);

	//std::cout << "Here 3.11" << std::endl;
	//Step 2
	one_person->currTripChain = rnpackageImpl.unpackageTripChain(ar);

	//Step 3 Pedestrian-related

	ar & (onerole->speed);
	ar & (onerole->xVel);
	ar & (onerole->yVel);
	ar & (onerole->goal);
	ar & (onerole->goalInLane);
	ar & (onerole->currentStage);

	onerole->trafficSignal = rnpackageImpl.unpackageSignal(ar);
	onerole->currCrossing = rnpackageImpl.unpackageCrossing(ar);

	//do not package crossing
	ar & (onerole->sigColor);
	ar & (onerole->curCrossingID);
	ar & (onerole->startToCross);
	ar & (onerole->cStartX);
	ar & (onerole->cStartY);
	ar & (onerole->cEndX);
	ar & (onerole->cEndY);
	ar & (onerole->firstTimeUpdate);

	ar & (onerole->xCollisionVector);
	ar & (onerole->yCollisionVector);

	return one_person;
}

template<class Archive>
Agent* sim_mob::AgentPackageManager::rebuildOneFeedbackPedestrian(Archive & ar)
{
	Person* one_person = new Person();

		Pedestrian* onerole = new Pedestrian(one_person);
		one_person->changeRole(onerole);

		/**
		 *Step 1: rebuild agent
		 *Step 2: rebuild person
		 *Step 3: rebuild driver
		 */

		ar & (one_person->id);
		ar & (one_person->isSubscriptionListBuilt);

		sim_mob::RoadNetworkPackageManager& rnpackageImpl = sim_mob::RoadNetworkPackageManager::instance();
		one_person->originNode = const_cast<sim_mob::Node*> (rnpackageImpl.unpackageNode(ar));
		one_person->destNode = const_cast<sim_mob::Node*> (rnpackageImpl.unpackageNode(ar));

		ar & (one_person->startTime);

		int x_pos, y_pos;
		double x_acc, y_acc;
		double x_vel, y_vel;

		ar & x_pos;
		ar & y_pos;
		ar & x_acc;
		ar & y_acc;
		ar & x_vel;
		ar & y_vel;

		one_person->xPos.force(x_pos);
		one_person->yPos.force(y_pos);
		one_person->xAcc.force(x_acc);
		one_person->yAcc.force(y_acc);
		one_person->xVel.force(x_vel);
		one_person->yVel.force(y_vel);

		ar & (one_person->toRemoved);

		//std::cout << "Here 3.11" << std::endl;
		//Step 2
		one_person->currTripChain = rnpackageImpl.unpackageTripChain(ar);

		//Step 3 Pedestrian-related

		ar & (onerole->speed);
		ar & (onerole->xVel);
		ar & (onerole->yVel);
		ar & (onerole->goal);
		ar & (onerole->goalInLane);
		ar & (onerole->currentStage);

		//onerole->trafficSignal = rnpackageImpl.unpackageSignal(ar);
		//onerole->currCrossing = rnpackageImpl.unpackageCrossing(ar);

		//do not package crossing
		ar & (onerole->sigColor);
		ar & (onerole->curCrossingID);
		ar & (onerole->startToCross);
		ar & (onerole->cStartX);
		ar & (onerole->cStartY);
		ar & (onerole->cEndX);
		ar & (onerole->cEndY);
		ar & (onerole->firstTimeUpdate);

		ar & (onerole->xCollisionVector);
		ar & (onerole->yCollisionVector);

		return one_person;
}

/**
 * Passenger Processing
 */

}

