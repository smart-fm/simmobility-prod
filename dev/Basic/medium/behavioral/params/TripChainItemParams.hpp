//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TripChainParams.hpp
 *
 *  Created on: Mar 17, 2014
 *      Author: harish
 */

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
 * Simple class to store information pertaining to a trip chain item
 * (trip or an activity)
 * This class is used as the model for TripChainSqlDao to write trip chains to
 * the database
 *
 * \author Harish Loganathan
 */
class TripChainItemParams {
public:
	const std::string& getActivityEndTime() const {
		return activityEndTime;
	}

	void setActivityEndTime(const std::string& activityEndTime) {
		this->activityEndTime = activityEndTime;
	}

	const std::string& getActivityId() const {
		return activityId;
	}

	void setActivityId(const std::string& activityId) {
		this->activityId = activityId;
	}

	int getActivityLocation() const {
		return activityLocation;
	}

	void setActivityLocation(int activityLocation) {
		this->activityLocation = activityLocation;
	}

	const std::string& getActivityStartTime() const {
		return activityStartTime;
	}

	void setActivityStartTime(const std::string& activityStartTime) {
		this->activityStartTime = activityStartTime;
	}

	const std::string& getActivityType() const {
		return activityType;
	}

	void setActivityType(const std::string& activityType) {
		this->activityType = activityType;
	}

	bool isIsPrimaryActivity() const {
		return isPrimaryActivity;
	}

	void setIsPrimaryActivity(bool isPrimaryActivity) {
		this->isPrimaryActivity = isPrimaryActivity;
	}

	bool isIsPrimaryMode() const {
		return isPrimaryMode;
	}

	void setIsPrimaryMode(bool isPrimaryMode) {
		this->isPrimaryMode = isPrimaryMode;
	}

	const std::string& getPersonId() const {
		return personId;
	}

	void setPersonId(const std::string& personId) {
		this->personId = personId;
	}

	const std::string& getStartTime() const {
		return startTime;
	}

	void setStartTime(const std::string& startTime) {
		this->startTime = startTime;
	}

	int getSubtripDestination() const {
		return subtripDestination;
	}

	void setSubtripDestination(int subtripDestination) {
		this->subtripDestination = subtripDestination;
	}

	const std::string& getSubtripId() const {
		return subtripId;
	}

	void setSubtripId(const std::string& subtripId) {
		this->subtripId = subtripId;
	}

	const std::string& getSubtripMode() const {
		return subtripMode;
	}

	void setSubtripMode(const std::string& subtripMode) {
		this->subtripMode = subtripMode;
	}

	int getSubtripOrigin() const {
		return subtripOrigin;
	}

	void setSubtripOrigin(int subtripOrigin) {
		this->subtripOrigin = subtripOrigin;
	}

	const std::string& getTcItemType() const {
		return tcItemType;
	}

	void setTcItemType(const std::string& tcItemType) {
		this->tcItemType = tcItemType;
	}

	int getTcSeqNum() const {
		return tcSeqNum;
	}

	void setTcSeqNum(int tcSeqNum) {
		this->tcSeqNum = tcSeqNum;
	}

	int getTripDestination() const {
		return tripDestination;
	}

	void setTripDestination(int tripDestination) {
		this->tripDestination = tripDestination;
	}

	const std::string& getTripId() const {
		return tripId;
	}

	void setTripId(const std::string& tripId) {
		this->tripId = tripId;
	}

	int getTripOrigin() const {
		return tripOrigin;
	}

	void setTripOrigin(int tripOrigin) {
		this->tripOrigin = tripOrigin;
	}

private:
	std::string personId;
	int tcSeqNum;
	std::string tcItemType;
	std::string tripId;
	int tripOrigin;
	int tripDestination;
	std::string subtripId;
	int subtripOrigin;
	int subtripDestination;
	std::string subtripMode;
	bool isPrimaryMode;
	std::string startTime;
	std::string activityId;
	std::string activityType;
	bool isPrimaryActivity;
	int activityLocation;
	std::string activityStartTime;
	std::string activityEndTime;
};
} //end namespace medium
} //end namespace sim_mob


