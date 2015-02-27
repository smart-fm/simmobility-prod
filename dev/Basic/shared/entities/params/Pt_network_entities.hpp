//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License";" as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/**
 * Pt_network_entities.hpp
 *
 *  Created on: Feb 24, 2015
 *  \author Prabhuraj
 */
#include <string>

namespace sim_mob{

class PT_NetworkEdges{
public:
	PT_NetworkEdges();
	virtual ~PT_NetworkEdges();

	double getDayTransitTime() const {
		return dayTransitTime;
	}

	void setDayTransitTime(double dayTransitTime) {
		this->dayTransitTime = dayTransitTime;
	}

	double getDist() const {
		return dist;
	}

	void setDist(double dist) {
		this->dist = dist;
	}

	int getEdgeId() const {
		return edgeId;
	}

	void setEdgeId(int edgeId) {
		this->edgeId = edgeId;
	}

	const std::string& getEndStop() const {
		return endStop;
	}

	void setEndStop(const std::string& endStop) {
		this->endStop = endStop;
	}

	double getLinkTravelTime() const {
		return linkTravelTime;
	}

	void setLinkTravelTime(double linkTravelTime) {
		this->linkTravelTime = linkTravelTime;
	}

	const std::string& getRoadIndex() const {
		return road_index;
	}

	void setRoadIndex(const std::string& roadIndex) {
		road_index = roadIndex;
	}

	const std::string& getRoadEdgeId() const {
		return roadEdgeId;
	}

	void setRoadEdgeId(const std::string& roadEdgeId) {
		this->roadEdgeId = roadEdgeId;
	}

	const std::string& getServiceLines() const {
		return rServiceLines;
	}

	void setServiceLines(const std::string& serviceLines) {
		rServiceLines = serviceLines;
	}

	const std::string& getR_Type() const {
		return rType;
	}

	void setR_Type(const std::string& type) {
		rType = type;
	}

	const std::string& getStartStop() const {
		return startStop;
	}

	void setStartStop(const std::string& startStop) {
		this->startStop = startStop;
	}

	double getTransferPenalty() const {
		return transferPenalty;
	}

	void setTransferPenalty(double transferPenalty) {
		this->transferPenalty = transferPenalty;
	}

	double getTransitTime() const {
		return transitTime;
	}

	void setTransitTime(double transitTime) {
		this->transitTime = transitTime;
	}

	double getWaitTime() const {
		return waitTime;
	}

	void setWaitTime(double waitTime) {
		this->waitTime = waitTime;
	}

	double getWalkTime() const {
		return walkTime;
	}

	void setWalkTime(double walkTime) {
		this->walkTime = walkTime;
	}

private:
	std::string startStop;  // Alphanumeric id
	std::string endStop;    // Alphanumeric id
 	std::string rType;      // Service Line type, can be "BUS","LRT","WALK"
	std::string road_index;          // Index for road type 0 for BUS , 1 for LRT , 2 for Walk
	std::string roadEdgeId; // Strings of passing road segments Ex: 4/15/35/43
	std::string rServiceLines;
	double linkTravelTime;
	int  edgeId;
	double waitTime;
	double walkTime;
	double transitTime;
	double transferPenalty;
	double dayTransitTime;
	double dist;
};

class PT_NetworkVertices{
public:
	PT_NetworkVertices();
	virtual ~PT_NetworkVertices();

	const std::string& getEzlinkName() const {
		return ezlinkName;
	}

	void setEzlinkName(const std::string& ezlinkName) {
		this->ezlinkName = ezlinkName;
	}

	const std::string& getStopCode() const {
		return stopCode;
	}

	void setStopCode(const std::string& stopCode) {
		this->stopCode = stopCode;
	}

	const std::string& getStopDesc() const {
		return stopDesc;
	}

	void setStopDesc(const std::string& stopDesc) {
		this->stopDesc = stopDesc;
	}

	const std::string& getStopId() const {
		return stopId;
	}

	void setStopId(const std::string& stopId) {
		this->stopId = stopId;
	}

	double getStopLatitude() const {
		return stopLatitude;
	}

	void setStopLatitude(double stopLatitude) {
		this->stopLatitude = stopLatitude;
	}

	double getStopLongitude() const {
		return stopLongitude;
	}

	void setStopLongitude(double stopLongitude) {
		this->stopLongitude = stopLongitude;
	}

	const std::string& getStopName() const {
		return stopName;
	}

	void setStopName(const std::string& stopName) {
		this->stopName = stopName;
	}

	int getStopType() const {
		return stopType;
	}

	void setStopType(int stopType) {
		this->stopType = stopType;
	}

private:
	std::string stopId;
	std::string stopCode;
	std::string stopName;
	double stopLatitude;
	double stopLongitude;
	std::string ezlinkName;
	int stopType;
	std::string stopDesc;
};
}//End of namespace sim_mob
