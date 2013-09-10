#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

namespace {
//Helper class: todo: consolidate later.
template <class Key, class Value>
Value find_in_map(std::map<Key, Value>& lookup, const Key& item) {
	typename std::map<Key, Value>::iterator it = lookup.find(item);
	if (it==lookup.end()) { throw std::runtime_error("Can't find key in map."); }
	return it->second;
}
}

void sim_mob::conf::react_times_pimpl::pre ()
{
}

void sim_mob::conf::react_times_pimpl::post_react_times ()
{
}

void sim_mob::conf::react_times_pimpl::leading_vehicle (const std::string& value)
{
	//config->simulation().leadingVehReactTime = find_in_map(config->constructs().distributions, value);
}

void sim_mob::conf::react_times_pimpl::subject_vehicle (const std::string& value)
{
	//config->simulation().subjectVehReactTime = find_in_map(config->constructs().distributions, value);
}

void sim_mob::conf::react_times_pimpl::vehicle_gap (const std::string& value)
{
	//config->simulation().vehicleGapReactTime = find_in_map(config->constructs().distributions, value);
}










