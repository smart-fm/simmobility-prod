/*
 * TaxiDriverFacets.cpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
 */

#include <entities/roles/driver/TaxiDriverFacets.hpp>
#include "TaxiDriver.hpp"
#include "path/PathSetManager.hpp"

namespace sim_mob
{
	namespace medium
	{
		TaxiDriverMovement::TaxiDriverMovement()
		{
			// TODO Auto-generated constructor stub

		}

		TaxiDriverMovement::~TaxiDriverMovement()
		{
			// TODO Auto-generated destructor stub
		}

		void TaxiDriverMovement::frame_init()
		{
			//initialize path of Taxi,set the origin node
			Vehicle* newVeh = new Vehicle(Vehicle::TAXI, sim_mob::TAXI_LENGTH);
			VehicleBase* oldTaxi = parentDriver->getResource();
			safe_delete_item(oldTaxi);
		}

		const Lane* TaxiDriverMovement::getBestTargetLane(const SegmentStats* nextSegStats, const SegmentStats* nextToNextSegStats)
		{
			const BusStop* nextTaxiStand; //= routeTracker.getNextStop();
			TaxiDriver::DriverMode mode = parentDriver->getDriverMode();
			//if it is Taxi Stand lane
			if(mode==TaxiDriver::DRIVE_TO_TAXISTAND&&nextTaxiStand && nextSegStats->hasBusStop(nextTaxiStand))
			{
				return nextSegStats->getOutermostLane();
			}
			else
			{
				const Lane* minLane = nullptr;
				double minQueueLength = std::numeric_limits<double>::max();
				double minLength = std::numeric_limits<double>::max();
				double que = 0.0;
				double total = 0.0;

				const Link* nextLink = getNextLinkForLaneChoice(nextSegStats);
				const std::vector<Lane*>& lanes = nextSegStats->getRoadSegment()->getLanes();
				for (std::vector<Lane* >::const_iterator lnIt=lanes.begin(); lnIt!=lanes.end(); ++lnIt)
				{
					const Lane* lane = *lnIt;
					if (!lane->isPedestrianLane())
					{
						if(nextToNextSegStats
								&& !isConnectedToNextSeg(lane, nextToNextSegStats->getRoadSegment())
								&& !nextSegStats->isConnectedToDownstreamLink(nextLink, lane))
						{
							if(parentDriver->getDriverMode()!=TaxiDriver::CRUISE)
							{
								if(nextLink&&(!nextSegStats->isConnectedToDownstreamLink(nextLink, lane)))
								{
									continue;
								}
							}
							else
							{
								continue;
							}

						}
						total = nextSegStats->getLaneTotalVehicleLength(lane);
						que = nextSegStats->getLaneQueueLength(lane);
						if (minLength > total)
						{
							//if total length of vehicles is less than current minLength
							minLength = total;
							minQueueLength = que;
							minLane = lane;
						}
						else if (minLength == total)
						{
							//if total length of vehicles is equal to current minLength
							if (minQueueLength > que)
							{
								//and if the queue length is less than current minQueueLength
								minQueueLength = que;
								minLane = lane;
							}
						}
					}
				}

				if(!minLane)
				{
					//throw std::runtime_error("best target lane was not set!");
					//TODO: if minLane is null, there is probably no lane connection from any lane in next segment stats to
					// the lanes in the nextToNextSegmentStats. The code in this block is a hack to avoid errors due to this reason.
					//This code must be removed and an error must be thrown here in future.
					for (std::vector<Lane* >::const_iterator lnIt=lanes.begin(); lnIt!=lanes.end(); ++lnIt)
					{
						if (!((*lnIt)->isPedestrianLane()))
						{
							const Lane* lane = *lnIt;
							total = nextSegStats->getLaneTotalVehicleLength(lane);
							que = nextSegStats->getLaneQueueLength(lane);
							if (minLength > total)
							{
								//if total length of vehicles is less than current minLength
								minLength = total;
								minQueueLength = que;
								minLane = lane;
							}
							else if (minLength == total)
							{
								//if total length of vehicles is equal to current minLength
								if (minQueueLength > que)
								{
									//and if the queue length is less than current minQueueLength
									minQueueLength = que;
									minLane = lane;
								}
							}
						}
					}
				}
				return minLane;
			}
		}

