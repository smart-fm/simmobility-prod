//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ZoneParams.hpp
 *
 *  Created on: Nov 30, 2013
 *      Author: Harish Loganathan
 */
#pragma once
#include <string>

namespace sim_mob {
namespace medium {

class ZoneParams {
public:
	virtual ~ZoneParams() {}

	double getArea() const {
		return area;
	}

	void setArea(double area) {
		this->area = area;
	}

	int getCentralDummy() const {
		return centralZone;
	}

	void setCentralDummy(int centralDummy) {
		this->centralZone = centralDummy;
	}

	double getEmployment() const {
		return employment;
	}

	void setEmployment(double employment) {
		this->employment = employment;
	}

	double getParkingRate() const {
		return parkingRate;
	}

	void setParkingRate(double parkingRate) {
		this->parkingRate = parkingRate;
	}

	double getPopulation() const {
		return population;
	}

	void setPopulation(double population) {
		this->population = population;
	}

	double getResidentStudents() const {
		return residentStudents;
	}

	void setResidentStudents(double residentStudents) {
		this->residentStudents = residentStudents;
	}

	double getResidentWorkers() const {
		return residentWorkers;
	}

	void setResidentWorkers(double residentWorkers) {
		this->residentWorkers = residentWorkers;
	}

	double getShop() const {
		return shop;
	}

	void setShop(double shop) {
		this->shop = shop;
	}

	double getTotalEnrollment() const {
		return totalEnrollment;
	}

	void setTotalEnrollment(double totalEnrollment) {
		this->totalEnrollment = totalEnrollment;
	}

	int getZoneCode() const {
		return zoneCode;
	}

	void setZoneCode(int zoneCode) {
		this->zoneCode = zoneCode;
	}

	int getZoneId() const {
		return zoneId;
	}

	void setZoneId(int zoneId) {
		this->zoneId = zoneId;
	}

private:
	int zoneId;
	int zoneCode;
	double shop;
	double parkingRate;
	double residentWorkers;
	bool centralZone;
	double employment;
	double population;
	double area;
	double totalEnrollment;
	double residentStudents;
};

class CostParams {
public:
	virtual ~CostParams() {}

	const std::string getOrgDest() const {
		return orgDest;
	}

	void setOrgDest() {
		std::stringstream ss;
		ss << originZone << "," << destinationZone;
		orgDest = ss.str();
	}

	double getAvgTransfer() const {
		return avgTransfer;
	}

	void setAvgTransfer(double avgTransfer) {
		this->avgTransfer = avgTransfer;
	}

	double getCarCostErp() const {
		return carCostERP;
	}

	void setCarCostErp(double carCostErp) {
		carCostERP = carCostErp;
	}

	double getCarIvt() const {
		return carIvt;
	}

	void setCarIvt(double carIvt) {
		this->carIvt = carIvt;
	}

	int getDestinationZone() const {
		return destinationZone;
	}

	void setDestinationZone(int destinationZone) {
		this->destinationZone = destinationZone;
	}

	double getDistance() const {
		return distance;
	}

	void setDistance(double distance) {
		this->distance = distance;
	}

	int getOriginZone() const {
		return originZone;
	}

	void setOriginZone(int originZone) {
		this->originZone = originZone;
	}

	double getPubCost() const {
		return pubCost;
	}

	void setPubCost(double pubCost) {
		this->pubCost = pubCost;
	}

	double getPubIvt() const {
		return pubIvt;
	}

	void setPubIvt(double pubIvt) {
		this->pubIvt = pubIvt;
	}

	double getPubOut() const {
		return pubOut;
	}

	void setPubOut(double pubOut) {
		this->pubOut = pubOut;
	}

	double getPubWalkt() const {
		return pubWalkt;
	}

	void setPubWalkt(double pubWalkt) {
		this->pubWalkt = pubWalkt;
	}

	double getPubWtt() const {
		return pubWtt;
	}

	void setPubWtt(double pubWtt) {
		this->pubWtt = pubWtt;
	}

private:
	int originZone;
	int destinationZone;
	std::string orgDest;
	double pubWtt;
	double carIvt;
	double pubOut;
	double pubWalkt;
	double distance;
	double carCostERP;
	double pubIvt;
	double avgTransfer;
	double pubCost;
};
}
}
