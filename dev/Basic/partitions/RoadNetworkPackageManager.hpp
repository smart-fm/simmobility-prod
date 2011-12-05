/*
 * RoadNetworkPackageManager.hpp
 * Target:
 * 1. package road networks and others (like trip chain)
 * 2. The modeler should define what to package and send;
 * 3. The modeler should define how to process the received data;
 */

#pragma once

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>

#include "geospatial/RoadNetwork.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Point2D.hpp"

#include "conf/simpleconf.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/StreetDirectory.hpp"

#include "entities/Signal.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "entities/misc/TripChain.hpp"
#include "util/RelAbsPoint.hpp"
#include "util/GeomHelpers.hpp"
#include "perception/FixedDelayed.hpp"

#include <list>
#include <vector>

namespace sim_mob {

class RoadNetworkPackageManager {
public:
	static RoadNetworkPackageManager &
	instance() {
		return instance_;
	}

private:
	static RoadNetworkPackageManager instance_;

public:
	template<class Archive>
	void packageNode(Archive & ar, const Node* one_node);

	template<class Archive>
	const Node* unpackageNode(Archive & ar);

	template<class Archive>
	void packageRoadSegment(Archive & ar, const RoadSegment* roadsegment);

	template<class Archive>
	const RoadSegment* unpackageRoadSegment(Archive & ar);

	template<class Archive>
	void packageLink(Archive & ar, const Link* one_link);

	template<class Archive>
	const Link* unpackageLink(Archive & ar);

	template<class Archive>
	void packageLane(Archive & ar, const Lane* one_lane);

	template<class Archive>
	const Lane* unpackageLane(Archive & ar);

	template<class Archive>
	void packageTripActivity(Archive & ar, const TripActivity* tripActivity);

	template<class Archive>
	TripActivity* unpackageTripActivity(Archive & ar);

	template<class Archive>
	void packageTripChain(Archive & ar, const TripChain* tripChain);

	template<class Archive>
	TripChain* unpackageTripChain(Archive & ar);

	template<class Archive>
	void packageSignal(Archive & ar, const Signal* one_signal);

	template<class Archive>
	const Signal* unpackageSignal(Archive & ar);

	template<class Archive>
	void packageSignalContent(Archive & ar, const Signal* one_signal);

	template<class Archive>
	void unpackageSignalContent(Archive & ar);

	template<class Archive>
	void packageVehicle(Archive & ar, const Vehicle* one_vehicle);

	template<class Archive>
	Vehicle* unpackageVehicle(Archive & ar);

	template<class Archive>
	void packageRelAbsPoint(Archive & ar, const RelAbsPoint* attributes);

	template<class Archive>
	RelAbsPoint* unpackageRelAbsPoint(Archive & ar);

	template<class Archive>
	void packageFixedDelayedPointer2D(Archive & ar, const FixedDelayed<Point2D*>& one_delay);

	template<class Archive>
	FixedDelayed<Point2D*>& unpackageFixedDelayedPointer2D(Archive & ar);

	template<class Archive>
	void packageFixedDelayedInt(Archive & ar, const FixedDelayed<int32_t>& one_delay);

	template<class Archive>
	FixedDelayed<int32_t>& unpackageFixedDelayedInt(Archive & ar);

	template<class Archive>
	void packageLaneAndIndexPair(Archive & ar, const StreetDirectory::LaneAndIndexPair* lane_pair);

	template<class Archive>
	StreetDirectory::LaneAndIndexPair* unpackageLaneAndIndexPair(Archive & ar);

	template<class Archive>
	void packageCrossing(Archive & ar, const Crossing* cross);

