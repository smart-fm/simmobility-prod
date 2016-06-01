//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "conf/params/ParameterManager.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/roles/driver/DriverUpdateParams.hpp"
#include "geospatial/network/TurningPath.hpp"
#include "util/DynamicVector.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif

namespace sim_mob
{

class TurningPath;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**Defines the types of intersection driving models supported*/
enum IntModelType
{
	/**Indicates that the object is of the simple intersection driving model class*/
	Int_Model_Simple = 0,

	/**Indicates that the object is of the MITSIM intersection driving model class*/
	Int_Model_MITSIM = 1,

	/**Indicates that the object is of the slot based intersection driving model class*/
	Int_Model_SlotBased = 2
};

/**Abstract class which describes driving behaviour within an intersection (without signals)*/
class IntersectionDrivingModel
{
protected:
	/**Indicates the type of model being used*/
	IntModelType modelType;

	/**Distance covered within the intersection*/
	double distCoveredOnTurning;

	/**The current turning path*/
	const TurningPath *currTurning;

public:
	IntersectionDrivingModel() :
	modelType(Int_Model_Simple), distCoveredOnTurning(0), currTurning(NULL)
	{
	}

	virtual ~IntersectionDrivingModel()
	{
	}

	double getDistCoveredOnTurning()
	{
		return distCoveredOnTurning;
	}

	void setCurrTurning(const TurningPath* currTurning)
	{
		this->currTurning = currTurning;
	}

	IntModelType getIntModelType() const
	{
		return modelType;
	}

	/**
	 * Calculates the acceleration for driving within the intersection
	 *
     * @param params driver parameters
	 *
     * @return acceleration
     */
	virtual double makeAcceleratingDecision(DriverUpdateParams& params) = 0;
};

/*
 * MITSIMLab version of the intersection driving model
 */
class MITSIM_IntDriving_Model : public IntersectionDrivingModel
{
private:
	/**The minimum value of the attentiveness factor within an intersection. The factor is a random value between the given min and max*/
	double intersectionAttentivenessFactorMin;

	/**The maximum value of the attentiveness factor within an intersection. The factor is a random value between the given min and max*/
	double intersectionAttentivenessFactorMax;

	/**The value of minimum gap*/
	double minimumGap;

	/**Contains the mean and standard deviation values for generating a random add-on to the critical gap*/
	double criticalGapAddOn[2];

	/**The multiplier for the impatience timer.*/
	double impatienceFactor;
	
	/**
	 * Reads and stores the parameters related to intersection driving from the driver parameter XML file
	 *
     * @param params driver parameters
     */
	void readDriverParameters(DriverUpdateParams &params);

	/**
	 * Calculate the acceleration needed to crawl
	 *
     * @param distance the stopping distance i.e. the distance at which the nearest obstacle is present
     * @param params driver parameters
	 *
     * @return acceleration
     */
	double calcCrawlingAcc(double distance, DriverUpdateParams& params);

protected:
	/**
	 * Calculates the acceleration rate required to accelerate / decelerate from current speed to a full
	 * stop within a given distance
     *
	 * @param params driver parameters
     * @param distance distance within which we need to stop
     *
	 * @return acceleration
     */
	double calcBrakeToStopAcc(double distance, DriverUpdateParams &params);

	/**
	 * Calculates the time required to reach the given distance
	 *
     * @param distance the distance to be covered
     * @param params driver parameters
	 *
     * @return time (in seconds) required to reach the given distance
     */
	double calcArrivalTime(double distance, DriverUpdateParams& params);

public:
	MITSIM_IntDriving_Model();
	MITSIM_IntDriving_Model(DriverUpdateParams &params);
	virtual ~MITSIM_IntDriving_Model();

	double getIntersectionAttentivenessFactorMin() const;
	double getIntersectionAttentivenessFactorMax() const;

	double getImpatienceFactor() const;

	/**
	 * Scans the conflicting vehicles  in the path and calculates the acceleration that allows the vehicle to
	 * pass through without colliding with the other vehicles
     *
	 * @param params driver parameters
     *
	 * @return acceleration
     */
	virtual double makeAcceleratingDecision(DriverUpdateParams &params);
};

/*
 * Slot-based intersection driving model
 */
class SlotBased_IntDriving_Model : public MITSIM_IntDriving_Model
{
private:
	/**Indicates whether a request has been sent to the intersection manager for access to the intersection*/
	bool isRequestSent;

public:
	SlotBased_IntDriving_Model();
	virtual ~SlotBased_IntDriving_Model();

	/**
	 * Calculates the acceleration required to reach the intersection at the allocated time so as to
	 * avoid collisions with conflicting vehicles
     *
	 * @param params driver parameters
	 *
     * @return acceleration
     */
	virtual double makeAcceleratingDecision(DriverUpdateParams &params);

	/**
	 * Sends the intersection access request to the intersection manager
	 *
     * @param params driver parameters
     */
	void sendAccessRequest(DriverUpdateParams &params);
};

}
