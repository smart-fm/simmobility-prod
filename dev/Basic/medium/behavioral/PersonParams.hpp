//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <vector>
#include <boost/unordered_map.hpp>

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
	virtual ~PersonParams();

	void initTimeWindows();

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

	long getFixedWorkLocation() const {
		return fixedWorkLocation;
	}

	void setFixedWorkLocation(long fixedWorkLocation) {
		this->fixedWorkLocation = fixedWorkLocation;
	}

	int getHasFlexibleWorkTiming() const {
		return hasFlexibleWorkTiming;
	}

	void setHasFlexibleWorkTiming(int hasFlexibleWorkTiming) {
		this->hasFlexibleWorkTiming = hasFlexibleWorkTiming;
	}

	long getHomeLocation() const {
		return homeLocation;
	}

	void setHomeLocation(long homeLocation) {
		this->homeLocation = homeLocation;
	}

	double getIncomeId() const {
		return incomeId;
	}

	void setIncomeId(double incomeId) {
		this->incomeId = incomeId;
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

	long getFixedSchoolLocation() const {
		return fixedSchoolLocation;
	}

	void setFixedSchoolLocation(long fixedSchoolLocation) {
		this->fixedSchoolLocation = fixedSchoolLocation;
	}

	int getStopType() const {
		return stopType;
	}

	void setStopType(int stopType) {
		this->stopType = stopType;
	}

	bool isStudent() {
		return personTypeId == 4;
	}

	int hasDrivingLicence() const {
		return drivingLicence;
	}

	void setHasDrivingLicence(bool hasDrivingLicence) {
		this->drivingLicence = (int)hasDrivingLicence;
	}

	/**
	 * get the availability for a time window
	 */
	int getTimeWindowAvailability(std::string& timeWnd);

	/**
	 * set availability of times in timeWnd to 0
	 *
	 * @param timeWnd "<startTime>,<endTime>" to block
	 */
	void blockTime(std::string& timeWnd);

	/**
	 * overload function to set availability of times in timeWnd to 0
	 *
	 * @param startTime start time
	 * @param endTime end time
	 */
	void blockTime(double startTime, double endTime);


private:
	int personTypeId;
	int ageId;
	int isUniversityStudent;
	int isFemale;
	double incomeId;
	int worksAtHome;
	int carOwnNormal;
	int carOwnOffpeak;
	int motorOwn;
	int hasFlexibleWorkTiming;
	long homeLocation;
	long fixedWorkLocation;
	long fixedSchoolLocation;
	int stopType;
	int drivingLicence;

	/**
	 * Time windows currently available for the person.
	 */
    boost::unordered_map<std::string, sim_mob::medium::TimeWindowAvailability*> timeWindowAvailability;

};
} //end namespace medium
}// end namespace sim_mob