	template<class Archive>
	const Crossing* unpackageCrossing(Archive & ar);
};

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageNode(Archive & ar, const Node* one_node) {
	bool hasSomthing = true;
	if (one_node == NULL) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	ar & (*(one_node->location));
}

template<class Archive>
const Node* sim_mob::RoadNetworkPackageManager::unpackageNode(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D location;
	ar & location;

	sim_mob::RoadNetwork& rn = ConfigParams::GetInstance().getNetwork();
	return rn.locateNode(location, true);
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageRoadSegment(Archive & ar, const RoadSegment* roadsegment) {
	bool hasSomthing = true;
	if (roadsegment == NULL) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	ar & (*roadsegment->getStart()->location);
	ar & (*roadsegment->getEnd()->location);
}

template<class Archive>
const RoadSegment* sim_mob::RoadNetworkPackageManager::unpackageRoadSegment(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D point_1;
	sim_mob::Point2D point_2;

	ar & point_1;
	ar & point_2;

	return sim_mob::getRoadSegmentBasedOnNodes(&point_1, &point_2);
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageLink(Archive & ar, const Link* one_link) {
	bool hasSomthing = true;
	if (one_link == NULL) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	ar & (*(one_link->getStart()->location));
	ar & (*(one_link->getEnd()->location));
}

template<class Archive>
const Link* sim_mob::RoadNetworkPackageManager::unpackageLink(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D point_1;
	sim_mob::Point2D point_2;

	ar & point_1;
	ar & point_2;

	return sim_mob::getLinkBetweenNodes(&point_1, &point_2);
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageLane(Archive & ar, const Lane* one_lane) {
	bool hasSomthing = true;
	if (one_lane == NULL) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	sim_mob::Point2D* start = one_lane->getRoadSegment()->getStart()->location;
	sim_mob::Point2D* end = one_lane->getRoadSegment()->getEnd()->location;
	int lane_id = one_lane->getLaneID();

	ar & (*start);
	ar & (*end);
	ar & lane_id;
}

template<class Archive>
const Lane* sim_mob::RoadNetworkPackageManager::unpackageLane(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D start;
	sim_mob::Point2D end;
	int lane_id;

	ar & start;
	ar & end;
	ar & lane_id;

	const sim_mob::RoadSegment* roadSegment = sim_mob::getRoadSegmentBasedOnNodes(&start, &end);

	const std::vector<sim_mob::Lane*>& lanes = roadSegment->getLanes();
	std::vector<sim_mob::Lane*>::const_iterator it = lanes.begin();

	for (; it != lanes.end(); it++) {
		if ((*it)->getLaneID() == lane_id)
			return (*it);
	}

	return NULL;
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageTripActivity(Archive & ar, const TripActivity* tripActivity) {
	bool hasSomthing = true;
	if (tripActivity == NULL) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	ar & (tripActivity->description);
	packageNode(ar, tripActivity->location);
}

template<class Archive>
TripActivity* sim_mob::RoadNetworkPackageManager::unpackageTripActivity(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	TripActivity* activity = new TripActivity();
	ar & (activity->description);

	activity->location = const_cast<sim_mob::Node*> (unpackageNode(ar));
	return activity;
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageTripChain(Archive & ar, const TripChain* tripChain) {
	bool hasSomthing = true;
	if (tripChain == NULL) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	packageTripActivity(ar, &(tripChain->from));
	packageTripActivity(ar, &(tripChain->to));

	ar & (tripChain->primary);
	ar & (tripChain->flexible);
	ar & (tripChain->startTime);
	ar & (tripChain->mode);
}

template<class Archive>
TripChain* sim_mob::RoadNetworkPackageManager::unpackageTripChain(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	TripChain* tripChain = new TripChain();
	tripChain->from = *(unpackageTripActivity(ar));
	tripChain->to = *(unpackageTripActivity(ar));

	ar & (tripChain->primary);
	ar & (tripChain->flexible);
	ar & (tripChain->startTime);
	ar & (tripChain->mode);

	return tripChain;
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageSignal(Archive & ar, const Signal* one_signal) {
	//std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.4:" << std::endl;
	bool hasSomthing = true;

	if (!one_signal) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	//	std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.4:" << hasSomthing << std::endl;
	//	std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.4:" << one_signal->getId() << std::endl;
	//	std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.4:" << one_signal->getNode().location << std::endl;
	//	std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.4:" << one_signal->getNode().location->getX() << std::endl;

	ar & (*(one_signal->getNode().location));
}

template<class Archive>
const Signal* sim_mob::RoadNetworkPackageManager::unpackageSignal(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D point_1;
	ar & point_1;

	//std::cout << "packageOneCrossDriver Pdesttrains 2.8.4.4.2:" << point_1.getX() << std::endl;

	return sim_mob::getSignalBasedOnNode(&point_1);
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageSignalContent(Archive & ar, const Signal* one_signal) {
	bool hasSomthing = true;
	if (!one_signal) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	//std::cout << "1111111111" << std::endl;

	packageSignal(ar, one_signal);

	//	int id = one_signal->getId();
	//	ar & id;
	ar & (one_signal->currCL);
	ar & (one_signal->currSplitPlan);
	ar & (one_signal->currOffset);
	ar & (one_signal->currPhase);
	ar & (one_signal->currSplitPlanID);
	ar & (one_signal->phaseCounter);

	//std::cout << "22222" << std::endl;

	//very dangerous, suggest to change
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			int value = one_signal->TC_for_Driver[i][j];
			//	std::cout << "value:" << value << std::endl;
			ar & value;
		}
	}

	//std::cout << "33333333" << std::endl;

	for (int i = 0; i < 4; i++) {
		int value = one_signal->TC_for_Pedestrian[i];
		ar & value;
	}

	//std::cout << "444444444444" << std::endl;
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::unpackageSignalContent(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return;

	const Signal* signal = unpackageSignal(ar);
	Signal* one_signal = const_cast<Signal*> (signal);

	//	int id;
	//	ar & (id);

	//	one_signal->id = id;
	ar & (one_signal->currCL);
	ar & (one_signal->currSplitPlan);
	ar & (one_signal->currOffset);
	ar & (one_signal->currPhase);
	ar & (one_signal->currSplitPlanID);
	ar & (one_signal->phaseCounter);

	//	int[][] TC_for_Driver = const_cast<int[][]> (one_signal->TC_for_Driver);

	//std::cout << "66666666666666" << std::endl;

	//very dangerous, suggest to change
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			//			int value;
			ar & (one_signal->TC_for_Driver[i][j]);
			//			TC_for_Driver[i][j] = value;
		}
	}

	//std::cout << "7777777777777" << std::endl;

	//	int[] TC_for_Pedestrian = const_cast<int[]> (one_signal->TC_for_Pedestrian);
	for (int i = 0; i < 4; i++) {
		//		int value;
		ar & (one_signal->TC_for_Pedestrian[i]);
		//		TC_for_Pedestrian[i] = value;
	}

	//std::cout << "888888888888" << std::endl;
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageVehicle(Archive & ar, const Vehicle* one_vehicle) {

	bool hasSomthing = true;
	if (one_vehicle == NULL) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	ar & (one_vehicle->length);
	ar & (one_vehicle->width);
	ar & (one_vehicle->timeStep);

	ar & (one_vehicle->xPos);
	ar & (one_vehicle->yPos);
	ar & (one_vehicle->xPos_);
	ar & (one_vehicle->yPos_);

	packageRelAbsPoint(ar, &(one_vehicle->velocity));
	packageRelAbsPoint(ar, &(one_vehicle->accel));
}

template<class Archive>
Vehicle* sim_mob::RoadNetworkPackageManager::unpackageVehicle(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	Vehicle* one_vehicle = new Vehicle();

	ar & (one_vehicle->length);
	ar & (one_vehicle->width);
	ar & (one_vehicle->timeStep);
	ar & (one_vehicle->xPos);
	ar & (one_vehicle->yPos);
	ar & (one_vehicle->xPos_);
	ar & (one_vehicle->yPos_);

	one_vehicle->velocity = *(unpackageRelAbsPoint(ar));
	one_vehicle->accel = *(unpackageRelAbsPoint(ar));

	return one_vehicle;
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageRelAbsPoint(Archive & ar, const RelAbsPoint* attributes) {
	bool hasSomthing = true;
	if (attributes == NULL) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	ar & (attributes->abs.x);
	ar & (attributes->abs.y);

	ar & (attributes->rel.x);
	ar & (attributes->rel.y);

	ar & (attributes->scaleDir.x);
	ar & (attributes->scaleDir.y);
}

template<class Archive>
RelAbsPoint* sim_mob::RoadNetworkPackageManager::unpackageRelAbsPoint(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	RelAbsPoint* attributes = new RelAbsPoint();

	ar & (attributes->abs.x);
	ar & (attributes->abs.y);

	ar & (attributes->rel.x);
	ar & (attributes->rel.y);

	ar & (attributes->scaleDir.x);
	ar & (attributes->scaleDir.y);

	return attributes;
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageFixedDelayedPointer2D(Archive & ar,
		const FixedDelayed<Point2D*>& one_delay) {
	int value = one_delay.delayMS;
	bool value_2 = one_delay.reclaimPtrs;

	ar & value;
	ar & value_2;

	ar.template register_type<FixedDelayed<Point2D*>::HistItem> ();
	ar & one_delay.history;
}

template<class Archive>
FixedDelayed<Point2D*>& sim_mob::RoadNetworkPackageManager::unpackageFixedDelayedPointer2D(Archive & ar) {
	int delayMS;
	ar & delayMS;

	bool reclaimPtrs;
	ar & reclaimPtrs;

	FixedDelayed<Point2D*>* one_delay = new FixedDelayed<Point2D*> (delayMS, reclaimPtrs);

	ar.template register_type<FixedDelayed<Point2D*>::HistItem> ();
	ar & (one_delay->history);

	return (*one_delay);
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageFixedDelayedInt(Archive & ar, const FixedDelayed<int32_t>& one_delay) {
	int value = one_delay.delayMS;
	bool value_2 = one_delay.reclaimPtrs;

	ar & value;
	ar & value_2;

	ar.template register_type<FixedDelayed<int32_t>::HistItem> ();
	ar & one_delay.history;
}

template<class Archive>
FixedDelayed<int32_t>& sim_mob::RoadNetworkPackageManager::unpackageFixedDelayedInt(Archive & ar) {
	int delayMS;
	ar & delayMS;

	bool reclaimPtrs;
	ar & reclaimPtrs;

	FixedDelayed<int32_t>* one_delay = new FixedDelayed<int32_t> (delayMS, reclaimPtrs);

	ar.template register_type<FixedDelayed<int32_t>::HistItem> ();
	ar & (one_delay->history);

	return (*one_delay);
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageLaneAndIndexPair(Archive & ar,
		const StreetDirectory::LaneAndIndexPair* lane_pair) {

	bool hasSomthing = true;
	if (lane_pair == NULL) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	packageLane(ar, lane_pair->lane_);
	ar & lane_pair->startIndex_;
	ar & lane_pair->endIndex_;
}

template<class Archive>
StreetDirectory::LaneAndIndexPair* sim_mob::RoadNetworkPackageManager::unpackageLaneAndIndexPair(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	StreetDirectory::LaneAndIndexPair* lane_pair = new StreetDirectory::LaneAndIndexPair();
	lane_pair->lane_ = unpackageLane(ar);
	ar & lane_pair->startIndex_;
	ar & lane_pair->endIndex_;

	return lane_pair;
}

template<class Archive>
void sim_mob::RoadNetworkPackageManager::packageCrossing(Archive & ar, const Crossing* cross) {
	bool hasSomthing = true;
	if (!cross) {
		hasSomthing = false;
		ar & hasSomthing;
		return;
	} else {
		ar & hasSomthing;
	}

	ar & cross->farLine.first;
	ar & cross->farLine.second;

	ar & cross->nearLine.first;
	ar & cross->nearLine.second;

	//	ar & cross->start;
	//	ar & cross->end;
}

template<class Archive>
const Crossing* sim_mob::RoadNetworkPackageManager::unpackageCrossing(Archive & ar) {
	bool hasSomthing;
	ar & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D from_1;
	sim_mob::Point2D from_2;
	sim_mob::Point2D to_1;
	sim_mob::Point2D to_2;

	ar & from_1;
	ar & from_2;
	ar & to_1;
	ar & to_2;

	return sim_mob::getCrossingBasedOnNode(&from_1, &from_2, &to_1, &to_2);
}

}
