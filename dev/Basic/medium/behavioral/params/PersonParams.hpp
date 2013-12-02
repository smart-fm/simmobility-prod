//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <vector>
#include <boost/unordered_map.hpp>
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

	int getFixedWorkLocation() const {
		return fixedWorkLocation;
	}

	void setFixedWorkLocation(int fixedWorkLocation) {
		this->fixedWorkLocation = fixedWorkLocation;
	}

	int getFixedWorkPlace() const {
		return (fixedWorkLocation == 0);
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

	void setIncomeId(double income) {
		if(income == 0.0) {
			incomeId = 12;
		}
		if(income < 1000.0) {
			incomeId = 1;
		}
		else if(income < 1500.0) {
			incomeId = 2;
		}
		else if(income < 2000.0) {
			incomeId = 3;
		}
		else if(income < 2500.0) {
			incomeId = 4;
		}
		else if(income < 3000.0) {
			incomeId = 5;
		}
		else if(income < 4000.0) {
			incomeId = 6;
		}
		else if(income < 5000.0) {
			incomeId = 7;
		}
		else if(income < 6000.0) {
			incomeId = 8;
		}
		else if(income < 7000.0) {
			incomeId = 9;
		}
		else if(income < 8000.0) {
			incomeId = 10;
		}
		else if(income >= 8000) {
			incomeId = 11;
		}
		else {
			// not sure what value of income should be mapped to ids 13 and 14.
			incomeId = 13;
		}
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

	int isStudent() const {
		return (personTypeId == 4);
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

	/**
	 * get the availability for a time window for tour
	 */
	int getTimeWindowAvailability(std::string& timeWnd) const;

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

private:
	std::string personId;
	int personTypeId;
	int ageId;
	int isUniversityStudent;
	int studentTypeId;
	int isFemale;
	int incomeId;
	int worksAtHome;
	int carOwnNormal;
	int carOwnOffpeak;
	int motorOwn;
	int hasFixedWorkTiming;
	int homeLocation;
	int fixedWorkLocation;
	int fixedSchoolLocation;
	int stopType;
	int drivingLicence;

	int hhOnlyAdults;
	int hhOnlyWorkers;
	int hhNumUnder4;
	int hasUnder15;

	double workLogSum;
	double eduLogSum;
	double shopLogSum;
	double otherLogSum;

	/**
	 * Time windows currently available for the person.
	 */
    boost::unordered_map<std::string, sim_mob::medium::TimeWindowAvailability*> timeWindowAvailability;
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

} //end namespace medium
}// end namespace sim_mob
