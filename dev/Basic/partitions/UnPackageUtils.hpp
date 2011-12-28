#pragma once
#include "GenConfig.h"

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/text_iarchive.hpp>
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
#include "geospatial/Point2D.hpp"
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
class DriverUpdateParams;
class PedestrianUpdateParams;

class UnPackageUtils {
private:
	std::stringstream buffer;
	boost::archive::text_iarchive* package;

public:
	//Basic type
	//Include: int, double, float, bool, std::string
	template<class DATA_TYPE>
	DATA_TYPE unpackageBasicData();

	//STL: return value
	template<class DATA_TYPE>
	std::list<DATA_TYPE> unpackageBasicDataList();

	template<class DATA_TYPE>
	std::vector<DATA_TYPE> unpackageBasicDataVector();

	template<class DATA_TYPE>
	std::set<DATA_TYPE> unpackageBasicDataSet();

	template<class DATA_TYPE_1, class DATA_TYPE_2>
	std::map<DATA_TYPE_1, DATA_TYPE_2> unpackageBasicDataMap();

	//Road Network
	const Node* unpackageNode();
	const RoadSegment* unpackageRoadSegment();
	const Link* unpackageLink();
	const Lane* unpackageLane();
	const TripChain* unpackageTripChain();
	const TripActivity* unpackageTripActivity();

	//Road Item
	const Vehicle* unpackageVehicle();
	void unpackageGeneralPathMover(GeneralPathMover* one_motor);
	const Crossing* unpackageCrossing();

	//Others
	IntersectionDrivingModel* unpackageIntersectionDrivingModel();
	FixedDelayed<DPoint*>& unpackageFixedDelayedDPoint();
	FixedDelayed<double>& unpackageFixedDelayedDouble();
	FixedDelayed<int>& unpackageFixedDelayedInt();
	Point2D* unpackagePoint2D();
	void unpackageDriverUpdateParams(DriverUpdateParams& one_driver);
	void unpackagePedestrianUpdateParams(PedestrianUpdateParams& one_pedestrain);

private:
	void initializePackage(std::string value);
	void clearPackage();

public:
	friend class BoundaryProcessor;
};

template<class DATA_TYPE>
DATA_TYPE UnPackageUtils::unpackageBasicData() {
	DATA_TYPE value;
	(*package) & value;
	return value;
}

//STL: return value
template<class DATA_TYPE>
std::list<DATA_TYPE> UnPackageUtils::unpackageBasicDataList() {
	std::list<DATA_TYPE> value;
	(*package) & value;
	return value;
}

template<class DATA_TYPE>
std::vector<DATA_TYPE> UnPackageUtils::unpackageBasicDataVector() {
	std::vector<DATA_TYPE> value;
	(*package) & value;
	return value;
}

template<class DATA_TYPE>
std::set<DATA_TYPE> UnPackageUtils::unpackageBasicDataSet() {
	std::set<DATA_TYPE> value;
	(*package) & value;
	return value;
}

template<class DATA_TYPE_1, class DATA_TYPE_2>
std::map<DATA_TYPE_1, DATA_TYPE_2> UnPackageUtils::unpackageBasicDataMap() {
	std::map<DATA_TYPE_1, DATA_TYPE_2> value;
	(*package) & value;
	return value;
}

}
#endif
