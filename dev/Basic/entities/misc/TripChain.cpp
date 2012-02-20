/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "TripChain.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "geospatial/Node.hpp"

namespace sim_mob {
void TripChain::pack(PackageUtils& package, const TripChain* chain) {
	if (chain == NULL) {
		bool is_NULL = true;
		package.packBasicData(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package.packBasicData(is_NULL);
	}

	package.packBasicData<std::string> (chain->from.description);
	sim_mob::Node::pack(package, chain->from.location);

	package.packBasicData<std::string> (chain->to.description);
	sim_mob::Node::pack(package, chain->to.location);

	package.packBasicData<bool> (chain->primary);
	package.packBasicData<bool> (chain->flexible);

	package.packDailyTime(chain->startTime);
	package.packBasicData<std::string> (chain->mode);
}

TripChain* TripChain::unpack(UnPackageUtils& unpackage) {
	bool is_NULL = unpackage.unpackBasicData<bool> ();
	if (is_NULL) {
		return NULL;
	}

	TripChain* chain = new TripChain();

	chain->from.description = unpackage.unpackBasicData<std::string> ();
	chain->from.location = Node::unpack(unpackage);
	chain->to.description = unpackage.unpackBasicData<std::string> ();
	chain->to.location = Node::unpack(unpackage);

	chain->primary = unpackage.unpackBasicData<bool> ();
	chain->flexible = unpackage.unpackBasicData<bool> ();

	unpackage.unpackDailyTime(chain->startTime);
	chain->mode = unpackage.unpackBasicData<std::string> ();

	return chain;
}
}
#endif
