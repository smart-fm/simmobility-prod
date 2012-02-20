/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Pavement.hpp"

#include <stdexcept>

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using namespace sim_mob;


RoadItemAndOffsetPair sim_mob::Pavement::nextObstacle(const Point2D& pos, bool isForward) const
{
	throw std::runtime_error("Not implemented yet.");
}


RoadItemAndOffsetPair sim_mob::Pavement::nextObstacle(centimeter_t offset, bool isForward) const
{
	//Simple!
	for (std::map<int, const RoadItem*>::const_iterator it=obstacles.begin(); it!=obstacles.end(); it++) {
		if (it->first >= offset) {
			return RoadItemAndOffsetPair(it->second, it->first);
		}
	}
	return RoadItemAndOffsetPair(nullptr, 0);
}

void sim_mob::Pavement::GeneratePolyline(Pavement* p, Point2D center, double bulge, int segmentLength)
{
	throw std::runtime_error("Not implemented yet.");
}

#ifndef SIMMOB_DISABLE_MPI
void sim_mob::Pavement::pack(PackageUtils& package, Pavement* one_pavement)
{
	if (one_pavement == NULL) {
		bool is_NULL = true;
		package.packBasicData(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package.packBasicData(is_NULL);
	}
}

const Pavement* sim_mob::Pavement::unpack(UnPackageUtils& unpackage)
{
	bool is_NULL = unpackage.unpackBasicData<bool> ();
	if (is_NULL) {
		return NULL;
	}

	return NULL;
}
#endif
