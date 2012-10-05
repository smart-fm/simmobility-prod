#include "geo9-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Point2D_t_pimpl::pre ()
{
	savedX = 0;
	savedY = 0;
}

void sim_mob::xml::Point2D_t_pimpl::xPos (unsigned int xPos)
{
	savedX = xPos;
}

void sim_mob::xml::Point2D_t_pimpl::yPos (unsigned int yPos)
{
	savedY = yPos;
}

sim_mob::Point2D sim_mob::xml::Point2D_t_pimpl::post_Point2D_t ()
{
	//TODO: I'm not 100% sure why the XSD specification demands unsigned integers.
	//      I am leaving a static_cast in here to ensure that we resolve this in the XSD file.
	return Point2D(static_cast<int>(savedX), static_cast<int>(savedY));
}

