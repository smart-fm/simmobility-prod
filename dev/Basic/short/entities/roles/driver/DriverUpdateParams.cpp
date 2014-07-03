//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverUpdateParams.hpp"
#include "../short/entities/roles/driver/DriverFacets.hpp"

namespace sim_mob
{

void DriverUpdateParams::setStatus(unsigned int s)
{
	status |= s;
}
void DriverUpdateParams::setStatusDoingLC(LANE_CHANGE_SIDE& lcs)
{
	if(lcs == LCS_RIGHT)
	{
		setStatus(STATUS_LC_RIGHT);
	}
	else if(lcs == LCS_LEFT)
	{
		setStatus(STATUS_LC_LEFT);
	}

	// else do nothing
}
void DriverUpdateParams::buildDebugInfo()
{
	std::stringstream s;
	double ct=cftimer;
	if(abs(cftimer)<0.001)
		ct=0;
	s<<ct<<":"<<newFwdAcc<<":"<<accSelect<<":"<<nvFwd.exists()<<":"<<perceivedDistToFwdCar / 100.0;
	debugInfo = s.str();

	std::cout<<debugInfo<<std::endl;
}
void DriverUpdateParams::addTargetLanes(set<const Lane*> tl)
{
	set<const Lane*> newTargetLanes;
	set<const Lane*>::iterator it;

	// find Lane* in both tl and targetLanes
	for(it=tl.begin();it!=tl.end();++it)
	{
		const Lane* l = *it;
		set<const Lane*>::iterator itFind = targetLanes.find(l);
		if(itFind != targetLanes.end())
		{
			newTargetLanes.insert(l);
		}
	}

	targetLanes = newTargetLanes;
}
void DriverUpdateParams::unsetStatus(unsigned int s)
{
	status &= ~s;
}

const RoadSegment* DriverUpdateParams::nextLink()
{
	DriverMovement *driverMvt = (DriverMovement*)driver->Movement();
	return driverMvt->fwdDriverMovement.getNextSegment(false);
}
bool DriverUpdateParams::willYield(unsigned int reason)
{
	//TODO willYield
	return true;
}
double DriverUpdateParams::lcMinGap(int type)
{
	std::vector<double> b = LC_GAP_MODELS[type];
	return b[2] * b[0];
}

}
