#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::default_model_pimpl::pre ()
{
}

void sim_mob::conf::default_model_pimpl::post_default_model ()
{
}

void sim_mob::conf::default_model_pimpl::type (const ::std::string& type)
{
  std::cout << "type: " << type << std::endl;
}

void sim_mob::conf::default_model_pimpl::default_ (const ::std::string& default_)
{
  std::cout << "default: " << default_ << std::endl;
}

