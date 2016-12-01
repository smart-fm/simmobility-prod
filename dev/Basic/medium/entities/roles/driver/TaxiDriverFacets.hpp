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
		class TaxiDriverMovement :public DriverMovement
		{
		public:
			TaxiDriverMovement();
			virtual ~TaxiDriverMovement();
			virtual void frame_init();
			virtual void frame_tick();
			TaxiDriver * getParentDriver()
			{
				return parentTaxiDriver;
			}
			void setParentTaxiDriver(TaxiDriver * taxiDriver);
			Node* getCurrentNode();
			Node* getDestinationNode();
			void addRouteChoicePath(std::vector<WayPoint> &currentRouteChoice,Conflux *);
		private:
			TaxiDriver *parentTaxiDriver = nullptr;
			Node * destinationNode=nullptr;
			Node *originNode=nullptr;
			Node *currentNode=nullptr;
			RoadSegment *currSegment;
			bool personBoarded = false;
			bool isPathInitialized = false;
			std::map<const Link*,int> mapOfLinksAndVisitedCounts;
			std::vector<RoadSegment *> currentRoute= std::vector<RoadSegment *>();
			std::vector<WayPoint> currentRouteChoice;
			void driveToDestinationNode(Node * destinationNode);
			void runRouteChoiceModel(Node *origin,Node *destination);
			void assignFirstNode();
			void setCruisingMode();
			void driveToTaxiStand();
			void setCurrentNode(Node *currNode);
			void setDestinationNode(Node *destinationNode);


			void driveToNode(Node *destinationNode);
			void getLinkAndRoadSegments(Node * start ,Node *end,std::vector<RoadSegment*>& segments);
			void reachNextLinkIfPossible(DriverUpdateParams& params);
			const Lane* getBestTargetLane(const SegmentStats* nextSegStats, const SegmentStats* nextToNextSegStats);
			bool canGoToNextRdSeg(DriverUpdateParams& params, const SegmentStats* nextSegStats, const Link* nextLink) const;
			bool moveToNextSegment(DriverUpdateParams& params);
			void selectNextNodeAndLinksWhileCruising();
			void setPath(std::vector<const SegmentStats*>&path);
			Conflux * getPrevConflux();

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
