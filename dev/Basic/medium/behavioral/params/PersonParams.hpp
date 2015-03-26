//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <vector>
#include <bitset>
#include <stdint.h>
#include "behavioral/PredayClasses.hpp"

namespace sim_mob {
namespace medium {

/**
 * Simple class to store information about a person from population database.
 * \note This class is used by the mid-term behavior models.
 *
 *
 * \author Harish Loganathan
 */
class PersonParams {
public:
	PersonParams();
	virtual ~PersonParams();

	const std::string& getHhId() const
	{
		return hhId;
	}

	void setHhId(const std::string& hhId)
	{
		this->hhId = hhId;
	}

	int getAgeId() const {
		return ageId;
	}

	void setAgeId(int ageId) {
		this->ageId = ageId;
	}

	int getCarOwnNormal() const {
		return carOwnNormal;
	}

	void setCarOwnNormal(int carOwnNormal) {
		this->carOwnNormal = carOwnNormal;
	}

	int getCarOwnOffpeak() const {
		return carOwnOffpeak;
	}

	void setCarOwnOffpeak(int carOwnOffpeak) {
		this->carOwnOffpeak = carOwnOffpeak;
	}

	int getFixedWorkLocation() const {
		return fixedWorkLocation;
	}

	void setFixedWorkLocation(int fixedWorkLocation) {
		this->fixedWorkLocation = fixedWorkLocation;
	}

	int hasFixedWorkPlace() const {
		return (fixedWorkLocation != 0);
	}

	int getHasFixedWorkTiming() const {
		return hasFixedWorkTiming;
	}

	void setHasFixedWorkTiming(int hasFixedWorkTiming) {
		this->hasFixedWorkTiming = hasFixedWorkTiming;
	}

	int getHomeLocation() const {
		return homeLocation;
	}

	void setHomeLocation(int homeLocation) {
		this->homeLocation = homeLocation;
	}

	int getIncomeId() const {
		return incomeId;
	}

	void setIncomeId(int income_id) {
		this->incomeId = income_id;
	}

	int getIsFemale() const {
		return isFemale;
	}

	void setIsFemale(int isFemale) {
		this->isFemale = isFemale;
	}

	int getIsUniversityStudent() const {
		return isUniversityStudent;
	}

	void setIsUniversityStudent(int isUniversityStudent) {
		this->isUniversityStudent = isUniversityStudent;
	}

	int getMotorOwn() const {
		return motorOwn;
	}

	void setMotorOwn(int motorOwn) {
		this->motorOwn = motorOwn;
	}

	int getPersonTypeId() const {
		return personTypeId;
	}

	void setPersonTypeId(int personTypeId) {
		this->personTypeId = personTypeId;
	}

	int getWorksAtHome() const {
		return worksAtHome;
	}

	void setWorksAtHome(int worksAtHome) {
		this->worksAtHome = worksAtHome;
	}

	int getFixedSchoolLocation() const {
		return fixedSchoolLocation;
	}

	void setFixedSchoolLocation(int fixedSchoolLocation) {
		this->fixedSchoolLocation = fixedSchoolLocation;
	}

	int getStopType() const {
		return stopType;
	}

	void setStopType(int stopType) {
		this->stopType = stopType;
	}

	int isWorker() const {
		return (personTypeId == 1 || personTypeId == 2 || personTypeId == 3 || personTypeId == 8 || personTypeId == 9 || personTypeId == 10);
	}

	int hasDrivingLicence() const {
		return drivingLicence;
	}

	void setHasDrivingLicence(bool hasDrivingLicence) {
		this->drivingLicence = (int)hasDrivingLicence;
	}

	std::string getPersonId() const {
		return personId;
	}

	void setPersonId(std::string personId) {
		this->personId = personId;
	}

	int getHH_HasUnder15() const {
		return hasUnder15;
	}

	void setHH_HasUnder15(int hhUnder15) {
		this->hasUnder15 = (hhUnder15 > 0);
	}

	int getHH_NumUnder4() const {
		return hhNumUnder4;
	}

