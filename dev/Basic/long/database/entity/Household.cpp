//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Household.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on April 22, 2013, 5:54 PM
 */

#include "Household.hpp"
#include "util/Utils.hpp"
#include "database/dao/SqlAbstractDao.hpp"

using namespace sim_mob::long_term;

Household::Household( BigSerial id, BigSerial lifestyleId, BigSerial unitId, BigSerial ethnicityId, BigSerial vehicleCategoryId,  int size, int childUnder4, int childUnder15, int adult, double income,
					  int housingDuration,int workers, int ageOfHead, int pendingStatusId,std::tm pendingFromDate,int unitPending,bool twoRoomHdbEligibility, bool threeRoomHdbEligibility,
					  bool fourRoomHdbEligibility, int familyType, bool taxiAvailability,  int vehicleOwnershipOptionId, double logsum, double currentUnitPrice, double householdAffordabilityAmount,
					  int buySellInterval, std::tm moveInDate,int timeOnMarket,int timeOffMarket,int isBidder,int isSeller,int hasMoved, int tenureStatus, int awakenedDay, bool existInDB, int lastAwakenedDay,int lastBidStatus): id(id), lifestyleId(lifestyleId), unitId(unitId),
					  ethnicityId(ethnicityId), vehicleCategoryId(vehicleCategoryId),size(size), childUnder4(childUnder4), childUnder15(childUnder15), adult(adult),income(income),  housingDuration(housingDuration),
					  workers(workers), ageOfHead(ageOfHead), pendingStatusId(pendingStatusId),pendingFromDate(pendingFromDate),unitPending(unitPending),twoRoomHdbEligibility(twoRoomHdbEligibility),
					  threeRoomHdbEligibility(threeRoomHdbEligibility),  fourRoomHdbEligibility(fourRoomHdbEligibility),familyType(familyType), taxiAvailability(taxiAvailability),
					  vehicleOwnershipOptionId(vehicleOwnershipOptionId), logsum(logsum), currentUnitPrice(currentUnitPrice),  householdAffordabilityAmount(householdAffordabilityAmount), buySellInterval(buySellInterval),
					  moveInDate(moveInDate),  timeOnMarket(timeOnMarket),timeOffMarket(timeOffMarket),isBidder(isBidder),isSeller(isSeller),hasMoved(hasMoved), tenureStatus(tenureStatus), awakenedDay(awakenedDay), existInDB(existInDB), lastAwakenedDay(lastAwakenedDay),lastBidStatus(lastBidStatus){}

Household::Household(): id(0), lifestyleId(0), unitId(0), ethnicityId(0), vehicleCategoryId(0),size(0), childUnder4(0), childUnder15(0), adult(0),income(0), housingDuration(0), workers(0), ageOfHead(0),
						pendingStatusId(0),pendingFromDate(std::tm()),unitPending(0), twoRoomHdbEligibility(0), threeRoomHdbEligibility(0), fourRoomHdbEligibility(0), familyType(0),taxiAvailability(false),
						vehicleOwnershipOptionId(0), logsum(0),  currentUnitPrice(0),householdAffordabilityAmount(0),buySellInterval(0), moveInDate(std::tm()),timeOnMarket(0),timeOffMarket(0),isBidder(0),
						isSeller(0),hasMoved(0), tenureStatus(0), awakenedDay(0),existInDB(false), lastAwakenedDay(0),lastBidStatus(0){}


Household::~Household() {}

Household& Household::operator=(const Household& source)
{
    this->id = source.id;
    this->lifestyleId = source.lifestyleId;
    this->unitId = source.unitId;
    this->ethnicityId = source.ethnicityId;
    this->vehicleCategoryId = source.vehicleCategoryId;
    this->income = source.income;
    this->size = source.size;
    this->childUnder4 = source.childUnder15;
    this->childUnder15 = source.childUnder15;
    this->adult = source.adult;
    this->housingDuration = source.housingDuration;
    this->workers = source.workers;
    this->ageOfHead = source.ageOfHead;
    this->pendingStatusId = source.pendingStatusId;
    this->pendingFromDate = source.pendingFromDate;
    this->unitPending = source.unitPending;
    this->taxiAvailability = source.taxiAvailability;
    this->vehicleOwnershipOptionId = source.vehicleOwnershipOptionId;
    this->logsum = source.logsum;
    this->currentUnitPrice = source.currentUnitPrice;
    this->householdAffordabilityAmount = source.householdAffordabilityAmount;
    this->timeOnMarket = source.timeOnMarket;
    this->timeOffMarket = source.timeOffMarket;
    this->isBidder = source.isBidder;
    this->isSeller = source.isSeller;
    this->buySellInterval = source.buySellInterval;
    this->moveInDate = source.moveInDate;
    this->tenureStatus = source.tenureStatus;
    this->awakenedDay = source.awakenedDay;

    return *this;
}

