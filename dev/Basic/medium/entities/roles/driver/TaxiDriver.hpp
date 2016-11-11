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
namespace sim_mob
{
	namespace medium
	{
		class TaxiDriver : public Driver
		{
		public:
			TaxiDriver(Person_MT* parent, const MutexStrategy& mtxStrat, TaxiDriverBehavior* behavior,
					TaxiDriverMovement* movement, std::string roleName, Role<Person_MT>::Type roleType=Role<Person_MT>::RL_TAXIDRIVER) :
					Driver(parent, behavior, movement, roleName, roleType)
		{
				taxiPassenger = nullptr;
		}
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
			virtual ~TaxiDriver();
			enum DriverMode
			{
				DRIVE_TO_NODE =0,
				DRIVE_TO_TAXISTAND,
				CRUISE,
				DRIVE_WITH_PASSENGER,
				QUEUING_AT_TAXISTAND
			};
			void setDriveMode(DriverMode mode);

		private:
			Passenger *taxiPassenger;
			Node * destinationNode;
			Node *originNode;
			Node *currentNode;
			RoadSegment *currSegment;
			bool personBoarded = false;
			std::vector<RoadSegment *> currentRoute;
			std::vector<WayPoint> currentRouteChoice;

			DriverMode driverMode;
			public:
			DriverMode getDriverMode()
			{
				return driverMode;
			}

			friend class TaxiDriverBehavior;
			friend class TaxiDriverMovement;
		};
	}
}

#endif /* ENTITIES_ROLES_DRIVER_TAXIDRIVER_HPP_ */
