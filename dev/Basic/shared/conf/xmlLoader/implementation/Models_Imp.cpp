#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::models_pimpl::pre ()
{
}

void sim_mob::conf::models_pimpl::post_models ()
{
}

void sim_mob::conf::models_pimpl::lane_changing (const pair<string, string>&)
{
}

void sim_mob::conf::models_pimpl::car_following (const pair<string, string>&)
{
}

void sim_mob::conf::models_pimpl::intersection_driving (const pair<string, string>&)
{
}

void sim_mob::conf::models_pimpl::sidewalk_movement (const pair<string, string>&)
{
}



