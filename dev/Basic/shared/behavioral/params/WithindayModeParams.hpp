//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "behavioral/StopType.hpp"

namespace sim_mob
{
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
	double walkDistance;
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
	double destinationArea;

	bool publicBusAvailable;
	bool mrtAvailable;
	bool privateBusAvailable;
	bool drive1Available;
	bool share2Available;
	bool share3Available;
	bool motorAvailable;
	bool walkAvailable;
	bool taxiAvailable;

public:
	WithindayModeParams()
		: tripType(NULL_STOP), costCarParking(0), walkDistance(0), centralZone(false), ttPublicInVehicle(0),
		  ttPublicWaiting(0), ttPublicWalk(0), ttCarInVehicle(0), avgTransfer(0), originArea(0), originResidentSize(0),
		  destinationWorkerSize(0), destinationStudentsSize(0), destinationArea(0), publicBusAvailable(false), mrtAvailable(false),
		  privateBusAvailable(false), drive1Available(false), share2Available(false), share3Available(false), motorAvailable(false),
		  walkAvailable(false), taxiAvailable(false)
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

	bool isCentralZone() const
	{
		return centralZone;
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

	bool isDrive1Available() const
	{
		return drive1Available;
	}

	void setDrive1Available(bool drive1Available)
	{
		this->drive1Available = drive1Available;
	}

	bool isMotorAvailable() const
	{
		return motorAvailable;
	}

	void setMotorAvailable(bool motorAvailable)
	{
		this->motorAvailable = motorAvailable;
	}

	bool isMrtAvailable() const
	{
		return mrtAvailable;
	}

	void setMrtAvailable(bool mrtAvailable)
	{
		this->mrtAvailable = mrtAvailable;
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

	bool isPrivateBusAvailable() const
	{
		return privateBusAvailable;
	}

	void setPrivateBusAvailable(bool privateBusAvailable)
	{
		this->privateBusAvailable = privateBusAvailable;
	}

	bool isPublicBusAvailable() const
	{
		return publicBusAvailable;
	}

	void setPublicBusAvailable(bool publicBusAvailable)
	{
		this->publicBusAvailable = publicBusAvailable;
	}

	bool isShare2Available() const
	{
		return share2Available;
	}

	void setShare2Available(bool share2Available)
	{
		this->share2Available = share2Available;
	}

	bool isShare3Available() const
	{
		return share3Available;
	}

	void setShare3Available(bool share3Available)
	{
		this->share3Available = share3Available;
	}

	bool isTaxiAvailable() const
	{
		return taxiAvailable;
	}

	void setTaxiAvailable(bool taxiAvailable)
	{
		this->taxiAvailable = taxiAvailable;
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

	bool isWalkAvailable() const
	{
		return walkAvailable;
	}

	void setWalkAvailable(bool walkAvailable)
	{
		this->walkAvailable = walkAvailable;
	}

	double getWalkDistance() const
	{
		return walkDistance;
	}

	void setWalkDistance(double walkDistance)
	{
		this->walkDistance = walkDistance;
	}

	StopType getTripType() const
	{
		return tripType;
	}

	void setTripType(StopType tripType)
	{
		this->tripType = tripType;
	}
};
}
