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

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "perception/FixedDelayed.hpp"
#include "util/DailyTime.hpp"
#include "util/DynamicVector.hpp"
#include "geospatial/Point2D.hpp"

namespace sim_mob
{

/**
 * \author Xu Yan
 * Function:
 * UnPackageUtils is used in receiver side to unpack basic data type (like: vector<int>) and some SimMobility data type (like: Node).
 * Note:
 * PackageUtils/UnPackageUtils have matching functions, if you add/edit/remove one function in this class, you need to check class PackageUtils
 */
class UnPackageUtils {
private:
	std::stringstream buffer;
	boost::archive::text_iarchive* package;

public:
	UnPackageUtils(std::string data);
	~UnPackageUtils();

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline DATA_TYPE unpackBasicData() const {
		DATA_TYPE value;
		(*package) & value;
		return value;
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline std::list<DATA_TYPE> unpackBasicDataList() const {
		std::list<DATA_TYPE> value;
		(*package) & value;
		return value;
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline std::vector<DATA_TYPE> unpackBasicDataVector() const {
		std::vector<DATA_TYPE> value;
		(*package) & value;
		return value;
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	inline std::set<DATA_TYPE> unpackBasicDataSet() const {
		std::set<DATA_TYPE> value;
		(*package) & value;
		return value;
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	inline std::map<DATA_TYPE_1, DATA_TYPE_2> unpackBasicDataMap() const {
		std::map<DATA_TYPE_1, DATA_TYPE_2> value;
		(*package) & value;
		return value;
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 * (not tested yet)
	 */
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	inline boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2> unpackUnorderedMap() const {
		boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2> value;
		(*package) & value;
		return value;
	}

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 * (not tested yet)
	 */
	template<class DATA_TYPE>
	inline boost::unordered_set<DATA_TYPE> packUnorderedSet() const {
		boost::unordered_set<DATA_TYPE> value;
		(*package) & value;
		return value;
	}

	//Road Network
//	const Node* unpackNode() const;
//	const RoadSegment* unpackRoadSegment() const;
//	const Link* unpackLink() const;
//	const Lane* unpackLane() const;
//	const TripChain* unpackTripChain() const;
//	const TripActivity* unpackTripActivity() const;

	//Road Item
//	const Vehicle* unpackVehicle() const;
//	void unpackGeneralPathMover(GeneralPathMover* one_motor) const;
//	const Crossing* unpackCrossing() const;

	//Others
//	IntersectionDrivingModel* unpackIntersectionDrivingModel() const;
	FixedDelayed<DPoint*>& unpackFixedDelayedDPoint() const;
	FixedDelayed<double>& unpackFixedDelayedDouble() const;
	FixedDelayed<int>& unpackFixedDelayedInt() const;
	Point2D* unpackPoint2D() const;
//	void unpackDriverUpdateParams(DriverUpdateParams& one_driver) const;
//	void unpackPedestrianUpdateParams(PedestrianUpdateParams& one_pedestrain) const;

	void unpackDailyTime(DailyTime& time) const;
	void unpackDPoint(DPoint& point) const;
	void unpackDynamicVector(DynamicVector& vector) const;

//private:
//	void initializePackage(std::string value);
//	void clearPackage();
};
}
#endif
