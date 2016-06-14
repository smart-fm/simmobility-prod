//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "behavioral/StopType.hpp"

namespace sim_mob
{
class WithindayModelsHelper;
/**
 * Simple class to store information pertinent to withinday-mode choice model
 *
 * \author Harish Loganathan
 */
class WithindayModeParams
{
private:
	StopType tripType;
	double costCarParking;
	bool centralZone;
	double ttPublicInVehicle;
	double ttPublicWaiting;
	double ttPublicWalk;
	double ttCarInVehicle;
	double avgTransfer;
	double originArea;
	double originResidentSize;
	double destinationWorkerSize;
	double destinationStudentsSize;
	double destinationPopulation;
	double destinationShops;
	double destinationArea;
	double distance;

	bool publicBusAvailable;
	bool mrtAvailable;
	bool privateBusAvailable;
	bool drive1Available;
	bool share2Available;
	bool share3Available;
	bool motorAvailable;
	bool walkAvailable;
	bool taxiAvailable;

	friend class WithindayModelsHelper;

public:
	WithindayModeParams()
		: tripType(NULL_STOP), costCarParking(0), centralZone(false), ttPublicInVehicle(0),
		  ttPublicWaiting(0), ttPublicWalk(0), ttCarInVehicle(0), avgTransfer(0), originArea(0), originResidentSize(0),
		  destinationWorkerSize(0), destinationStudentsSize(0), destinationArea(0), destinationShops(0), distance(0),
		  destinationPopulation(0),
		  //all modes are available by default
		  publicBusAvailable(true), mrtAvailable(true), privateBusAvailable(true), drive1Available(true),
		  share2Available(true), share3Available(true), motorAvailable(true), walkAvailable(true), taxiAvailable(true)
	{}

	virtual ~WithindayModeParams() {}

	double getAvgTransfer() const
	{
		return avgTransfer;
	}

	void setAvgTransfer(double avgTransfer)
	{
		this->avgTransfer = avgTransfer;
	}

	int isCentralZone() const
	{
		return (centralZone? 1 : 0);
	}

	void setCentralZone(bool centralZone)
	{
		this->centralZone = centralZone;
	}

	double getCostCarParking() const
	{
		return costCarParking;
	}

	void setCostCarParking(double costCarParking)
	{
		this->costCarParking = costCarParking;
	}

	double getDestinationArea() const
	{
		return destinationArea;
	}

	void setDestinationArea(double destinationArea)
	{
		this->destinationArea = destinationArea;
	}

	double getDestinationStudentsSize() const
	{
		return destinationStudentsSize;
	}

	void setDestinationStudentsSize(double destinationStudentsSize)
	{
		this->destinationStudentsSize = destinationStudentsSize;
	}

	double getDestinationWorkerSize() const
	{
		return destinationWorkerSize;
	}

	void setDestinationWorkerSize(double destinationWorkerSize)
	{
		this->destinationWorkerSize = destinationWorkerSize;
	}

	int isDrive1Available() const
	{
		return drive1Available;
	}

	void unsetDrive1Availability()
	{
		this->drive1Available = false;
	}

	int isMotorAvailable() const
	{
		return motorAvailable;
	}

	void unsetMotorAvailability()
	{
		this->motorAvailable = false;
	}

	int isMrtAvailable() const
	{
		return mrtAvailable;
	}

	void unsetMrtAvailability()
	{
		this->mrtAvailable = false;
	}

	double getOriginArea() const
	{
		return originArea;
	}

	void setOriginArea(double originArea)
	{
		this->originArea = originArea;
	}

	double getOriginResidentSize() const
	{
		return originResidentSize;
	}

	void setOriginResidentSize(double originResidentSize)
	{
		this->originResidentSize = originResidentSize;
	}

	int isPrivateBusAvailable() const
	{
		return privateBusAvailable;
	}

	void unsetPrivateBusAvailability()
	{
		this->privateBusAvailable = false;
	}

	int isPublicBusAvailable() const
	{
		return publicBusAvailable;
	}

	void unsetPublicBusAvailability()
	{
		this->publicBusAvailable = false;
	}

	int isShare2Available() const
	{
		return share2Available;
	}

	void unsetShare2Availability()
	{
		this->share2Available = false;
	}

	int isShare3Available() const
	{
		return share3Available;
	}

	void unsetShare3Availability()
	{
		this->share3Available = false;
	}

	int isTaxiAvailable() const
	{
		return taxiAvailable;
	}

	void unsetTaxiAvailability()
	{
		this->taxiAvailable = false;
	}

	double getTtCarInVehicle() const
	{
		return ttCarInVehicle;
	}

	void setTtCarInVehicle(double ttCarInVehicle)
	{
		this->ttCarInVehicle = ttCarInVehicle;
	}

	double getTtPublicInVehicle() const
	{
		return ttPublicInVehicle;
	}

	void setTtPublicInVehicle(double ttPublicInVehicle)
	{
		this->ttPublicInVehicle = ttPublicInVehicle;
	}

	double getTtPublicWaiting() const
	{
		return ttPublicWaiting;
	}

	void setTtPublicWaiting(double ttPublicWaiting)
	{
		this->ttPublicWaiting = ttPublicWaiting;
	}

	double getTtPublicWalk() const
	{
		return ttPublicWalk;
	}

	void setTtPublicWalk(double ttPublicWalk)
	{
		this->ttPublicWalk = ttPublicWalk;
	}

	int isWalkAvailable() const
	{
		return walkAvailable;
	}

	void unsetWalkAvailability()
	{
		this->walkAvailable = false;
	}

	StopType getTripType() const
	{
		return tripType;
	}

	void setTripType(StopType tripType)
	{
		this->tripType = tripType;
	}

	double getDistance() const
	{
		return distance;
	}

	void setDistance(double distance)
	{
		this->distance = distance;
	}

	double getDestinationShops() const
	{
		return destinationShops;
	}

	void setDestinationShops(double destinationShops)
	{
		this->destinationShops = destinationShops;
	}

	double getDestinationPopulation() const
	{
		return destinationPopulation;
	}

	void setDestinationPopulation(double destinationPopulation)
	{
		this->destinationPopulation = destinationPopulation;
	}
};
}
