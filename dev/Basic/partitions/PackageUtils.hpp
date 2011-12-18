#pragma once

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>

#include <sstream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>

#include "geospatial/RoadNetwork.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "perception/FixedDelayed.hpp"

namespace sim_mob {

class BoundaryProcessor;

class Point2D;
class DPoint;
class Vehicle;
class TripChain;
class TripActivity;
class GeneralPathMover;
class IntersectionDrivingModel;
class SimpleIntDrivingModel;

class PackageUtils {
private:
	std::stringstream buffer;
	boost::archive::text_oarchive* package;

public:
	//Basic type
	//Include: int, double, float, bool, std::string
	template<class DATA_TYPE>
	void packageBasicData(DATA_TYPE value);

	//STL
	template<class DATA_TYPE>
	void packageBasicDataList(std::list<DATA_TYPE> value);

	template<class DATA_TYPE>
	void packageBasicDataVector(std::vector<DATA_TYPE> value);

	template<class DATA_TYPE>
	void packageBasicDataSet(std::set<DATA_TYPE> value);

	template<class DATA_TYPE_1, class DATA_TYPE_2>
	void packageBasicDataMap(std::map<DATA_TYPE_1, DATA_TYPE_2> value);

	//Road Network
	void packageNode(const Node* one_node);
	void packageRoadSegment(const RoadSegment* roadsegment);
	void packageLink(const Link* one_link);
	void packageLane(const Lane* one_lane);
	void packageTripChain(const TripChain* tripChain);
	void packageTripActivity(const TripActivity* tripActivity);

	//Road Item
	void packageVehicle(const Vehicle* one_vehicle);
	void packageGeneralPathMover(const sim_mob::GeneralPathMover* mover);
	void packageCrossing(const Crossing* one_crossing);

	//Other struct
	void packageIntersectionDrivingModel(SimpleIntDrivingModel* one_model);
	void packageFixedDelayedDPoint(FixedDelayed<DPoint*>& one_delay);
	void packageFixedDelayedDouble(FixedDelayed<double>& one_delay);
	void packageFixedDelayedInt(FixedDelayed<int>& one_delay);
	void packagePoint2D(const Point2D* one_point);

private:
	std::string getPackageData();
	void initializePackage();
	void clearPackage();

public:
	friend class BoundaryProcessor;
};

template<class DATA_TYPE>
void PackageUtils::packageBasicData(DATA_TYPE value) {
	(*package) & value;
}

template<class DATA_TYPE>
void PackageUtils::packageBasicDataList(std::list<DATA_TYPE> value) {
	(*package) & value;
}

template<class DATA_TYPE>
void PackageUtils::packageBasicDataVector(std::vector<DATA_TYPE> value) {
	(*package) & value;
}

template<class DATA_TYPE>
void PackageUtils::packageBasicDataSet(std::set<DATA_TYPE> value) {
	(*package) & value;
}

template<class DATA_TYPE_1, class DATA_TYPE_2>
void PackageUtils::packageBasicDataMap(std::map<DATA_TYPE_1, DATA_TYPE_2> value) {
	(*package) & value;
}

}
#endif
