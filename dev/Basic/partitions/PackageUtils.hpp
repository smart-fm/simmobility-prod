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

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "perception/FixedDelayed.hpp"
#include "util/DailyTime.hpp"
#include "util/DynamicVector.hpp"
#include "geospatial/Point2D.hpp"

namespace sim_mob {

class BoundaryProcessor;

class Point2D;
class DPoint;

class IntersectionDrivingModel;
class SimpleIntDrivingModel;

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
	PackageUtils();
	~PackageUtils();
public:
	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline void packBasicData(DATA_TYPE value) {
		(*package) & value;
	}

	template<class DATA_TYPE>
	inline void operator<<(DATA_TYPE& value) {
		packBasicData(value);
	}

	/**
	 *Check whether the double value is NaN.
	 */
	inline void packBasicData(double value) {
		double buffer = 0;
		if (value != value)
			(*package) & buffer;
		else
			(*package) & value;
	}

	inline void operator<<(double value) {
		packBasicData(value);
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline void packBasicDataList(const std::list<DATA_TYPE>& value) {
		(*package) & value;
	}

	template<class DATA_TYPE>
	inline void operator<<(const std::list<DATA_TYPE>& value) {
		packBasicDataList(value);
	}

	//	template<class DATA_TYPE>
	//	inline void operator<<(const std::list<DATA_TYPE>& value) {
	//		(*package) & value;
	//	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline void packBasicDataVector(const std::vector<DATA_TYPE>& value) {
		(*package) & value;
	}

	template<class DATA_TYPE>
	inline void operator<<(const std::vector<DATA_TYPE>& value) {
		packBasicDataVector(value);
	}

	//	template<class DATA_TYPE>
	//	inline void operator<<(const std::list<DATA_TYPE>& value) {
	//		(*package) & value;
	//	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline void packBasicDataSet(const std::set<DATA_TYPE>& value) {
		(*package) & value;
	}

	template<class DATA_TYPE>
	inline void operator<<(const std::set<DATA_TYPE>& value) {
		packBasicDataSet(value);
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	inline void packBasicDataMap(const std::map<DATA_TYPE_1, DATA_TYPE_2>& value) {
		(*package) & value;
	}

	template<class DATA_TYPE_1, class DATA_TYPE_2>
	inline void operator<<(const std::map<DATA_TYPE_1, DATA_TYPE_2>& value) {
		packBasicDataMap(value);
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 * (not tested yet)
	 */
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	inline void packUnorderedMap(const boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2>& value) {
		(*package) & value;
	}

	template<class DATA_TYPE_1, class DATA_TYPE_2>
	inline void operator<<(const boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2>& value) {
		packUnorderedMap(value);
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 * (not tested yet)
	 */
	template<class DATA_TYPE>
	inline void packUnorderedSet(const boost::unordered_set<DATA_TYPE>& value) {
		(*package) & value;
	}

	template<class DATA_TYPE>
	inline void operator<<(const boost::unordered_set<DATA_TYPE>& value) {
		packUnorderedSet(value);
	}

	//Road Network
	//void packNode(const Node* one_node);
	//void packRoadSegment(const RoadSegment* roadsegment);
	//void packLink(const Link* one_link);
	//void packLane(const Lane* one_lane);
//	void packTripChain(const TripChain* tripChain);
//	void packTripActivity(const TripActivity* tripActivity);

	//Road Item
	//void packVehicle(const Vehicle* one_vehicle);
	//void packGeneralPathMover(const sim_mob::GeneralPathMover* mover);
	//void packCrossing(const Crossing* one_crossing);

	//Other struct
//	void packIntersectionDrivingModel(const SimpleIntDrivingModel* one_model);
	void packFixedDelayedDPoint(const FixedDelayed<DPoint*>& one_delay);
	void packFixedDelayedDouble(const FixedDelayed<double>& one_delay);
	void packFixedDelayedInt(const FixedDelayed<int>& one_delay);
	void packPoint2D(const Point2D& one_point);
//	void packDriverUpdateParams(const DriverUpdateParams& one_driver);
//	void packPedestrianUpdateParams(const PedestrianUpdateParams& one_pedestrain);

	void packDailyTime(const DailyTime& time);
	void packDPoint(const DPoint& point);
	void packDynamicVector(const DynamicVector& vector);

private:
	std::string getPackageData();
//	void initializePackage();
//	void clearPackage();

public:
	friend class BoundaryProcessor;
};

}
#endif
