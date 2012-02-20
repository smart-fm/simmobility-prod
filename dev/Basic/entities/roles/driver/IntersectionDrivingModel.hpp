/* Copyright Singapore-MIT Alliance for Research and Technology */


#pragma once


#include "util/DynamicVector.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif

namespace sim_mob {

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * \author Seth N. Hetu
 */
class IntersectionDrivingModel {
public:
	virtual void startDriving(const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset) = 0;
	virtual DPoint continueDriving(double amount) = 0;
	virtual bool isDone() = 0;
	virtual double getCurrentAngle() = 0;

	//add by xuyan, do nothing
#ifndef SIMMOB_DISABLE_MPI
public:
	static void pack(PackageUtils& package, const IntersectionDrivingModel* params);
	static void unpack(UnPackageUtils& unpackage, IntersectionDrivingModel* params);
#endif
};

/**
 * \author Seth N. Hetu
 */
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

	//add by xuyan
#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const SimpleIntDrivingModel* params);

	static void unpack(UnPackageUtils& unpackage, SimpleIntDrivingModel* params);
#endif
};


}
