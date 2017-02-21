/*
 * TaxiDriverFacets.hpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
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
	TaxiDriver * getParentDriver();
	void setParentTaxiDriver(TaxiDriver * taxiDriver);
	const Node* getCurrentNode();
	const Node* getDestinationNode();
	void addRouteChoicePath(std::vector<WayPoint> &currentChoice);
	void setCurrentNode(const Node * node);
	void setDestinationNode(const Node * node);
	bool isToBeRemovedFromTaxiStand();

private:
	const Node * destinationNode = nullptr;
	const Node *currentNode = nullptr;
	TaxiDriver *parentTaxiDriver = nullptr;
	Node *originNode = nullptr;
	bool isQueuingTaxiStand = false;
	bool isPathInitialized = false;
	const TaxiStand *destinationTaxiStand = nullptr;
	const TaxiStand *previousTaxiStand = nullptr;
	bool toBeRemovedFromTaxiStand = false;
	std::map<const Link*, int> mapOfLinksAndVisitedCounts;
	double cruisingTooLongTime = 0.0;
	double queuingTooLongTime = 0.0;
	Link* selectedNextLinkInCrusing = nullptr;

private:
	void driveToDestinationNode(Node * destinationNode);
	void addTaxiStandPath(std::vector<WayPoint> &routeToTaxiStand);
	void assignFirstNode();
	void setCruisingMode();
	void driveToTaxiStand();
	void setCurrentNode(Node *currNode);
	void setDestinationNode(Node *destinationNode);
	const Lane* getBestTargetLane(const SegmentStats* nextSegStats,const SegmentStats* nextToNextSegStats);
	bool moveToNextSegment(DriverUpdateParams& params);
	void selectNextNodeAndLinksWhileCruising();
	void addCruisingPath(const Link* selectedLink);
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
