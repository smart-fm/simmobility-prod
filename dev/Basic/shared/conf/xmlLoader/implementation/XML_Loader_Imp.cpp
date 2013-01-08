#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::xml_loader_pimpl::pre ()
{
}

void sim_mob::conf::xml_loader_pimpl::post_xml_loader ()
{
}

void sim_mob::conf::xml_loader_pimpl::file (const ::std::string& file)
{
  std::cout << "file: " << file << std::endl;
}

void sim_mob::conf::xml_loader_pimpl::root_element (const ::std::string& root_element)
{
  std::cout << "root_element: " << root_element << std::endl;
}


