#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::distribution_pimpl::pre ()
{
}

void sim_mob::conf::distribution_pimpl::post_distribution ()
{
}

void sim_mob::conf::distribution_pimpl::id (const ::std::string& id)
{
  std::cout << "id: " << id << std::endl;
}

void sim_mob::conf::distribution_pimpl::type (const ::std::string& type)
{
  std::cout << "type: " << type << std::endl;
}

void sim_mob::conf::distribution_pimpl::mean (int mean)
{
  std::cout << "mean: " << mean << std::endl;
}

void sim_mob::conf::distribution_pimpl::stdev (int stdev)
{
  std::cout << "stdev: " << stdev << std::endl;
}

