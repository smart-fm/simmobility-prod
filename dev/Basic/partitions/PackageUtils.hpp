#pragma once

#ifndef SIMMOB_DISABLE_MPI

#include <boost/archive/text_oarchive.hpp>
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
class DriverUpdateParams;
class PedestrianUpdateParams;


/**
 * \author Xu Yan
 */
class PackageUtils {
private:
	std::stringstream buffer;
	boost::archive::text_oarchive* package;

public:
	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string, char
	//(Can not be pointer, struct, class, union, array)
	template<class DATA_TYPE>
	inline void packageBasicData(DATA_TYPE value)
	{
		(*package) & value;
	}

	//check the double value is not non and then package
	void safePackageDoubleValue(double value);

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE>
	inline void packageBasicDataList(const std::list<DATA_TYPE>& value)
	{
		(*package) & value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE>
	inline void packageBasicDataVector(const std::vector<DATA_TYPE>& value)
	{
		(*package) & value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE>
	inline void packageBasicDataSet(const std::set<DATA_TYPE>& value)
	{
		(*package) & value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE>
	inline void packageBasicDataUnorderedSet(const std::unordered_set<DATA_TYPE>& value) {
		(*package) & value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE>
	inline void packageBasicDataUnorderedMap(const std::unordered_map<DATA_TYPE>& value) {
		(*package) & value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	inline void packageBasicDataMap(const std::map<DATA_TYPE_1, DATA_TYPE_2>& value)
	{
		(*package) & value;
	}

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
	void packagePoint2D(const Point2D& one_point);
	void packageDriverUpdateParams(const DriverUpdateParams& one_driver);
	void packagePedestrianUpdateParams(const PedestrianUpdateParams& one_pedestrain);

private:
	std::string getPackageData();
	void initializePackage();
	void clearPackage();

public:
	friend class BoundaryProcessor;
};

}
#endif
