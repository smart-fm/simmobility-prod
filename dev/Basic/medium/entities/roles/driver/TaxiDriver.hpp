/*
 * TaxiDriver.hpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
 */

#ifndef ENTITIES_ROLES_DRIVER_TAXIDRIVER_HPP_
#define ENTITIES_ROLES_DRIVER_TAXIDRIVER_HPP_

#include "Driver.hpp"
#include "TaxiDriverFacets.hpp"
#include  "entities/roles/passenger/Passenger.hpp"
#include "buffering/Shared.hpp"
#include "buffering/BufferedDataManager.hpp"
namespace sim_mob
{
	namespace medium
	{
		class TaxiDriver : public Driver
		{
		public:
			TaxiDriver(Person_MT* parent, const MutexStrategy& mtxStrat,TaxiDriverBehavior* behavior,
					TaxiDriverMovement* movement, std::string roleName, Role<Person_MT>::Type roleType=Role<Person_MT>::RL_TAXIDRIVER);

			TaxiDriver(Person_MT* parent, const MutexStrategy& mtx);
			void addPassenger(Passenger* passenger);
			Passenger * alightPassenger();
			void boardPassenger(Passenger *passenger);
			void driveToDestinationNode(Node * destinationNode);
			void runRouteChoiceModel(Node *origin,Node *destination);
			void setCruisingMode();
			void driveToTaxiStand();
			bool hasPersonBoarded();
			void setCurrentNode(Node *currNode);
			void setDestinationNode(Node *destinationNode);
			Node* getDestinationNode();
			Person* getParent();
			Node* getCurrentNode();
			void driveToNode(Node *destinationNode);
			void getLinkAndRoadSegments(Node * start ,Node *end,std::vector<RoadSegment*>& segments);
			void checkPersonsAndPickUpAtNode(timeslice now);
			TaxiDriverMovement * getMovementFacet();
			virtual Role<Person_MT>* clone(Person_MT *parent) const;
			virtual ~TaxiDriver();
			void make_frame_tick_params(timeslice now);
			std::vector<BufferedBase*> getSubscriptionParams();
			enum DriverMode
			{
				DRIVE_TO_NODE =0,
				DRIVE_TO_TAXISTAND,
				CRUISE,
				DRIVE_WITH_PASSENGER,
				QUEUING_AT_TAXISTAND
			};
			void setDriveMode(DriverMode mode);
			sim_mob::medium::TaxiDriver::DriverMode getDriverMode();

		private:
			Passenger *taxiPassenger;
			Node * destinationNode;
			Node *originNode;
			Node *currentNode;
			RoadSegment *currSegment;
			TaxiDriverMovement *taxiDriverMovement;
			TaxiDriverBehavior *taxiDriverBehaviour;
			bool personBoarded = false;
			std::vector<RoadSegment *> currentRoute;
			std::vector<WayPoint> currentRouteChoice;
			DriverMode driverMode;
			public:



			friend class TaxiDriverBehavior;
			friend class TaxiDriverMovement;
		};
	}
}

#endif /* ENTITIES_ROLES_DRIVER_TAXIDRIVER_HPP_ */
