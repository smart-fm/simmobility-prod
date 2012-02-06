#pragma once
#include "GenConfig.h"

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/unordered_map.hpp>

#include <sstream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <unordered_set>
#include <unordered_map>

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


/**
 * \author Xu Yan
 */
class UnPackageUtils {
private:
	std::stringstream buffer;
	boost::archive::text_iarchive* package;

public:
	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE>
	inline DATA_TYPE unpackageBasicData()
	{
		DATA_TYPE value;
		(*package) & value;
		return value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE>
	inline std::list<DATA_TYPE> unpackageBasicDataList() const
	{
		std::list<DATA_TYPE> value;
		(*package) & value;
		return value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE>
	inline std::vector<DATA_TYPE> unpackageBasicDataVector() const
	{
		std::vector<DATA_TYPE> value;
		(*package) & value;
		return value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE>
	inline std::set<DATA_TYPE> unpackageBasicDataSet() const
	{
		std::set<DATA_TYPE> value;
		(*package) & value;
		return value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	inline std::map<DATA_TYPE_1, DATA_TYPE_2> unpackageBasicDataMap() const
	{
		std::map<DATA_TYPE_1, DATA_TYPE_2> value;
		(*package) & value;
		return value;
	}

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
}
#endif
