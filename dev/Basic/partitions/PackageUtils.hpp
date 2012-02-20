#pragma once

#ifndef SIMMOB_DISABLE_MPI

#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
//#include <boost/serialization/unordered_set.hpp>
//#include <boost/serialization/unordered_map.hpp>

#include <sstream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>
//#include <unordered_set>
//#include <unordered_map>

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
 * Function:
 * PackageUtils is used in sender side to pack basic data type (like: vector<int>) and some SimMobility data type (like: Node).
 * Note:
 * PackageUtils/UnPackageUtils have matching functions, if you add/edit/remove one function in this class, you need to check class UnPackageUtils
 */
class PackageUtils {
private:
	std::stringstream buffer;
	boost::archive::text_oarchive* package;

public:
	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline void packBasicData(DATA_TYPE value)
	{
		(*package) & value;
	}

	//check the double value is not non and then package
	void safePackageDoubleValue(double value);

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline void packBasicDataList(const std::list<DATA_TYPE>& value)
	{
		(*package) & value;
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline void packBasicDataVector(const std::vector<DATA_TYPE>& value)
	{
		(*package) & value;
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline void packBasicDataSet(const std::set<DATA_TYPE>& value)
	{
		(*package) & value;
	}

	//Basic type (DATA_TYPE)
	//Includes: int, double, float, bool, std::string
	//(Can not be pointer, struct, class, union)
//	template<class DATA_TYPE>
//	inline void packBasicDataUnorderedSet(const std::unordered_set<DATA_TYPE>& value) {
//		(*package) & value;
//	}
//
//	//Basic type (DATA_TYPE)
//	//Includes: int, double, float, bool, std::string
//	//(Can not be pointer, struct, class, union)
//	template<class DATA_TYPE>
//	inline void packBasicDataUnorderedMap(const std::unordered_map<DATA_TYPE>& value) {
//		(*package) & value;
//	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	inline void packBasicDataMap(const std::map<DATA_TYPE_1, DATA_TYPE_2>& value)
	{
		(*package) & value;
	}

	//Road Network
	void packRoadNetworkElement(const Node* one_node);
	void packRoadNetworkElement(const RoadSegment* roadsegment);
	void packRoadNetworkElement(const Link* one_link);
	void packRoadNetworkElement(const Lane* one_lane);
	void packRoadNetworkElement(const TripChain* tripChain);
	void packRoadNetworkElement(const TripActivity* tripActivity);

	//Road Item
	void packVehicle(const Vehicle* one_vehicle);
	void packGeneralPathMover(const sim_mob::GeneralPathMover* mover);
	void packCrossing(const Crossing* one_crossing);

	//Other struct
	void packIntersectionDrivingModel(const SimpleIntDrivingModel* one_model);
	void packFixedDelayedDPoint(const FixedDelayed<DPoint*>& one_delay);
	void packFixedDelayedDouble(const FixedDelayed<double>& one_delay);
	void packFixedDelayedInt(const FixedDelayed<int>& one_delay);
	void packPoint2D(const Point2D& one_point);
	void packDriverUpdateParams(const DriverUpdateParams& one_driver);
	void packPedestrianUpdateParams(const PedestrianUpdateParams& one_pedestrain);

private:
	std::string getPackageData();
	void initializePackage();
	void clearPackage();

public:
	friend class BoundaryProcessor;
};

}
#endif
