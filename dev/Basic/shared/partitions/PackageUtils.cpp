#include "conf/settings/DisableMPI.h"

#include "PackageUtils.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "util/GeomHelpers.hpp"
#include "partitions/PartitionManager.hpp"

using namespace sim_mob;

std::string sim_mob::PackageUtils::getPackageData() {
	return buffer.str().data();
}

sim_mob::PackageUtils::PackageUtils()
{
	package = new boost::archive::text_oarchive(buffer);
}

sim_mob::PackageUtils::~PackageUtils()
{
	buffer.clear();
	safe_delete_item(package);
}

void sim_mob::PackageUtils::operator<<(double value) {
	if (value != value) {
		throw std::runtime_error("Double value is NAN.");
	}
	(*package) & value;
}


#endif