	void setHH_NumUnder4(int hhNumUnder4) {
		this->hhNumUnder4 = hhNumUnder4;
	}

	int getHH_OnlyAdults() const {
		return hhOnlyAdults;
	}

	void setHH_OnlyAdults(int hhOnlyAdults) {
		this->hhOnlyAdults = hhOnlyAdults;
	}

	int getHH_OnlyWorkers() const {
		return hhOnlyWorkers;
	}

	void setHH_OnlyWorkers(int hhOnlyWorkers) {
		this->hhOnlyWorkers = hhOnlyWorkers;
	}

	double getEduLogSum() const {
		return eduLogSum;
	}

	void setEduLogSum(double eduLogSum) {
		this->eduLogSum = eduLogSum;
	}

	double getOtherLogSum() const {
		return otherLogSum;
	}

	void setOtherLogSum(double otherLogSum) {
		this->otherLogSum = otherLogSum;
	}

	double getShopLogSum() const {
		return shopLogSum;
	}

	void setShopLogSum(double shopLogSum) {
		this->shopLogSum = shopLogSum;
	}

	double getWorkLogSum() const {
		return workLogSum;
	}

	void setWorkLogSum(double workLogSum) {
		this->workLogSum = workLogSum;
	}

	int getStudentTypeId() const {
		return studentTypeId;
	}

	void setStudentTypeId(int studentTypeId) {
		this->studentTypeId = studentTypeId;
	}

	double getHouseholdFactor() const
	{
		return householdFactor;
	}

	void setHouseholdFactor(double householdFactor)
	{
		this->householdFactor = householdFactor;
	}

	int getMissingIncome() const
	{
		return missingIncome;
	}

	void setMissingIncome(int missingIncome)
	{
		this->missingIncome = missingIncome;
	}

	int getCarOwn() const
	{
		return carOwn;
	}

	void setCarOwn(int carOwn)
	{
		this->carOwn = carOwn;
	}

	/** makes all time windows to available*/
	void initTimeWindows();

	/**
	 * get the availability for a time window for tour
	 */
	int getTimeWindowAvailability(size_t timeWnd) const;

	/**
	 * overload function to set availability of times in timeWnd to 0
	 *
	 * @param startTime start time
	 * @param endTime end time
	 */
	void blockTime(double startTime, double endTime);

	/**
	 * prints the fields of this object
	 */
	void print();

	double getDpsLogsum() const
	{
		return dpsLogsum;
	}

	void setDpsLogsum(double dpsLogsum)
	{
		this->dpsLogsum = dpsLogsum;
	}

	double getDptLogsum() const
	{
		return dptLogsum;
	}

	void setDptLogsum(double dptLogsum)
	{
		this->dptLogsum = dptLogsum;
	}

	double getDpbLogsum() const
	{
		return dpbLogsum;
	}

	void setDpbLogsum(double dpbLogsum)
	{
		this->dpbLogsum = dpbLogsum;
	}

	bool getCarLicense() const
	{
		return carLicense;
	}

	void setCarLicense(bool carLicense)
	{
		this->carLicense = carLicense;
	}

	int getHhSize() const
	{
		return hhSize;
	}

	void setHH_Size(int hhSize)
	{
		this->hhSize = hhSize;
	}

	bool getMotorLicense() const
	{
		return motorLicense;
	}

	void setMotorLicense(bool motorLicence)
	{
		this->motorLicense = motorLicence;
	}

	bool getVanbusLicense() const
	{
		return vanbusLicense;
	}

	void setVanbusLicense(bool vanbusLicense)
	{
		this->vanbusLicense = vanbusLicense;
	}

	int getGenderId() const
	{
		return genderId;
	}

	void setGenderId(int genderId)
	{
		this->genderId = genderId;
	}

	long getHomeAddressId() const
	{
		return homeAddressId;
	}

	void setHomeAddressId(long homeAddressId)
	{
		this->homeAddressId = homeAddressId;
	}

