//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TourModeParams.hpp
 *
 *  Created on: Nov 29, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include "behavioral/PredayClasses.hpp"

namespace sim_mob {
namespace medium {

/**
 * Simple class to store information pertaining tour tour-mode models
 * \note This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class TourModeParams {
public:
	virtual ~TourModeParams() {}

	double getAvgTransfer() const {
		return avgTransfer;
	}

	void setAvgTransfer(double avgTransfer) {
		this->avgTransfer = avgTransfer;
	}

	int isCentralZone() const {
		return centralZone;
	}

	void setCentralZone(bool centralZone) {
		this->centralZone = centralZone;
	}

	double getCostCarErpFirst() const {
		return costCarERP_First;
	}

	void setCostCarErpFirst(double costCarErpFirst) {
		costCarERP_First = costCarErpFirst;
	}

	double getCostCarErpSecond() const {
		return costCarERP_Second;
	}

	void setCostCarErpSecond(double costCarErpSecond) {
		costCarERP_Second = costCarErpSecond;
	}

	double getCostCarOpFirst() const {
		return costCarOP_First;
	}

	void setCostCarOpFirst(double costCarOpFirst) {
		costCarOP_First = costCarOpFirst;
	}

	double getCostCarOpSecond() const {
		return costCarOP_Second;
	}

	void setCostCarOpSecond(double costCarOpSecond) {
		costCarOP_Second = costCarOpSecond;
	}

	double getCostCarParking() const {
		return costCarParking;
	}

	void setCostCarParking(double costCarParking) {
		this->costCarParking = costCarParking;
	}

	double getCostPublicFirst() const {
		return costPublicFirst;
	}

	void setCostPublicFirst(double costPublicFirst) {
		this->costPublicFirst = costPublicFirst;
	}

	double getCostPublicSecond() const {
		return costPublicSecond;
	}

	void setCostPublicSecond(double costPublicSecond) {
		this->costPublicSecond = costPublicSecond;
	}

	int isDrive1Available() const {
		return drive1Available;
	}

	void setDrive1Available(bool drive1Available) {
		this->drive1Available = drive1Available;
	}

	int isMotorAvailable() const {
		return motorAvailable;
	}

	void setMotorAvailable(bool motorAvailable) {
		this->motorAvailable = motorAvailable;
	}

	int isMrtAvailable() const {
		return mrtAvailable;
	}

	void setMrtAvailable(bool mrtAvailable) {
		this->mrtAvailable = mrtAvailable;
	}

	int isPrivateBusAvailable() const {
		return privateBusAvailable;
	}

	void setPrivateBusAvailable(bool privateBusAvailable) {
		this->privateBusAvailable = privateBusAvailable;
	}

	int isPublicBusAvailable() const {
		return publicBusAvailable;
	}

	void setPublicBusAvailable(bool publicBusAvailable) {
		this->publicBusAvailable = publicBusAvailable;
	}

	int isShare2Available() const {
		return share2Available;
	}

	void setShare2Available(bool share2Available) {
		this->share2Available = share2Available;
	}

	int isShare3Available() const {
		return share3Available;
	}

	void setShare3Available(bool share3Available) {
		this->share3Available = share3Available;
	}

	StopType getStopType() const {
		return stopType;
	}

	void setStopType(StopType stopType) {
		this->stopType = stopType;
	}

	int isTaxiAvailable() const {
		return taxiAvailable;
	}

	void setTaxiAvailable(bool taxiAvailable) {
		this->taxiAvailable = taxiAvailable;
	}

	double getTtCarIvtFirst() const {
		return ttCarIvtFirst;
	}

	void setTtCarIvtFirst(double ttCarIvtFirst) {
		this->ttCarIvtFirst = ttCarIvtFirst;
	}

	double getTtCarIvtSecond() const {
		return ttCarIvtSecond;
	}

	void setTtCarIvtSecond(double ttCarIvtSecond) {
		this->ttCarIvtSecond = ttCarIvtSecond;
	}

	double getTtPublicIvtFirst() const {
		return ttPublicIvtFirst;
	}

	void setTtPublicIvtFirst(double ttPublicIvtFirst) {
		this->ttPublicIvtFirst = ttPublicIvtFirst;
	}

	double getTtPublicIvtSecond() const {
		return ttPublicIvtSecond;
	}

	void setTtPublicIvtSecond(double ttPublicIvtSecond) {
		this->ttPublicIvtSecond = ttPublicIvtSecond;
	}

	double getTtPublicWaitingFirst() const {
		return ttPublicWaitingFirst;
	}

	void setTtPublicWaitingFirst(double ttPublicWaitingFirst) {
		this->ttPublicWaitingFirst = ttPublicWaitingFirst;
	}

	double getTtPublicWaitingSecond() const {
		return ttPublicWaitingSecond;
	}

	void setTtPublicWaitingSecond(double ttPublicWaitingSecond) {
		this->ttPublicWaitingSecond = ttPublicWaitingSecond;
	}

	double getTtPublicWalkFirst() const {
		return ttPublicWalkFirst;
	}

	void setTtPublicWalkFirst(double ttPublicWalkFirst) {
		this->ttPublicWalkFirst = ttPublicWalkFirst;
	}

	double getTtPublicWalkSecond() const {
		return ttPublicWalkSecond;
	}

	void setTtPublicWalkSecond(double ttPublicWalkSecond) {
		this->ttPublicWalkSecond = ttPublicWalkSecond;
	}

	int isWalkAvailable() const {
		return walkAvailable;
	}

	void setWalkAvailable(bool walkAvailable) {
		this->walkAvailable = walkAvailable;
	}

	double getWalkDistance1() const {
		return walkDistance1;
	}

	void setWalkDistance1(double walkDistance1) {
		this->walkDistance1 = walkDistance1;
	}

	double getWalkDistance2() const {
		return walkDistance2;
	}

	void setWalkDistance2(double walkDistance2) {
		this->walkDistance2 = walkDistance2;
	}

	double getDestinationArea() const {
		return destinationArea;
	}

	void setDestinationArea(double destinationArea) {
		this->destinationArea = destinationArea;
	}

	double getOriginArea() const {
		return originArea;
	}

	void setOriginArea(double originArea) {
		this->originArea = originArea;
	}

	double getResidentSize() const {
		return residentSize;
	}

	void setResidentSize(double residentSize) {
		this->residentSize = residentSize;
	}

	double getWorkOp() const {
		return workOP;
	}

	void setWorkOp(double workOp) {
		workOP = workOp;
	}

private:
	StopType stopType;
	double costPublicFirst;
	double costPublicSecond;
	double costCarERP_First;
	double costCarERP_Second;
	double costCarOP_First;
	double costCarOP_Second;
	double costCarParking;
	double walkDistance1;
	double walkDistance2;
	bool centralZone;
	double ttPublicIvtFirst;
	double ttPublicIvtSecond;
	double ttPublicWaitingFirst;
	double ttPublicWaitingSecond;
	double ttPublicWalkFirst;
	double ttPublicWalkSecond;
	double ttCarIvtFirst;
	double ttCarIvtSecond;
	double avgTransfer;
	double residentSize;
	double workOP;
	double originArea;
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

};
} //end namespace medium
} //end namespace sim_mob

