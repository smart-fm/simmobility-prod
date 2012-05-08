#pragma once
#include "GenConfig.h"
#include "util/LangHelpers.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#endif

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
 *
 * \note
 * PackageUtils/UnPackageUtils have matching functions, if you add/edit/remove one function in this class, you need to check class PackageUtils
 *
 * \note
 * If the flag SIMMOB_DISABLE_MPI is defined, then this class is completely empty. It still exists as a friend class to anything
 * which can be serialized so that we can avoid lots of #idefs elsewhere in the code. ~Seth
 */
class UnPackageUtils {


private:
	std::stringstream buffer;

#ifndef SIMMOB_DISABLE_MPI
	boost::archive::text_iarchive* package;
#endif

public:
	UnPackageUtils(std::string data) CHECK_MPI_THROW ;
	~UnPackageUtils() CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	DATA_TYPE unpackBasicData() const CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	std::list<DATA_TYPE> unpackBasicDataList() const CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	std::vector<DATA_TYPE> unpackBasicDataVector() const CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE>
	std::set<DATA_TYPE> unpackBasicDataSet() const CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 */
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	std::map<DATA_TYPE_1, DATA_TYPE_2> unpackBasicDataMap() const CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 * (not tested yet)
	 */
	template<class DATA_TYPE_1, class DATA_TYPE_2>
	boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2> unpackUnorderedMap() const CHECK_MPI_THROW ;

	/**
	 * DATA_TYPE can be:
	 * (1)int {unsigned, signed}, long, short
	 * (2)float, double, char
	 * (3)bool
	 * (not tested yet)
	 */
	template<class DATA_TYPE>
	boost::unordered_set<DATA_TYPE> packUnorderedSet() const CHECK_MPI_THROW ;

	//Others
	FixedDelayed<DPoint*>& unpackFixedDelayedDPoint() const CHECK_MPI_THROW ;
	FixedDelayed<double>& unpackFixedDelayedDouble() const CHECK_MPI_THROW ;
	FixedDelayed<int>& unpackFixedDelayedInt() const CHECK_MPI_THROW ;
	Point2D* unpackPoint2D() const CHECK_MPI_THROW ;

	void unpackDailyTime(DailyTime& time) const CHECK_MPI_THROW ;
	void unpackDPoint(DPoint& point) const CHECK_MPI_THROW ;
	void unpackDynamicVector(DynamicVector& vector) const CHECK_MPI_THROW ;

};
}



//Template declarations. As this is considered source, it is if-def'd for now.

#ifndef SIMMOB_DISABLE_MPI

template<class DATA_TYPE>
inline DATA_TYPE sim_mob::UnPackageUtils::unpackBasicData() const {
	DATA_TYPE value;
	(*package) & value;
	return value;
}

template<class DATA_TYPE>
inline std::list<DATA_TYPE> sim_mob::UnPackageUtils::unpackBasicDataList() const {
	std::list<DATA_TYPE> value;
	(*package) & value;
	return value;
}

template<class DATA_TYPE>
inline std::vector<DATA_TYPE> sim_mob::UnPackageUtils::unpackBasicDataVector() const {
	std::vector<DATA_TYPE> value;
	(*package) & value;
	return value;
}

template<class DATA_TYPE>
inline std::set<DATA_TYPE> sim_mob::UnPackageUtils::unpackBasicDataSet() const {
	std::set<DATA_TYPE> value;
	(*package) & value;
	return value;
}

template<class DATA_TYPE_1, class DATA_TYPE_2>
inline std::map<DATA_TYPE_1, DATA_TYPE_2> sim_mob::UnPackageUtils::unpackBasicDataMap() const {
	std::map<DATA_TYPE_1, DATA_TYPE_2> value;
	(*package) & value;
	return value;
}

template<class DATA_TYPE_1, class DATA_TYPE_2>
inline boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2> sim_mob::UnPackageUtils::unpackUnorderedMap() const {
	boost::unordered_map<DATA_TYPE_1, DATA_TYPE_2> value;
	(*package) & value;
	return value;
}

template<class DATA_TYPE>
inline boost::unordered_set<DATA_TYPE> sim_mob::UnPackageUtils::packUnorderedSet() const {
	boost::unordered_set<DATA_TYPE> value;
	(*package) & value;
	return value;
}


#endif

