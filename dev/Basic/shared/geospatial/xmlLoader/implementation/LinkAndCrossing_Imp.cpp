#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::linkAndCrossing_t_pimpl::pre ()
{
	model = sim_mob::LinkAndCrossing();
}

sim_mob::LinkAndCrossing sim_mob::xml::linkAndCrossing_t_pimpl::post_linkAndCrossing_t ()
{
	return model;
}

void sim_mob::xml::linkAndCrossing_t_pimpl::ID (unsigned char value)
{
	model.id = value;
}

void sim_mob::xml::linkAndCrossing_t_pimpl::linkID (unsigned int value)
{
	model.link = book.getLink(value);
}

void sim_mob::xml::linkAndCrossing_t_pimpl::crossingID (unsigned int value)
{
	model.crossing = book.getCrossing(value);
	//std::cout << "crossingID: " <<value << std::endl;
}

void sim_mob::xml::linkAndCrossing_t_pimpl::angle (unsigned char value)
{
	//std::cout << "angle: " << static_cast<unsigned short>(value) << std::endl;
	model.angle = value;
}