		bool TaxiDriverMovement::canGoToNextRdSeg(DriverUpdateParams& params, const SegmentStats* nextSegStats, const Link* nextLink) const
		{
			//return false if the Driver cannot be added during this time tick
			if (params.elapsedSeconds >= params.secondsInTick)
			{
				return false;
			}

			//check if the next road segment has sufficient empty space to accommodate one more vehicle
			if (!nextSegStats)
			{
				return false;
			}

			double enteringVehicleLength = parentDriver->getResource()->getLengthInM();
			double maxAllowed = nextSegStats->getNumVehicleLanes() * nextSegStats->getLength();
			double total = nextSegStats->getTotalVehicleLength();

			//if the segment is shorter than the vehicle's length and there are no vehicles in the segment just allow the vehicle to pass through
			//this is just an interim arrangement. this segment should either be removed from database or it's length must be updated.
			//if this hack is not in place, all vehicles will start queuing in upstream segments forever.
			//TODO: remove this hack and put permanent fix
			if ((maxAllowed < enteringVehicleLength) && (total <= 0))
			{
				return true;
			}

			//if this segment is a bus terminus segment, we assume only buses try to enter this segment and allow the bus inside irrespective of available space.
			/*if(TaxiDriver::DRIVE_TO_TAXISTAND&&nextSegStats->getRoadSegment()->isTaxiStandSegment())
			{
				return true;
			}*/


			bool hasSpaceInNextStats = ((maxAllowed - total) >= enteringVehicleLength);
			return hasSpaceInNextStats;
		}

		void TaxiDriverMovement::reachNextLinkIfPossible(DriverUpdateParams& params)
		{
			const SegmentStats* nextSegStats = pathMover.getNextSegStats(false);
			const SegmentStats* nextToNextSegStats = pathMover.getSecondSegStatsAhead();
			const Lane* laneInNextSegment = getBestTargetLane(nextSegStats, nextToNextSegStats);
			double departTime = getLastAccept(laneInNextSegment, nextSegStats);
			params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (params.now.ms()/1000.0));
			//const Link* nextLink = getNextLinkForLaneChoice(nxtSegStat);
			const Link *nextLink = getNextLinkForLaneChoice(nextSegStats);
			if (canGoToNextRdSeg(params, nextSegStats,nextLink ))
			{
				if (isQueuing)
				{
					removeFromQueue();
				}
				else
				{
					//departFromCurrentTaxiStand();
				}

				currLane = laneInNextSegment;
				pathMover.advanceInPath();
				pathMover.setPositionInSegment(nextSegStats->getLength());

			}
			else
			{
				if (parentDriver->getResource()->isMoving())
				{
					//Person is in previous segment (should be added to queue if canGoTo failed)
					if (pathMover.getCurrSegStats() == parentDriver->parent->getCurrSegStats())
					{
						if (currLane)
						{
							if (parentDriver->parent->isQueuing)
							{
								moveInQueue();
							}
							else
							{
								addToQueue(); // adds to queue if not already in queue
							}
							parentDriver->parent->canMoveToNextSegment = Person_MT::NONE; // so that advance() and setParentData() is called subsequently
						}
					}
					else if (pathMover.getNextSegStats(false) == parentDriver->parent->getCurrSegStats())
					{
						//Person is in virtual queue (should remain in virtual queues if canGoTo failed)
						//do nothing
					}
					else
					{
						DebugStream << "Driver " << parentDriver->parent->getId() << "was neither in virtual queue nor in previous segment!" << "\ndriver| segment: "
								<< pathMover.getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|id: "
								<< pathMover.getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|lane: " << currLane->getLaneId() << "\nPerson| segment: "
								<< parentDriver->parent->getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|id: "
								<< parentDriver->parent->getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|lane: "
								<< (parentDriver->parent->getCurrLane() ? parentDriver->parent->getCurrLane()->getLaneId() : 0) << std::endl;

						throw ::std::runtime_error(DebugStream.str());
					}
					params.elapsedSeconds = params.secondsInTick;
					parentDriver->parent->setRemainingTimeThisTick(0.0);
				}
				else
				{
					//the bus driver is currently serving a stop
					params.elapsedSeconds = params.secondsInTick; //remain in bus stop
					parentDriver->parent->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
					parentDriver->parent->canMoveToNextSegment = Person_MT::NONE; // so that in the next tick, flowIntoNextLinkIfPossible() is not called in the next tick without requesting for permission again
				}
			}
		}