void Household::setAgeOfHead(int ageOfHead)
{
    this->ageOfHead = ageOfHead;
}

int Household::getAgeOfHead() const
{
    return ageOfHead;
}

void Household::setWorkers(int workers)
{
    this->workers = workers;
}

int Household::getWorkers() const
{
    return workers;
}

const std::tm& Household::getPendingFromDate() const
{
	return pendingFromDate;
}

void Household::setPendingFromDate(const std::tm& pendingFromDate)
{
	this->pendingFromDate = pendingFromDate;
}

int Household::getPendingStatusId() const
{
	return pendingStatusId;
}

void Household::setPendingStatusId(int pendingStatusId)
{
	this->pendingStatusId = pendingStatusId;
}

int Household::getUnitPending() const
{
	return unitPending;
}

void Household::setUnitPending(int unitPending)
{
	this->unitPending = unitPending;
}

void Household::setHousingDuration(int housingDuration)
{
    this->housingDuration = housingDuration;
}

int Household::getHousingDuration() const
{
    return housingDuration;
}

void Household::setIncome(double income)
{
    this->income = income;
}

double Household::getIncome() const
{
    return income;
}

void Household::setChildUnder4(int childUnder4)
{
    this->childUnder4 = childUnder4;
}

void Household::setChildUnder15(int childUnder15)
{
    this->childUnder15 = childUnder15;
}

int Household::getChildUnder4() const
{
    return childUnder4;
}

int Household::getChildUnder15() const
{
    return childUnder15;
}

int Household::getAdult() const
{
	return adult;
}

void Household::setAdult(int adult)
{
	this->adult = adult;
}

void Household::setSize(int size)
{
    this->size = size;
}

int Household::getSize() const
{
    return size;
}

void Household::setVehicleCategoryId(BigSerial vehicleCategoryId)
{
    this->vehicleCategoryId = vehicleCategoryId;
}

BigSerial Household::getVehicleCategoryId() const
{
    return vehicleCategoryId;
}

void Household::setEthnicityId(BigSerial ethnicityId)
{
    this->ethnicityId = ethnicityId;
}

BigSerial Household::getEthnicityId() const
{
    return ethnicityId;
}

void Household::setUnitId(BigSerial unitId)
{
    this->unitId = unitId;
}

BigSerial Household::getUnitId() const
{
    return unitId;
}

void Household::setLifestyleId(BigSerial lifestyleId)
{
    this->lifestyleId = lifestyleId;
}

BigSerial Household::getLifestyleId() const
{
    return lifestyleId;
}

void Household::setId(BigSerial id)
{
    this->id = id;
}

BigSerial Household::getId() const
{
    return id;
}

std::vector<BigSerial> Household::getIndividuals() const
{
	return individuals;
}

void Household::setIndividual( BigSerial individual )
{
	individuals.push_back( individual );
}


bool  Household::getTwoRoomHdbEligibility() const
{
	return twoRoomHdbEligibility;
}

bool  Household::getThreeRoomHdbEligibility() const
{
	return threeRoomHdbEligibility;
}

bool  Household::getFourRoomHdbEligibility() const
{
	return fourRoomHdbEligibility;
}

void  Household::setTwoRoomHdbEligibility(bool eligibility)
{
	twoRoomHdbEligibility = true;
}

void  Household::setThreeRoomHdbEligibility(bool eligibility)
{
	threeRoomHdbEligibility = true;
}

void  Household::setFourRoomHdbEligibility(bool eligibility)
{
	fourRoomHdbEligibility = true;
}

void Household::setFamilyType( int type )
{
	familyType = type;
}

int Household::getFamilyType()
{
	return familyType;
}

bool Household::getTaxiAvailability() const
{
	return this->taxiAvailability;
}

void Household::setTaxiAvailability(bool taxiAvailable)
{
	taxiAvailability = taxiAvailable;
}

int Household::getVehicleOwnershipOptionId() const
{
	return this->vehicleOwnershipOptionId;
}

