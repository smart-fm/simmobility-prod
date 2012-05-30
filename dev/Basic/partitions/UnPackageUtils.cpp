#include "UnPackageUtils.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "conf/simpleconf.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DynamicVector.hpp"
#include "util/DailyTime.hpp"
#include "geospatial/Point2D.hpp"

using namespace sim_mob;

sim_mob::UnPackageUtils::UnPackageUtils(std::string data)
{
	buffer << data;
	package = new boost::archive::text_iarchive(buffer);
}

sim_mob::UnPackageUtils::~UnPackageUtils()
{
	buffer.clear();
	safe_delete_item(package);
}

#endif