		bool TaxiDriverMovement::moveToNextSegment(DriverUpdateParams& params)
		{
			const SegmentStats* currSegStat = pathMover.getCurrSegStats();
			//const BusStop* nextStop = routeTracker.getNextStop();
			//const TaxiStand *taxiStand =getTaxiStand();
			/*if (nextStop && currSegStat->hasBusStop(nextStop))
			{
				BusStopAgent* stopAg = BusStopAgent::getBusStopAgentForStop(nextStop);
				if (stopAg)
				{
					if (stopAg->canAccommodate(parentBusDriver->getResource()->getLengthInM()))
					{
						if (parentBusDriver->getResource()->isMoving())
						{
							if (isQueuing)
							{
								removeFromQueue();
							}
						}
						else
						{
							departFromCurrentStop();
						}

						DailyTime startTm = ConfigManager::GetInstance().FullConfig().simStartTime();
						DailyTime current(params.now.ms() + converToMilliseconds(params.elapsedSeconds) + startTm.getValue());
						routeTracker.setCurrentStop(nextStop);
						routeTracker.updateNextStop();
						parentBusDriver->openBusDoors(current.getStrRepr(), stopAg);
						double remainingTime = params.secondsInTick - params.elapsedSeconds;
						if (parentBusDriver->waitingTimeAtbusStop > remainingTime)
						{
							parentBusDriver->waitingTimeAtbusStop -= remainingTime;
							params.elapsedSeconds = params.secondsInTick;
						}
						else
						{
							params.elapsedSeconds += parentBusDriver->waitingTimeAtbusStop;
							parentBusDriver->waitingTimeAtbusStop = 0;
							// There is remaining time, try to move to next segment
							bool movedToNextSegStat = moveToNextSegment(params);
							if (movedToNextSegStat)
							{
								//moveToNextSegment() calls advance() which inturn may call moveToNextSegment().
								//It is possible that the above call to moveToNextSegment was successful but the bus driver
								//has entered another stop in a downstream segment. In this case, currentStop will be a pointer to the
								//downstream stop that the bus driver entered.
								parentBusDriver->closeBusDoors(stopAg); //this handles departure and sets isMoving to true
								if (routeTracker.getCurrentStop())
								{
									if (routeTracker.getCurrentStop() == stopAg->getBusStop())
									{
										//trivial case. the bus driver has simply moved out of current stop
										routeTracker.setCurrentStop(nullptr);
									}
									else
									{
										//the bus driver has moved out of current segment and moved into some other stop in a downstream segment
										//set isMoving back to false because some other stop is being served
										parentBusDriver->getResource()->setMoving(false);
									}
								}
							}
							//else remain in bus stop
							return movedToNextSegStat;
						}
					}
					else
					{
						if(parentDriver->getResource()->isMoving())
						{
							if (isQueuing)
							{
								moveInQueue();
							}
							else
							{
								addToQueue();
							}
						}
						params.elapsedSeconds = params.secondsInTick;
						parentDriver->parent->setRemainingTimeThisTick(0.0);
					}*/



			//else
			//{
				bool res = false;
				bool isEndOfLink_CruiseMode = false;
				bool isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
				if(parentDriver->getDriverMode()==TaxiDriver::CRUISE)
				{
					if(pathMover.isEndOfPath())
					{
						isEndOfLink_CruiseMode = true;
					}
				}

				currSegStat = pathMover.getCurrSegStats();
				const SegmentStats* nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);

