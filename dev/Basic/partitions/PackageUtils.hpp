#pragma once

#include "GenConfig.h"
#include "util/LangHelpers.hpp"

#include <sstream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#endif

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "perception/FixedDelayed.hpp"


namespace unit_tests {
class PackUnpackUnitTests;
}


namespace sim_mob {

class BoundaryProcessor;
class DynamicVector;
class DPoint;
class DailyTime;
class Point2D;
class IntersectionDrivingModel;
class SimpleIntDrivingModel;



/**
 * \author Xu Yan
 * Function:
 * PackageUtils is used in sender side to pack basic data type (like: vector<int>) and some SimMobility data type (like: Node).
 *
 * \note
 * PackageUtils/UnPackageUtils have matching functions, if you add/edit/remove one function in this class, you need to check class UnPackageUtils
 *
 * \note
 * If the flag SIMMOB_DISABLE_MPI is defined, then this class is completely empty. It still exists as a friend class to anything
 * which can be serialized so that we can avoid lots of #idefs elsewhere in the code. ~Seth
 */
class PackageUtils {

public:
	PackageUtils() CHECK_MPI_THROW ;
	~PackageUtils() CHECK_MPI_THROW ;
public:
	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	void packBasicData(DATA_TYPE value) CHECK_MPI_THROW ;

	template<class DATA_TYPE>
	void operator<<(DATA_TYPE& value) CHECK_MPI_THROW ;

	/**
	 *Check whether the double value is NaN.
	 */
	void packBasicData(double value) CHECK_MPI_THROW ;
	void operator<<(double value) CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	void packBasicDataList(const std::list<DATA_TYPE>& value) CHECK_MPI_THROW ;

	template<class DATA_TYPE>
	void operator<<(const std::list<DATA_TYPE>& value) CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	void packBasicDataVector(const std::vector<DATA_TYPE>& value) CHECK_MPI_THROW ;

	template<class DATA_TYPE>
	void operator<<(const std::vector<DATA_TYPE>& value) CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	void packBasicDataSet(const std::set<DATA_TYPE>& value) CHECK_MPI_THROW ;

	template<class DATA_TYPE>
	void operator<<(const std::set<DATA_TYPE>& value) CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	void packBasicDataMap(const std::map<DATA_TYPE_1, DATA_TYPE_2>& value) CHECK_MPI_THROW ;

	template<class DATA_TYPE_1, class DATA_TYPE_2>
	void operator<<(const std::map<DATA_TYPE_1, DATA_TYPE_2>& value) CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 * (not tested yet)
	 */
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	void packUnorderedMap(const boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2>& value) CHECK_MPI_THROW ;

	template<class DATA_TYPE_1, class DATA_TYPE_2>
	void operator<<(const boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2>& value) CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 * (not tested yet)
	 */
	template<class DATA_TYPE>
	void packUnorderedSet(const boost::unordered_set<DATA_TYPE>& value) CHECK_MPI_THROW ;

	template<class DATA_TYPE>
	void operator<<(const boost::unordered_set<DATA_TYPE>& value) CHECK_MPI_THROW ;

	//Other struct
	void packFixedDelayedDPoint(const FixedDelayed<DPoint*>& one_delay) CHECK_MPI_THROW ;
	void packFixedDelayedDouble(const FixedDelayed<double>& one_delay) CHECK_MPI_THROW ;
	void packFixedDelayedInt(const FixedDelayed<int>& one_delay) CHECK_MPI_THROW ;
	void packPoint2D(const Point2D& one_point) CHECK_MPI_THROW ;

	void packDailyTime(const DailyTime& time) CHECK_MPI_THROW ;
	void packDPoint(const DPoint& point) CHECK_MPI_THROW ;
	void packDynamicVector(const DynamicVector& vector) CHECK_MPI_THROW ;

private:
	std::string getPackageData() CHECK_MPI_THROW ;


private:
	friend class BoundaryProcessor;
	friend class unit_tests::PackUnpackUnitTests;

#ifndef SIMMOB_DISABLE_MPI
	std::stringstream buffer;
	boost::archive::text_oarchive* package;
#endif

};


//Template definitions. These are essentially source, so they are #ifdef'd like everything else.

#ifndef SIMMOB_DISABLE_MPI

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::packBasicData(DATA_TYPE value) {
	(*package) & value;
}

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::operator<<(DATA_TYPE& value) {
	packBasicData(value);
}

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::packBasicDataList(const std::list<DATA_TYPE>& value) {
	(*package) & value;
}

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::operator<<(const std::list<DATA_TYPE>& value) {
	packBasicDataList(value);
}

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::packBasicDataVector(const std::vector<DATA_TYPE>& value) {
	(*package) & value;
}

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::operator<<(const std::vector<DATA_TYPE>& value) {
	packBasicDataVector(value);
}

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::packBasicDataSet(const std::set<DATA_TYPE>& value) {
	(*package) & value;
}

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::operator<<(const std::set<DATA_TYPE>& value) {
	packBasicDataSet(value);
}

template<class DATA_TYPE_1, class DATA_TYPE_2>
inline void sim_mob::PackageUtils::packBasicDataMap(const std::map<DATA_TYPE_1, DATA_TYPE_2>& value) {
	(*package) & value;
}

template<class DATA_TYPE_1, class DATA_TYPE_2>
inline void sim_mob::PackageUtils::operator<<(const std::map<DATA_TYPE_1, DATA_TYPE_2>& value) {
	packBasicDataMap(value);
}

template<class DATA_TYPE_1, class DATA_TYPE_2>
inline void sim_mob::PackageUtils::packUnorderedMap(const boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2>& value) {
	(*package) & value;
}

template<class DATA_TYPE_1, class DATA_TYPE_2>
inline void sim_mob::PackageUtils::operator<<(const boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2>& value) {
	packUnorderedMap(value);
}

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::packUnorderedSet(const boost::unordered_set<DATA_TYPE>& value) {
	(*package) & value;
}

template<class DATA_TYPE>
inline void sim_mob::PackageUtils::operator<<(const boost::unordered_set<DATA_TYPE>& value) {
	packUnorderedSet(value);
}

#endif


}

