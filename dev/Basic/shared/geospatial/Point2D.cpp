/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Point2D.hpp"

using namespace sim_mob;

bool Point2D::nearToPoint(Point2D another, double distance) const
{
	double x_dis = xPos - another.getX();
	double y_dis = yPos - another.getY();

	if(x_dis < distance && x_dis > -distance) {
		if(y_dis < distance && y_dis > -distance) {
			return true;
		}
	}
	return false;
}


std::ostream& sim_mob::operator<<(std::ostream& stream, Point2D const & point)
{
    stream << "(" << point.getX() << ", " << point.getY() << ")";
    return stream;
}


bool sim_mob::operator==(Point2D const & p1, Point2D const & p2)
{
    return (p1.getX() == p2.getX()) && (p1.getY() == p2.getY());
}


bool sim_mob::operator!=(Point2D const & p1, Point2D const & p2)
{
    return !(p1 == p2);
}

