//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
#include <geospatial/TurningPolyline.hpp>

namespace sim_mob {

TurningPolyline::TurningPolyline()
:Polyline(),turningId(-1),turning(nullptr)
{
}

TurningPolyline::TurningPolyline(const sim_mob::TurningPolyline& tp) : 
	Polyline(tp), turning(tp.turning), turningId(tp.turningId)
{
}

TurningPolyline::~TurningPolyline() 
{
}

void TurningPolyline::setPolypoints(std::vector<Polypoint*> polypoints)
{
	this->polypoints = polypoints;
}

void TurningPolyline::addPolypoint(Polypoint* point)
{
	this->polypoints.push_back(point);
}

std::vector<Polypoint*> TurningPolyline::getPolypoints() const
{
	return polypoints;
}

void TurningPolyline::setTurning(TurningSection* turning)
{
	this->turning = turning;
}

TurningSection* TurningPolyline::getTurning() const
{
	return turning;
}

void TurningPolyline::setTurningId(int turningId)
{
	this->turningId = turningId;
}

int TurningPolyline::getTurningId() const
{
	return turningId;
}

} /* namespace sim_mob */
