/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Crossing.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "util/GeomHelpers.hpp"

namespace sim_mob {
void Crossing::pack(PackageUtils& package, Crossing* one_cross) {
	if (one_cross == NULL) {
		bool is_NULL = true;
		package.packBasicData(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package.packBasicData(is_NULL);
	}

	Point2D near1 = one_cross->nearLine.first;
	Point2D near2 = one_cross->nearLine.second;
	Point2D far1 = one_cross->farLine.first;
	Point2D far2 = one_cross->farLine.second;

	package.packPoint2D(near1);
	package.packPoint2D(near2);
	package.packPoint2D(far1);
	package.packPoint2D(far2);

}

const Crossing* Crossing::unpack(UnPackageUtils& unpackage) {
	bool is_NULL = unpackage.unpackBasicData<bool>();
	if(is_NULL)
	{
		return NULL;
	}

	sim_mob::Point2D near_1 = *(unpackage.unpackPoint2D());
	sim_mob::Point2D near_2 = *(unpackage.unpackPoint2D());
	sim_mob::Point2D far_1 = *(unpackage.unpackPoint2D());
	sim_mob::Point2D far_2 = *(unpackage.unpackPoint2D());

	return sim_mob::getCrossingBasedOnNode(&near_1, &near_2, &far_1, &far_2);
}
}
#endif
