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
	void addRouteChoicePath(std::vector<WayPoint> &currentChoice,Conflux *);
	void setCurrentNode(const Node * node);
	void setDestinationNode(const Node * node);
	bool isToBeRemovedFromTaxiStand();

private:
	TaxiDriver *parentTaxiDriver = nullptr;
	const Node * destinationNode = nullptr;
	Node *originNode = nullptr;
	bool isQueuingTaxiStand = false;
	const Node *currentNode = nullptr;
	bool isPathInitialized = false;
	double waitingTimeAtTaxiStand = -1;
	const TaxiStand *destinationTaxiStand = nullptr;
	const TaxiStand *previousTaxiStand = nullptr;
	bool toBeRemovedFromTaxiStand = false;
	bool toCallMovTickInTaxiStand = false;
	std::map<const Link*, int> mapOfLinksAndVisitedCounts;
	std::vector<RoadSegment *> currentRoute;
	std::vector<WayPoint> currentRouteChoice;

private:
	void driveToDestinationNode(Node * destinationNode);
	void addTaxiStandPath(std::vector<WayPoint> &routeToTaxiStand);
	void assignFirstNode();
	void setCruisingMode();
	void driveToTaxiStand();
	void setCurrentNode(Node *currNode);
	void setDestinationNode(Node *destinationNode);
	void getLinkAndRoadSegments(Node * start, Node *end,std::vector<RoadSegment*>& segments);
	const Lane* getBestTargetLane(const SegmentStats* nextSegStats,const SegmentStats* nextToNextSegStats);
	bool moveToNextSegment(DriverUpdateParams& params);
	void selectNextNodeAndLinksWhileCruising();
	void setPath(std::vector<const SegmentStats*>&path);

public:
	void setToCallMovementTick(bool toCall);
	bool getToCallMovementTick();

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
