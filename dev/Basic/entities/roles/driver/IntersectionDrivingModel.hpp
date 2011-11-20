/* Copyright Singapore-MIT Alliance for Research and Technology */


#pragma once


#include "util/DynamicVector.hpp"


namespace sim_mob {


class IntersectionDrivingModel {
public:
	virtual void startDriving(const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset) = 0;
	virtual DPoint continueDriving(double amount) = 0;
	virtual bool isDone() = 0;
	virtual double getCurrentAngle() = 0;
};

class SimpleIntDrivingModel : public IntersectionDrivingModel {
private:
	DynamicVector intTrajectory;
	double totalMovement;

public:
	virtual void startDriving(const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset) {
		intTrajectory = DynamicVector(fromLanePt.x, fromLanePt.y, toLanePt.x, toLanePt.y);
		totalMovement = startOffset;
	}


	virtual DPoint continueDriving(double amount) {
		totalMovement += amount;
		DynamicVector temp(intTrajectory);
		temp.scaleVectTo(totalMovement).translateVect();
		return DPoint(temp.getX(), temp.getY());
	}


	virtual double getCurrentAngle() { return intTrajectory.getAngle(); }
	virtual bool isDone() { return totalMovement >= intTrajectory.getMagnitude(); }
};


}
