#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::xml_loader_pimpl::pre ()
{
	model = std::make_pair("", "");
}

std::pair<std::string, std::string> sim_mob::conf::xml_loader_pimpl::post_xml_loader ()
{
	return model;
}

void sim_mob::conf::xml_loader_pimpl::file (const ::std::string& value)
{
	model.first = value;
}

void sim_mob::conf::xml_loader_pimpl::root_element (const ::std::string& value)
{
	model.first = value;
}


