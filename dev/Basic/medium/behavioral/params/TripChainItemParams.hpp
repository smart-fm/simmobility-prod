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
#include <stdint.h> //<cstdint> belongs to C++11!
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
	TripChainItemParams() :
		personId(std::string()),tcSeqNum(0), tcItemType("UNKNOWN"), tripId("0"), subtripId("0"), primaryMode(false),
		tripOrigin(0), tripDestination(0), subtripOrigin(0), subtripDestination(0), activityId("0"), activityType("dummy"),
		activityLocation(0), primaryActivity(false)
	{}

	TripChainItemParams(const std::string& personId, const std::string& tcItemType, uint8_t seqNo) :
		personId(personId),tcSeqNum(seqNo), tcItemType(tcItemType), tripId("0"), subtripId("0"), primaryMode(false),
		tripOrigin(0), tripDestination(0), subtripOrigin(0), subtripDestination(0), activityId("0"), activityType("dummy"),
		activityLocation(0), primaryActivity(false)
	{}

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

	bool isPrimaryActivity() const {
		return primaryActivity;
	}

	void setPrimaryActivity(bool isPrimaryActivity) {
		this->primaryActivity = isPrimaryActivity;
	}

	bool isPrimaryMode() const {
		return primaryMode;
	}

	void setPrimaryMode(bool isPrimaryMode) {
		this->primaryMode = isPrimaryMode;
	}

	const std::string& getPersonId() const {
		return personId;
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

	int getTcSeqNum() const {
		return tcSeqNum;
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

	void setPersonId(const std::string& personId)
	{
		this->personId = personId;
	}

	void setTcItemType(const std::string& tcItemType)
	{
		this->tcItemType = tcItemType;
	}

	void setTcSeqNum(uint8_t tcSeqNum)
	{
		this->tcSeqNum = tcSeqNum;
	}

private:
	std::string personId;
	uint8_t tcSeqNum;
	std::string tcItemType;
	std::string tripId;
	int tripOrigin;
	int tripDestination;
	std::string subtripId;
	int subtripOrigin;
	int subtripDestination;
	std::string subtripMode;
	bool primaryMode;
	std::string startTime;
	std::string activityId;
	std::string activityType;
	bool primaryActivity;
	int activityLocation;
	std::string activityStartTime;
	std::string activityEndTime;
};
} //end namespace medium
} //end namespace sim_mob


