//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#include"models/IntersectionDrivingModel.hpp"

using namespace std;
using namespace sim_mob;

MITSIM_IntDriving_Model::MITSIM_IntDriving_Model()
{
	
}

MITSIM_IntDriving_Model::~MITSIM_IntDriving_Model()
{
	
}

DPoint MITSIM_IntDriving_Model::continueDriving(double amount)
{
	totalMovement += amount;
	DynamicVector temp (intTrajectory);
	temp.scaleVectTo (totalMovement).translateVect ();
	return DPoint (temp.getX (), temp.getY ());
}

double MITSIM_IntDriving_Model::getCurrentAngle()
{
	return intTrajectory.getAngle ();
}

bool MITSIM_IntDriving_Model::isDone()
{
	return totalMovement >= intTrajectory.getMagnitude ();
}

void MITSIM_IntDriving_Model::startDriving(const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset)
{
	intTrajectory = DynamicVector (fromLanePt.x, fromLanePt.y, toLanePt.x, toLanePt.y);
	totalMovement = startOffset;
}