void Household::setVehicleOwnershipOptionId(int vehicleOwnershipOption)
{
	this->vehicleOwnershipOptionId = vehicleOwnershipOption;
}

void Household::setAffordabilityAmount(double value)
{
	householdAffordabilityAmount = value;
}

double Household::getAffordabilityAmount() const
{
	return householdAffordabilityAmount;
}

void Household::setLogsum(double value)
{
	logsum = value;
}

double Household::getLogsum() const
{
	return logsum;
}


void Household::setCurrentUnitPrice( double value)
{
	currentUnitPrice = value;
}
double	Household::getCurrentUnitPrice() const
{
	return currentUnitPrice;
}

int Household::getLastAwakenedDay()
{
	return lastAwakenedDay;
}

void Household::setLastAwakenedDay(int lastAwkenedDate)
{
	this->lastAwakenedDay = lastAwkenedDate;
}

int Household::getBuySellInterval() const
{
	return buySellInterval;
}

void Household::setBuySellInterval(int buyerSellerInterval)
{
	this->buySellInterval = buyerSellerInterval;
}

int Household::getIsBidder() const
{
	return isBidder;
}

void Household::setIsBidder(int isBidder)
{
	this->isBidder = isBidder;
}

int Household::getIsSeller() const
{
	return isSeller;
}

void Household::setIsSeller(int isSeller)
{
	this->isSeller = isSeller;
}

int Household::getTimeOffMarket() const
{
	return timeOffMarket;
}

void Household::setTimeOffMarket(int timeOffMarket)
{
	this->timeOffMarket = timeOffMarket;
}

int Household::getTimeOnMarket() const
{
	return timeOnMarket;
}

void Household::setTimeOnMarket(int timeOnMarket)
{
		this->timeOnMarket = timeOnMarket;
}

double Household::getHouseholdAffordabilityAmount() const
{
	return householdAffordabilityAmount;
}

void Household::setHouseholdAffordabilityAmount(double householdAffordabilityAmount)
{
	this->householdAffordabilityAmount = householdAffordabilityAmount;
}

const std::tm& Household::getMoveInDate() const
{
	return moveInDate;
}

void Household::setMoveInDate(const std::tm& moveInDate)
{
	this->moveInDate = moveInDate;
}

int Household::getHasMoved() const
{
	return this->hasMoved;
}

void Household::setHasMoved(int hasMove)
{
	this->hasMoved = hasMove;
}

int Household::getTenureStatus() const
{
	return tenureStatus;
}

void Household::setTenureStatus(int val)
{
	tenureStatus = val;
}

int Household::getAwaknedDay() const
{
	return this->awakenedDay;
}

void Household::setAwakenedDay(int awakenDay)
{
	this->awakenedDay = awakenDay;
}

bool Household::getExistInDB() const
{
	return this->existInDB;
}

void Household::setExistInDB(bool exist)
{
	this->existInDB = exist;
}

int Household::getLastBidStatus() const
{
	return lastBidStatus;
}

void Household::setLastBidStatus(int lastBidStatus)
{
	this->lastBidStatus = lastBidStatus;
}


void Household::setHouseholdStats(HouseholdStatistics stats)
{
	householdStats = stats;
}

HouseholdStatistics Household::getHouseholdStats()
{
	return householdStats;
}


namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const Household& data)
        {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"lifestyleId\":\"" << data.lifestyleId << "\","
                    << "\"unitId\":\"" << data.unitId << "\","
                    << "\"ethnicityId\":\"" << data.ethnicityId << "\","
                    << "\"vehicleCategoryId\":\"" << data.vehicleCategoryId << "\","
                    << "\"size\":\"" << data.size << "\","
                    << "\"childUnder4\":\"" << data.childUnder4 << "\","
                    << "\"childUnder15\":\"" << data.childUnder15 << "\","
                    << "\"income\":\"" << data.income << "\","
                    << "\"housingDuration\":\"" << data.housingDuration << "\","
                    << "\"workers\":\"" << data.workers << "\","
                    << "\"ageOfHead\":\"" << data.ageOfHead << "\""
                    << "\"taxiAvailability\":\"" << data.taxiAvailability << "\","
                    <<"\"vehicleOwnershipOptionId\":\"" << data.vehicleOwnershipOptionId << "\","
					<<"\"currentUnitPrice\":\"" << data.currentUnitPrice << "\""
					<<"\"tenureStatus\":\"" << data.tenureStatus << "\""
                    << "}";
        }
    }
}
