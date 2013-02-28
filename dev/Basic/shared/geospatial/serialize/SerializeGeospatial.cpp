/* Copyright Singapore-MIT Alliance for Research and Technology */
#include "conf/settings/DisableMPI.h"

#include "util/LangHelpers.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "geospatial/Lane.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Pavement.hpp"

#include "util/GeomHelpers.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"

#include "conf/simpleconf.hpp"

/*
 * \author Xu Yan
 */

namespace sim_mob {

/**
 * Serialize Class Lane
 */
void Lane::pack(sim_mob::PackageUtils& package, const sim_mob::Lane* one_lane) {
	if (one_lane == NULL) {
		bool is_NULL = true;
		package<<(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package<<(is_NULL);
	}

	package << one_lane->getRoadSegment()->getStart()->location;
	package << one_lane->getRoadSegment()->getEnd()->location;
	package << one_lane->getLaneID();
}

Lane* Lane::unpack(sim_mob::UnPackageUtils& unpackage) {
	bool is_NULL = false;
	unpackage >> is_NULL;
	if (is_NULL) {
		return NULL;
	}

	sim_mob::Point2D start;
	sim_mob::Point2D end;
	int lane_id;

	unpackage >> start;
	unpackage >> end;
	unpackage >> lane_id;

	const sim_mob::RoadSegment* roadSegment = sim_mob::getRoadSegmentBasedOnNodes(&start, &end);
	const std::vector<sim_mob::Lane*>& lanes = roadSegment->getLanes();
	std::vector<sim_mob::Lane*>::const_iterator it = lanes.begin();

	for (; it != lanes.end(); it++) {
		if ((*it)->getLaneID() == lane_id)
			return *it;
	}
	return NULL;
}

/**
 * Serialize Class Link
 */

void sim_mob::Link::pack(PackageUtils& package,const Link* one_link)
{
	if (one_link == NULL) {
		bool is_NULL = true;
		package<<(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package<<(is_NULL);
	}

	package << one_link->getStart()->location;
	package << one_link->getEnd()->location;

//	sim_mob::Point2D point_1 = one_link->getStart()->location;
//	sim_mob::Point2D point_2 = one_link->getEnd()->location;
//
//	package.packPoint2D(point_1);
//	package.packPoint2D(point_2);
}

const Link* sim_mob::Link::unpack(UnPackageUtils& unpackage)
{
	bool is_NULL = false;
	unpackage >> is_NULL;
	if (is_NULL) {
		return NULL;
	}

	sim_mob::Point2D point_1;
	sim_mob::Point2D point_2;

	unpackage >> point_1;
	unpackage >> point_2;

//	point_1 = *(unpackage.unpackPoint2D());
//	point_2 = *(unpackage.unpackPoint2D());

	return sim_mob::getLinkBetweenNodes(&point_1, &point_2);
}

/**
 * Serialize Class Cross
 */
void Crossing::pack(PackageUtils& package, Crossing* one_cross) {
	if (one_cross == NULL) {
		bool is_NULL = true;
		package<<(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package<<(is_NULL);
	}

	package << one_cross->nearLine.first;
	package << one_cross->nearLine.second;
	package << one_cross->farLine.first;
	package << one_cross->farLine.second;

//	Point2D near1 = one_cross->nearLine.first;
//	Point2D near2 = one_cross->nearLine.second;
//	Point2D far1 = one_cross->farLine.first;
//	Point2D far2 = one_cross->farLine.second;
//
//	package.packPoint2D(near1);
//	package.packPoint2D(near2);
//	package.packPoint2D(far1);
//	package.packPoint2D(far2);

}

const Crossing* Crossing::unpack(UnPackageUtils& unpackage) {
	bool is_NULL = false;
	unpackage >> is_NULL;
	if(is_NULL)
	{
		return NULL;
	}

	sim_mob::Point2D near_1; //= *(unpackage.unpackPoint2D());
	sim_mob::Point2D near_2; //= *(unpackage.unpackPoint2D());
	sim_mob::Point2D far_1; //= *(unpackage.unpackPoint2D());
	sim_mob::Point2D far_2; //= *(unpackage.unpackPoint2D());

	unpackage >> near_1;
	unpackage >> near_2;
	unpackage >> far_1;
	unpackage >> far_2;

	return sim_mob::getCrossingBasedOnNode(&near_1, &near_2, &far_1, &far_2);
}

/**
 * Serialize Class Node
 */
void Node::pack(PackageUtils& package, const Node* one_node)
{
	if (one_node == NULL) {
		bool is_NULL = true;
		package<<(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package<<(is_NULL);
	}

	package<<(one_node->location);
}

Node* Node::unpack(UnPackageUtils& unpackage)
{
	bool is_NULL = false;
	unpackage >> is_NULL;
	if (is_NULL) {
		return NULL;
	}

	 sim_mob::Point2D location;
	 unpackage >> location;

	 const sim_mob::RoadNetwork& rn = ConfigParams::GetInstance().getNetwork();
	 return rn.locateNode(location, true);
}

/**
 * Class Road Segment
 */
void sim_mob::RoadSegment::pack(PackageUtils& package, const RoadSegment* one_segment)
{
	package << one_segment->getStart()->location;
	package << one_segment->getEnd()->location;

//	package.packPoint2D(one_segment->getStart()->location);
//	package.packPoint2D(one_segment->getEnd()->location);
}

const RoadSegment* sim_mob::RoadSegment::unpack(UnPackageUtils& unpackage)
{
	sim_mob::Point2D point_1;
	sim_mob::Point2D point_2;

	unpackage >> point_1;
	unpackage >> point_2;

//	point_1 = *(unpackage.unpackPoint2D());
//	point_2 = *(unpackage.unpackPoint2D());

	return sim_mob::getRoadSegmentBasedOnNodes(&point_1, &point_2);
}

/**
 * Class Pavement
 */
void sim_mob::Pavement::pack(PackageUtils& package, Pavement* one_pavement)
{
	if (one_pavement == NULL) {
		bool is_NULL = true;
		package << (is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package << (is_NULL);
	}
}

const Pavement* sim_mob::Pavement::unpack(UnPackageUtils& unpackage)
{
	bool is_NULL = false;
	unpackage >> is_NULL;
	if (is_NULL) {
		return NULL;
	}

	return NULL;
}
}

#endif
