/*
 * TaxiDriverFacets.hpp
 *
 *  Created on: 5 Nov 2016
 *      Author: zhang huai peng
 */

#ifndef ENTITIES_ROLES_DRIVER_TAXIDRIVERFACETS_HPP_
#define ENTITIES_ROLES_DRIVER_TAXIDRIVERFACETS_HPP_

#include "DriverFacets.hpp"

namespace sim_mob
{
namespace medium
{
class TaxiDriver;
class TaxiDriverMovement: public DriverMovement
{
public:
	TaxiDriverMovement();
	virtual ~TaxiDriverMovement();
	virtual void frame_init();
	virtual void frame_tick();
	/**
	 * internal structure for breaking
	 */
	struct BrokenInfo
	{
		const Node* parkingNode = nullptr;
		unsigned int duration = 0;
	};
	/**
	 * get parent taxi driver
	 * @return the result holding parent taxi driver.
	 */
	TaxiDriver * getParentDriver();
	/**
	 * set parent taxi driver
	 * @param taxiDriver is a pointer to parent taxi driver
	 */
	void setParentTaxiDriver(TaxiDriver * taxiDriver);
	/**
	 * get current node
	 * @return current start node
	 */
	const Node* getCurrentNode();
	/**
	 * get next destination node
	 * @return next destination node
	 */
	const Node* getDestinationNode();
	/**
	 * set new route after route choice
	 * @param currentChoice is a list of links holding new route
	 */
	void addRouteChoicePath(std::vector<WayPoint> &currentChoice);
	/**
	 * set current node as new start node
	 * @param node is current start node
	 */
	void setCurrentNode(const Node * node);
	/**
	 * set next destination node
	 * @param node is next destination node
	 */
	void setDestinationNode(const Node * node);
	/**
	 * query whether this taxi can be remove from taxi stand or not
	 * @return true when taxi is leaving from current stand
	 */
	bool isToBeRemovedFromTaxiStand();
	/**
	 * driving taxi for oncall request
	 * @param personId is person id who is calling taxi
	 * @param destination is a node where calling person is waiting there
	 * @return true if calling request is successful
	 */
	bool driveToNodeOnCall(const std::string& personId, const Node* destination);

private:
	/**record next destination node*/
	const Node * destinationNode = nullptr;
	/**record current start node*/
	const Node *currentNode = nullptr;
	/**record parent taxi driver*/
	TaxiDriver *parentTaxiDriver = nullptr;
	/**record original start node*/
	Node *originNode = nullptr;
	/**indicate whether the path already initialized or not*/
	bool isPathInitialized = false;
	/**record next destination stand*/
	const TaxiStand *destinationTaxiStand = nullptr;
	/**record previous taxi stand when leaving from stand*/
	const TaxiStand *previousTaxiStand = nullptr;
	/**indicate whether taxi will be left from current stand*/
	bool toBeRemovedFromTaxiStand = false;
	std::map<const Link*, int> mapOfLinksAndVisitedCounts;
	/**record total cruising time*/
	double cruisingTooLongTime = 0.0;
	/**record total queuing time at the stand*/
	double queuingTooLongTime = 0.0;
	/**record next selected link when cruising*/
	Link* selectedNextLinkInCrusing = nullptr;
	/**the vector to record second or third drivers*/
	std::queue<TaxiFleetManager::TaxiFleet> taxiFleets;
	/**record breaking information*/
	std::shared_ptr<BrokenInfo> nextBroken;
	/**record person id to be picked up*/
	std::string personIdPickedUp;
private:
	/**
	 * assign taxi at original node
	 */
	void assignFirstNode();
	/**
	 * set current mode as cruising
	 */
	void setCruisingMode();
	/**
	 * drive taxi to next taxi stand
	 */
	void driveToTaxiStand();
	/**
	 * set current node
	 * @param currNode is a pointer to current node
	 */
	void setCurrentNode(Node *currNode);
	/**
	 * set destination node
	 * @param destinationNode is a pointer to destination node
	 */
	void setDestinationNode(Node *destinationNode);
	/**
	 * get best lane when enter a new road segment
	 * @param nextSegStats is a pointer to next segment stats
	 * @param nextToNextSegStats is a pointer to next to next segment stats
	 * @return target lane
	 */
	const Lane* getBestTargetLane(const SegmentStats* nextSegStats,const SegmentStats* nextToNextSegStats);
	/**
	 * perform movement into next segment
	 * @param params hold current driver information
	 */
	bool moveToNextSegment(DriverUpdateParams& params);
	/**
	 * select next link when cruising
	 */
	void selectNextLinkWhileCruising();
	/**
	 * add next cruising path
	 * @param selectedLink is a pointer to next select link
	 */
	void addCruisingPath(const Link* selectedLink);
	/**
	 * check fleet information for driver changing
	 */
	bool checkNextFleet();
	/**
	 * set break mode at destination node
	 */
	bool setBreakMode();
	/**
	 * set broken information
	 */
	bool setBreakInfo(const Node* next, const unsigned int duration);
};

class TaxiDriverBehavior: public DriverBehavior
{
public:
	TaxiDriverBehavior();
	virtual ~TaxiDriverBehavior();
};
}
}

#endif /* ENTITIES_ROLES_DRIVER_TAXIDRIVERFACETS_HPP_ */
