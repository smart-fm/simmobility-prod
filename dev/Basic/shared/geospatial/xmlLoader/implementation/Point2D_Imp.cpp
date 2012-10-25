#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Point2D_t_pimpl::pre ()
{
	model = Point2D();
}

void sim_mob::xml::Point2D_t_pimpl::xPos (unsigned int value)
{
	//TODO: I'm not 100% sure why the XSD specification demands unsigned integers.
	//      I am leaving a static_cast in here to ensure that we resolve this in the XSD file.
	model.setX(static_cast<int>(value));
}

void sim_mob::xml::Point2D_t_pimpl::yPos (unsigned int value)
{
	model.setY(static_cast<int>(value));
}

sim_mob::Point2D sim_mob::xml::Point2D_t_pimpl::post_Point2D_t ()
{
	return model;
}

