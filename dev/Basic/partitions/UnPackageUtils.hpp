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
#include <string>


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

	template<class DATA_TYPE>
	void operator>>(DATA_TYPE& value) CHECK_MPI_THROW ;

};
}



//Template declarations. As this is considered source, it is if-def'd for now.

#ifndef SIMMOB_DISABLE_MPI

template<class DATA_TYPE>
inline void sim_mob::UnPackageUtils::operator>>(DATA_TYPE& value) {
	(*package) & value;
}

#endif

