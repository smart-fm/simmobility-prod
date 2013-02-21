#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using sim_mob::WorkGroupFactory;
using std::string;
using std::pair;

void sim_mob::conf::workgroup_pimpl::pre ()
{
}

pair<string, WorkGroupFactory> sim_mob::conf::workgroup_pimpl::post_workgroup ()
{
	//TODO: We currently only have 2 types of workgroups, and they are not yet fully
	//      generic. Eventually, you can specify any kind of WorkGroup here and hook it
	//      up later. For now, we only suppor the old syntax.
	if (wgID=="agent") {
		return std::make_pair(wgID, WorkGroupFactory(numWorkers, true, false));
	} else if (wgID=="signal") {
		return std::make_pair(wgID, WorkGroupFactory(numWorkers, false, true));
	} else {
		throw std::runtime_error("Fully generic WorkGroups not yet supported (see Workgroup_Imp.cpp).");
	}
}

void sim_mob::conf::workgroup_pimpl::id (const ::std::string& id)
{
	wgID = id;
}

void sim_mob::conf::workgroup_pimpl::workers (int workers)
{
	numWorkers = workers;
}