				//currently the best place to call a handler indicating 'Done' with segment.
				const RoadSegment *curRs = (*(pathMover.getCurrSegStats())).getRoadSegment();
				//Although the name of the method suggests segment change, it is actually segStat change. so we check again!
				const RoadSegment *nxtRs = (nxtSegStat ? nxtSegStat->getRoadSegment() : nullptr);

				if (curRs && curRs != nxtRs)
				{
					onSegmentCompleted(curRs, nxtRs);
				}

				if (isNewLinkNext||isEndOfLink_CruiseMode)
				{
					onLinkCompleted(curRs->getParentLink(), (nxtRs ? nxtRs->getParentLink() : nullptr));
				}

				//reset these local variables in case path has been changed in onLinkCompleted
				isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
				currSegStat = pathMover.getCurrSegStats();
				nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);

				if (!nxtSegStat)
				{
					//vehicle is done
					pathMover.advanceInPath();
					if (pathMover.isPathCompleted())
					{
						const Link* currLink = currSegStat->getRoadSegment()->getParentLink();
						double linkExitTime = params.elapsedSeconds + (params.now.ms()/1000.0);
						parentDriver->parent->currLinkTravelStats.finalize(currLink, linkExitTime, nullptr);
						TravelTimeManager::getInstance()->addTravelTime(parentDriver->parent->currLinkTravelStats); //in seconds
						currSegStat->getParentConflux()->setLinkTravelTimes(linkExitTime, currLink);
						parentDriver->parent->currLinkTravelStats.reset();
						currSegStat->getParentConflux()->getLinkStats(currLink).removeEntitiy(parentDriver->parent);
						setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat) - 1), currSegStat);
						currLane = nullptr;
						parentDriver->parent->setToBeRemoved();
					}
					params.elapsedSeconds = params.secondsInTick;
					return true;
				}

				if (isNewLinkNext||isEndOfLink_CruiseMode)
				{
					parentDriver->parent->requestedNextSegStats = nxtSegStat;
					parentDriver->parent->canMoveToNextSegment = Person_MT::NONE;
					if(isEndOfLink_CruiseMode)
					{
						//select the next cruising node and link
						selectNextNodeAndLinksWhileCruising();
						//check for any persons and then pick up at current node
						parentDriver->checkPersonsAndPickUpAtNode(params.now);

					}
					return false; // return whenever a new link is to be entered. Seek permission from Conflux.
				}

				const SegmentStats* nextToNextSegStat = pathMover.getSecondSegStatsAhead();
				const Lane* laneInNextSegment = getBestTargetLane(nxtSegStat, nextToNextSegStat);

				double departTime = getLastAccept(laneInNextSegment, nxtSegStat);
		/*		if(!nxtSegStat->isShortSegment())
				{
					if(nxtSegStat->hasQueue())
					{
						departTime += getAcceptRate(laneInNextSegment, nxtSegStat); //in seconds
					}
					else
					{
						departTime += (PASSENGER_CAR_UNIT / (nxtSegStat->getNumVehicleLanes() * nxtSegStat->getSegSpeed(true)));
					}
				}
		*/
				params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (params.now.ms()/1000.0)); //in seconds

				const Link* nextLink = getNextLinkForLaneChoice(nxtSegStat);
				if (canGoToNextRdSeg(params, nxtSegStat, nextLink))
				{
					if (isQueuing)
					{
						removeFromQueue();
					}

					setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat) - 1), currSegStat); // decrement from the currLane before updating it
					currLane = laneInNextSegment;
					pathMover.advanceInPath();
					pathMover.setPositionInSegment(nxtSegStat->getLength());
					double segExitTimeSec = params.elapsedSeconds + (params.now.ms()/1000.0);
					setLastAccept(currLane, segExitTimeSec, nxtSegStat);

					const SegmentStats* prevSegStats = pathMover.getPrevSegStats(true); //previous segment is in the same link
					if (prevSegStats
							&& prevSegStats->getRoadSegment() != pathMover.getCurrSegStats()->getRoadSegment())
					{
						// update road segment travel times
						updateScreenlineCounts(prevSegStats, segExitTimeSec);
					}

					res = true;
					parentDriver->getResource()->setMoving(true);//set moving is set to true here explicitly because the BD could try to move to next segement from within advance and we want the correct moving status there
					advance(params);
				}
				else
				{
					if(parentDriver->getResource()->isMoving())
					{
						if (isQueuing)
						{
							moveInQueue();
						}
						else
						{
							addToQueue();
						}
					}

					params.elapsedSeconds = params.secondsInTick;
					parentDriver->parent->setRemainingTimeThisTick(0.0);
				}
				return res;
			//}
		}

		void TaxiDriverMovement::frame_tick()
		{
			DriverUpdateParams& params = parentDriver->getParams();
			TaxiDriver::DriverMode mode = parentDriver->getDriverMode();
			/*if(mode == TaxiDriver::DRIVE_TO_TAXISTAND)
			{
				if (parentDriver->parent->canMoveToNextSegment == Person_MT::GRANTED)
				{
					reachNextLinkIfPossible(params);
				}
				else if (parentDriver->parent->canMoveToNextSegment == Person_MT::DENIED)
				{
					if (parentDriver->getResource()->isMoving())
					{
						if (currLane)
						{
							if (parentDriver->parent->isQueuing)
							{
								moveInQueue();
							}
							else
							{
								addToQueue(); // adds to queue if not already in queue
							}

							params.elapsedSeconds = params.secondsInTick;
							setParentData(params);
						}
					}
					else
					{
						params.elapsedSeconds = params.secondsInTick;
						pathMover.setPositionInSegment(0);
						setParentData(params);
					}
					parentDriver->parent->canMoveToNextSegment = Person_MT::NONE;
				}
			}

			else if(mode == TaxiDriver::QUEUING_AT_TAXISTAND)
			{
				//waiting for passenger
			}

			else if (mode == TaxiDriver::DRIVE_WITH_PASSENGER)
			{
				/*if(!parentDriver->hasPersonBoarded())
				{
					parentDriver->boardPassenger()
				}
			}*/

			if (mode == TaxiDriver::CRUISE)
			{
				//if it has reached at the end of the segment
				if (parentDriver->parent->canMoveToNextSegment == Person_MT::GRANTED)
				{
					if (parentDriver->parent->canMoveToNextSegment == Person_MT::GRANTED)
					{
						reachNextLinkIfPossible(params);
					}
				}

				else if (parentDriver->parent->canMoveToNextSegment == Person_MT::DENIED)
				{
					if (parentDriver->getResource()->isMoving())
					{
						if (currLane)
						{
							if (parentDriver->parent->isQueuing)
							{
								moveInQueue();
							}
							else
							{
								addToQueue(); // adds to queue if not already in queue
							}

							params.elapsedSeconds = params.secondsInTick;
							setParentData(params);
						}
					}
					parentDriver->parent->canMoveToNextSegment = Person_MT::NONE;
				}
				/*double pos;
				if(pos == 0)
				{
					//get next node
					Node * nextNode;
					parentDriver->setCurrentNode(parentDriver->getDestinationNode());
					parentDriver->setDestinationNode(nextNode);
					SubTrip currSubTrip;
					//currSubTrip.origin = WayPoint(parentDriver->getCurrentNode());
					//currSubTrip.destination = WayPoint(parentDriver->getDestinationNode());
					//parentDriver->driveToNode(parentDriver->getDestinationNode());
					Node *currNode = parentDriver->getCurrentNode();
					Node *destNode = parentDriver->getDestinationNode();
					std::map<unsigned int,Link*> mapOfDownStreamLinks = currNode->getDownStreamLinks();
					std::map<unsigned int,Link*>::iterator linkItr = mapOfDownStreamLinks.begin();
					while(linkItr!=mapOfDownStreamLinks.end())
					{
						Link *link = (*linkItr).second;
						Node *toNode = link->getToNode();
						if(toNode == destNode)
						{
							link->getRoadSegments();
							break;
						}
						linkItr++;
					}

				}
				else
				{
					// move in the segment
					// it has crossed the end of the link go to next node
				}*/
				if (!parentDriver->parent->requestedNextSegStats && params.elapsedSeconds < params.secondsInTick)
				{
					DriverMovement::frame_tick();
				}
				else if(parentDriver->parent->canMoveToNextSegment == Person_MT::NONE)
				{
					// canMoveToNextSegment is set to Granted or Denied only by the conflux
					// If canMoveToNextSegment is not NONE at this point, the bus driver must have remained in VQ
					// Set parent data must not be called if the driver remained in VQ.
					setParentData(params);
				}
			}

		}

		void TaxiDriverMovement::getLinkAndRoadSegments(Node * start ,Node *end,std::vector<RoadSegment*>& segments)
		{
			std::map<unsigned int,Link*> downStreamLinks = start->getDownStreamLinks();
			std::map<unsigned int,Link*>::iterator itr = downStreamLinks.begin();
			while(itr!=downStreamLinks.end())
			{
				Link *link=itr->second;
				Node *toNode = link->getToNode();
				if(toNode == end)
				{
					segments = link->getRoadSegments();
					break;
				}
				itr++;
			}
		}

		void TaxiDriverMovement::selectNextNodeAndLinksWhileCruising()
		{
			if(!destinationNode)
			{
				currentNode = originNode;
			}
			else
			{
				currentNode = destinationNode;
			}
			std::vector<Node*> nodeVector = currentNode->getNeighbouringNodes();
			if(nodeVector.size()>1)
			{
				std::vector<Node*>::iterator itr = nodeVector.begin();
				Node *destNode = (*(itr+1));
				Node *originNode = *itr;
				std::map<unsigned int,Link*> mapOfDownStreamLinks = originNode->getDownStreamLinks();
				std::map<unsigned int,Link*>::iterator linkItr = mapOfDownStreamLinks.begin();
				while(linkItr!=mapOfDownStreamLinks.end())
				{
					Link *link = (*linkItr).second;
					Node *toNode = link->getToNode();
					if(toNode == destNode)
					{
						//append the new segments to current segment
						//currentRoute.push_back(currentRoute.end(),link->getRoadSegments().begin(),link->getRoadSegments().end());
					}
					linkItr++;
				}
			}
			//depth first or breath first
			//select a node
		}

		void TaxiDriverMovement::setCruisingMode()
		{
			parentDriver->setDriveMode(TaxiDriver::CRUISE);
			selectNextNodeAndLinksWhileCruising();
		}

		void TaxiDriverMovement::driveToTaxiStand(/*TaxiStand *stand */)
		{
			parentDriver->setDriveMode(TaxiDriver::DRIVE_TO_TAXISTAND);
			SubTrip currSubTrip;
			//currSubTrip.origin = assign node or segment
			bool useInSimulationTT = parentDriver->getParent()->usesInSimulationTravelTime();
			RoadSegment *taxiStandSegment;
			//RoadSegment taxiStandSegment = TaxiStand->getSegment();
			const Link *taxiStandLink = taxiStandSegment->getParentLink();
			Node *destForRouteChoice = taxiStandLink->getFromNode();
			//get the current segment
			const Link *currSegmentParentLink = currSegment->getParentLink();
			Node *originNode = currSegmentParentLink->getToNode();
			//set origin and destination node
			currSubTrip.origin = WayPoint(originNode);
			currSubTrip.destination = WayPoint(destForRouteChoice);
			PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip,false, nullptr, useInSimulationTT);

		}


		void TaxiDriverMovement::driveToNode(Node *destinationNode)
		{
			const Link *currSegmentParentLink = currSegment->getParentLink();
			Node *originNode = currSegmentParentLink->getToNode();
			SubTrip currSubTrip;
			currSubTrip.origin = WayPoint(originNode);
			currSubTrip.destination = WayPoint(destinationNode);
			bool useInSimulationTT = parentDriver->getParent()->usesInSimulationTravelTime();
			std::vector<WayPoint> currentRoute = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip,false, nullptr, useInSimulationTT);
		}




		TaxiDriverBehavior::TaxiDriverBehavior()
		{

		}

		TaxiDriverBehavior::~TaxiDriverBehavior()
		{

		}
	}
}

