#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::val_units_pimpl::pre ()
{
	model = std::make_pair(0,"");
}

sim_mob::Granularity sim_mob::conf::val_units_pimpl::post_val_units ()
{
	return sim_mob::Granularity(model.first, model.second);
}

void sim_mob::conf::val_units_pimpl::value (int value)
{
	model.first = value;
}

void sim_mob::conf::val_units_pimpl::units (const ::std::string& value)
{
  model.second = value;
}

