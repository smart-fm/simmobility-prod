//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file Vehicle.hpp
 *
 * \author Li Zhemin
 * \author Seth N. Hetu
 * \author Zhang Shuai
 * \author Xu Yan
 */

#pragma once

#include <stdexcept>
#include <sstream>
#include <vector>

#include "conf/settings/DisableMPI.h"
#include "entities/vehicle/VehicleBase.hpp"
#include "entities/models/Constants.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/WayPoint.hpp"
#include "util/MovementVector.hpp"
#include "util/DynamicVector.hpp"

namespace sim_mob
{

class PackageUtils;
class UnPackageUtils;

/**
 * The Vehicle class has vehicle Id, position, forward velocity, lat velocity and acceleration parameters etc for Driver use
 * Each Driver object has a vehicle to move in the network
 **/
class Vehicle : public VehicleBase
{
private:
	/**Id of the vehicle - currently not used*/
	int vehicleId;
	
	/**
	 * Lateral movement of the vehicle (in metre). This value represents how far to the LEFT of the current lane 
	 * the vehicle has moved. Note: This is not the same as lateral position
	 */
	double latMovement;
	
	/**Forward velocity of the vehicle (m/s)*/
	double forwardVelocity;
	
	/**Lateral velocity of the vehicle (m/s)*/
	double lateralVelocity;
	
	/**The forward acceleration of the vehicle (m/s)*/
	double forwardAcceleration;
	
	/**The lane the vehicle is changing to - left, right or same*/
	LaneChangeTo turningDirection;
	
	/**Current position of the vehicle*/
	Point currPos;

	/**The name of the vehicle as defined in the configuration file*/
	std::string vehicleName;

public:
	Vehicle(const VehicleType vehType, double lengthM, double widthM, const std::string& vehName);
	Vehicle(const VehicleType vehType, int vehicleId, double lengthM, double widthM, const std::string& vehName);
	virtual ~Vehicle();

	double getLateralMovement() const;
	void moveLat(double amt);
	void resetLateralMovement();	
	
	double getVelocity() const;
	void setVelocity(double value);
	
	double getLateralVelocity() const;
	void setLateralVelocity(double value);
	
	double getAcceleration() const;
	void setAcceleration(double value);

	LaneChangeTo getTurningDirection() const;
	void setTurningDirection(LaneChangeTo direction);
	
	const Point& getCurrPosition() const;
	void setCurrPosition(Point currPosition);

	const std::string& getVehicleName() const;	

#ifndef SIMMOB_DISABLE_MPI
	//Serialisation-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;
	
	///Serialisation
	static void pack(PackageUtils& package, Vehicle* one_vehicle);
	static Vehicle* unpack(UnPackageUtils& unpackage);
#endif
};

}
