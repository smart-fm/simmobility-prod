//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_map.hpp>
namespace sim_mob {
/**
 * Simple class to store information about a person from population database.
 * \note This class is used by the mid-term behavior models.
 *
 *
 * \author Harish Loganathan
 */
class PersonParams {
public:
	void initTimeWindows() {
		std::stringstream tw;
		for(double i=3.25; i<27.0; i=i+0.5) {
			for(double j=i; j<27.0; j=j+0.5) {
				tw << i << "," << j;
				timeWindowAvailability[tw.str()] = 1; //initialize availability of all time windows to 1
			}
		}
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

	/**
	 * get the availability for a time window
	 */
	int getTimeWindowAvailability(const std::string& timeWnd) {
		return timeWindowAvailability.at(timeWnd);
	}

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

    /**
     * Time windows available for the person as he plans his day.
     */
    boost::unordered_map<std::string, int> timeWindowAvailability;
};
}// end namespace sim_mob