	long getActivityAddressId() const
	{
		return activityAddressId;
	}

	void setActivityAddressId(long activityAddressId)
	{
		this->activityAddressId = activityAddressId;
	}

	int getHH_NumAdults() const
	{
		return hhNumAdults;
	}

	void setHH_NumAdults(int hhNumAdults)
	{
		this->hhNumAdults = hhNumAdults;
	}

	int getHH_NumUnder15() const
	{
		return hhNumUnder15;
	}

	void setHH_NumUnder15(int hhNumUnder15)
	{
		this->hhNumUnder15 = hhNumUnder15;
	}

	int getHH_NumWorkers() const
	{
		return hhNumWorkers;
	}

	void setHH_NumWorkers(int hhNumWorkers)
	{
		this->hhNumWorkers = hhNumWorkers;
	}

	bool hasWorkplace() const
	{
		return fixedWorkplace;
	}

	void setHasWorkplace(bool hasFixedWorkplace)
	{
		this->fixedWorkplace = hasFixedWorkplace;
	}

	int isStudent() const {
		return student;
	}

	void setIsStudent(bool isStudent)
	{
		this->student = student;
	}

	static double* getIncomeCategoryLowerLimits()
	{
		return incomeCategoryLowerLimits;
	}

	static std::map<int, std::bitset<4> >& getVehicleCategoryLookup()
	{
		return vehicleCategoryLookup;
	}

	static std::map<long, int>& getAddressTazLookup()
	{
		return addressTazLookup;
	}

	int getTAZCodeForAddressId(long addressId);

	void setIncomeIdFromIncome(double income);
	void setVehicleOwnershipFromCategoryId(int vehicleCategoryId);

	/**
	 * function to infer params used in preday system of models from params read from LT population
	 */
	void fixUpForLtPerson();

private:
	std::string personId;
	std::string hhId;
	int personTypeId;
	int ageId;
	int isUniversityStudent;
	int studentTypeId;
	int genderId;
	int isFemale;
	int incomeId;
	int missingIncome;
	int worksAtHome;
	int carOwn;
	int carOwnNormal;
	int carOwnOffpeak;
	int motorOwn;
	int hasFixedWorkTiming;
	int homeLocation;
	long homeAddressId;
	int fixedWorkLocation;
	int fixedSchoolLocation;
	long activityAddressId;
	int stopType;
	int drivingLicence;
	bool carLicense;
	bool motorLicense;
	bool vanbusLicense;
	bool fixedWorkplace;
	bool student;

	//household related
	int hhSize;
	int hhNumAdults;
	int hhNumWorkers;
	int hhNumUnder4;
	int hhNumUnder15;
	int hhOnlyAdults;
	int hhOnlyWorkers;
	int hasUnder15;
	double householdFactor;

	//logsums
	double workLogSum;
	double eduLogSum;
	double shopLogSum;
	double otherLogSum;
	double dptLogsum;
	double dpsLogsum;
	double dpbLogsum;

	/**
	 * Time windows availability for the person.
	 */
    std::vector<sim_mob::medium::TimeWindowAvailability> timeWindowAvailability;

    /**
     * income category lookup containing lower limits of each category.
     * income category id for a specific income is the index of the greatest element lower than the income in this array
     * index 0 corresponds no income.
     */
    static double incomeCategoryLowerLimits[12];

    /**
     * vehicle category map of id->bitset<4> (4 bits representing 0-carOwn, 1-carOwnNormal, 2-carOwnOffPeak and 3-motorOwn bit for the id)
     */
    static std::map<int, std::bitset<4> > vehicleCategoryLookup;

    /**
     * address to taz map
     */
    static std::map<long, int> addressTazLookup;
};

/**
 * Class for storing person parameters related to usual work location model
 *
 * \author Harish Loganathan
 */
class UsualWorkParams {
public:
	int getFirstOfMultiple() const {
		return firstOfMultiple;
	}

	void setFirstOfMultiple(int firstOfMultiple) {
		this->firstOfMultiple = firstOfMultiple;
	}

	int getSubsequentOfMultiple() const {
		return subsequentOfMultiple;
	}

	void setSubsequentOfMultiple(int subsequentOfMultiple) {
		this->subsequentOfMultiple = subsequentOfMultiple;
	}

	double getWalkDistanceAm() const {
		return walkDistanceAM;
	}

	void setWalkDistanceAm(double walkDistanceAm) {
		walkDistanceAM = walkDistanceAm;
	}

	double getWalkDistancePm() const {
		return walkDistancePM;
	}

	void setWalkDistancePm(double walkDistancePm) {
		walkDistancePM = walkDistancePm;
	}

	double getZoneEmployment() const {
		return zoneEmployment;
	}

	void setZoneEmployment(double zoneEmployment) {
		this->zoneEmployment = zoneEmployment;
	}

private:
	int firstOfMultiple;
	int subsequentOfMultiple;
	double walkDistanceAM;
	double walkDistancePM;
	double zoneEmployment;
};

/**
 * Simple class to store information pertaining sub tour model
 * \note This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class SubTourParams {
public:
	SubTourParams(const Tour& tour);
	virtual ~SubTourParams();

	int isFirstOfMultipleTours() const
	{
		return firstOfMultipleTours;
	}

	void setFirstOfMultipleTours(bool firstOfMultipleTours)
	{
		this->firstOfMultipleTours = firstOfMultipleTours;
	}

	int isSubsequentOfMultipleTours() const
	{
		return subsequentOfMultipleTours;
	}

	void setSubsequentOfMultipleTours(bool subsequentOfMultipleTours)
	{
		this->subsequentOfMultipleTours = subsequentOfMultipleTours;
	}

	int getTourMode() const
	{
		return tourMode;
	}

	void setTourMode(int tourMode)
	{
		this->tourMode = tourMode;
	}

	int isUsualLocation() const
	{
		return usualLocation;
	}

	void setUsualLocation(bool usualLocation)
	{
		this->usualLocation = usualLocation;
	}

	int getSubTourPurpose() const
	{
		return subTourPurpose;
	}

	void setSubTourPurpose(StopType subTourpurpose)
	{
		this->subTourPurpose = subTourpurpose;
	}

	/**
	 * make time windows between startTime and endTime available
	 * @param startTime start time of available window
	 * @param endTime end time of available window
	 */
	void initTimeWindows(double startTime, double endTime);

	/**
	 * get the availability for a time window for sub-tour
	 */
	int getTimeWindowAvailability(size_t timeWnd) const;

	/**
	 * make time windows between startTime and endTime unavailable
	 *
	 * @param startTime start time
	 * @param endTime end time
	 */
	void blockTime(double startTime, double endTime);

	/**
	 * check if all time windows are unavailable
	 * @return true if all time windows are unavailable; false otherwise
	 */
	bool allWindowsUnavailable();

	bool isCbdDestZone() const
	{
		return cbdDestZone;
	}

	void setCbdDestZone(bool cbdDestZone)
	{
		this->cbdDestZone = cbdDestZone;
	}

	bool isCbdOrgZone() const
	{
		return cbdOrgZone;
	}

	void setCbdOrgZone(bool cbdOrgZone)
	{
		this->cbdOrgZone = cbdOrgZone;
	}

private:
	/**mode choice for parent tour*/
	int tourMode;
	/**parent tour is the first of many tours for person*/
	bool firstOfMultipleTours;
	/**parent tour is the 2+ of many tours for person*/
	bool subsequentOfMultipleTours;
	/**parent tour is to a usual location*/
	bool usualLocation;
	/**sub tour type*/
	StopType subTourPurpose;
	/** Time windows available for sub-tour.*/
	std::vector<sim_mob::medium::TimeWindowAvailability> timeWindowAvailability;
    /** bitset of availablilities for fast checking*/
    std::bitset<1176> availabilityBit;

	bool cbdOrgZone;
	bool cbdDestZone;
};

} //end namespace medium
}// end namespace sim_mob
