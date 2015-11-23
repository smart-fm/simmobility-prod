//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#include "DriverFacets.hpp"

#include <limits>
#include <algorithm>

#include "boost/bind.hpp"
#include "BusDriver.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/ST_Config.hpp"
#include "entities/AuraManager.hpp"
#include "entities/Person_ST.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "entities/UpdateParams.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/TurningPath.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/RoadRunnerRegion.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "IncidentPerformer.hpp"
#include "network/CommunicationDataManager.hpp"
#include "path/PathSetManager.hpp"

using namespace sim_mob;
using std::vector;
using std::set;
using std::string;
using std::endl;

//Helper functions
namespace
{
//Helpful constants
const int distanceCheckToChangeLane = 150;
const double METER_TO_CENTIMETER_CONVERT_UNIT = 100;
const double MILLISECS_CONVERT_UNIT = 1000.0;
const double KILOMETER_TO_METER_CONVERT_UNIT = 1000.0;
const double HOUR_TO_SEC_CONVERT_UNIT = 3600.0;
const double KILOMETER_PER_HOUR_TO_METER_PER_SEC = 3.6;
const double DEFAULT_DIS_TO_STOP = 1000;
}

map<const RoadSegment *, unsigned long> DriverMovement::rdSegDensityMap;
boost::mutex DriverMovement::densityUpdateMutex;

DriverMovement::DriverMovement() :
MovementFacet(), parentDriver(nullptr), trafficSignal(NULL), targetLaneIndex(0), lcModel(nullptr), cfModel(nullptr), intModel(nullptr),
intModelBkUp(NULL), targetSpeed(0.0)
{
}

DriverMovement::~DriverMovement()
{
	safe_delete_item(lcModel);
	safe_delete_item(cfModel);
	safe_delete_item(intModel);

	//Usually the metrics for the last sub-trip is not manually finalised
	//if(!travelMetric.finalized)
	//{
	//	finalizeTravelTimeMetric();
	//}
}

void DriverMovement::init()
{
	if (!parentDriver)
	{
		throw runtime_error("Failed to initialise driver movement... Driver does not exist!!!");
	}

	DriverUpdateParams &params = parentDriver->getParams();
	params.parentId = parentDriver->getParent()->getId();

	//Create the driving models
	lcModel = new MITSIM_LC_Model(params, &fwdDriverMovement);
	cfModel = new MITSIM_CF_Model(params);
	intModel = new MITSIM_IntDriving_Model(params);

	parentDriver->initReactionTime();
}

void DriverMovement::frame_init()
{
	Vehicle *vehicle = initializePath(true);

	if (vehicle)
	{
		parentDriver->setVehicle(vehicle);
		parentDriver->setResource(vehicle);
	}
	else
	{
		throw std::runtime_error("No vehicle associated with the driver!");
	}

	if (fwdDriverMovement.isDrivingPathSet())
	{
		setOrigin(parentDriver->getParams());
	}
	else
	{
		throw std::runtime_error("No path found!");
	}
}

void DriverMovement::frame_tick()
{
	DriverUpdateParams &params = parentDriver->getParams();

	//Check if we're done with the route
	if (fwdDriverMovement.isDoneWithEntireRoute())
	{
		if (parentDriver->getParent()->amodId != "-1")
		{
			parentDriver->getParent()->handleAMODArrival();
		}

		parentDriver->getParent()->setToBeRemoved();
		return;
	}
	
	identifyAdjacentLanes(params);

	//If the vehicle is in the loading queue, we need to check if some empty space has opened up.
	if (parentDriver->isVehicleInLoadingQueue && parentDriver->isVehiclePositionDefined)
	{
		//Use the aura manager to find out nearby vehicles. If none of the nearby vehicles on the same lane
		//take up the position that is with-in a particular tolerance of the origin of the current vehicle,
		//then we can set isVehicleInLoadingQueue to false.
		bool isEmptySpaceFound = findEmptySpaceAhead();

		//If an empty space has opened up, remove the vehicle from the queue
		if (isEmptySpaceFound)
		{
			parentDriver->isVehicleInLoadingQueue = false;
		}
	}	

	//Update the "current" time
	unsigned int currentTime = params.now.ms();
	parentDriver->perceivedFwdVel->update(currentTime);
	parentDriver->perceivedFwdAcc->update(currentTime);
	parentDriver->perceivedDistToFwdCar->update(currentTime);
	parentDriver->perceivedVelOfFwdCar->update(currentTime);
	parentDriver->perceivedAccOfFwdCar->update(currentTime);
	parentDriver->perceivedTrafficColor->update(currentTime);
	parentDriver->perceivedDistToTrafficSignal->update(currentTime);

	//Retrieve the current "sensed" values.
	if (parentDriver->perceivedFwdVel->can_sense())
	{
		params.perceivedFwdVelocity = parentDriver->perceivedFwdVel->sense();
	}
	else
	{
		params.perceivedFwdVelocity = parentDriver->vehicle->getVelocity();
	}

	//General update behaviour.
	if (parentDriver->isVehicleInLoadingQueue == false)
	{
		if (updateSensors() && updateMovement() && updatePostMovement())
		{
			//Update parent data.
			setParentBufferedData();
		}
	}

	//Update our Buffered types
	parentDriver->distCoveredOnCurrWayPt_.set(fwdDriverMovement.getDistCoveredOnCurrWayPt());
	parentDriver->isInIntersection_.set(fwdDriverMovement.isInIntersection());
	parentDriver->currLane_.set(fwdDriverMovement.getCurrLane());
	parentDriver->currTurning_.set(fwdDriverMovement.getCurrTurning());
	
	parentDriver->latMovement_.set(parentDriver->vehicle->getLateralMovement());
	parentDriver->fwdVelocity_.set(parentDriver->vehicle->getVelocity());
	parentDriver->latVelocity_.set(parentDriver->vehicle->getLateralVelocity());
	parentDriver->fwdAccel_.set(parentDriver->vehicle->getAcceleration());
	parentDriver->turningDirection_.set(parentDriver->vehicle->getTurningDirection());

	//Update your perceptions
	parentDriver->perceivedFwdVel->delay(parentDriver->vehicle->getVelocity());
	parentDriver->perceivedFwdAcc->delay(parentDriver->vehicle->getAcceleration());
	
	Point position = getPosition();
	parentDriver->setCurrPosition(position);
	parentDriver->vehicle->setCurrPosition(position);

	setParentBufferedData();
	parentDriver->isVehiclePositionDefined = true;

	//Clear the NearestVehicles list in the conflictTurnings
	params.conflictVehicles.clear();
}

bool DriverMovement::findEmptySpaceAhead()
{
	bool isSpaceFound = true;

	DriverUpdateParams &driverUpdateParams = parentDriver->getParams();

	//To store the agents that are in the nearby region
	vector<const Agent *> nearby_agents;

	//To store the closest driver approaching from the rear, if any
	//This is a pair of the driver object and his/her gap from the driver looking to exit the loading
	//queue
	pair<Driver *, double> driverApproachingFromRear(NULL, DBL_MAX);

	//Get the agents in nearby the current vehicle
	WayPoint wayPoint(fwdDriverMovement.getCurrLane());
	nearby_agents = AuraManager::instance().nearbyAgents(parentDriver->getCurrPosition(), wayPoint, distanceInFront, distanceBehind, NULL);

	//Now if a particular agent is a vehicle and is in the same lane as the one we want to get into
	//then we have to check if it's occupying the space we need
	for (vector<const Agent *>::iterator itAgents = nearby_agents.begin(); itAgents != nearby_agents.end(); ++itAgents)
	{
		//We only need to only process agents those are vehicle drivers - this means that they are of type Person
		//and have role as driver or bus driver
		const Person_ST *person = dynamic_cast<const Person_ST *> (*itAgents);

		if (person != NULL)
		{
			Role<Person_ST> *role = person->getRole();
			if (role != NULL)
			{
				if (role->roleType == Role<Person_ST>::RL_DRIVER || role->roleType == Role<Person_ST>::RL_BUSDRIVER)
				{
					Driver *nearbyDriver = dynamic_cast<Driver *> (role);
					DriverUpdateParams &nearbyDriversParams = nearbyDriver->getParams();

					//Make sure we're not checking distance from ourselves or someone in the loading queue
					//also ensure that the other vehicle is in our lane
					if (parentDriver != nearbyDriver && nearbyDriver->isVehicleInLoadingQueue == false &&
							driverUpdateParams.currLane == nearbyDriversParams.currLane)
					{
						DriverMovement *nearbyDriverMovement = dynamic_cast<DriverMovement *> (nearbyDriver->Movement());

						//Get the gap to the nearby driver
						double availableGap = fwdDriverMovement.getDistToEndOfCurrWayPt() - nearbyDriverMovement->fwdDriverMovement.getDistToEndOfCurrWayPt();

						//The gap between current driver and the one in front (or the one coming from behind) should be greater than
						//length(in m) + (headway(in s) * initial speed(in m/s))
						double requiredGap = 0;
						if (availableGap > 0)
						{
							//As the gap is positive, there is a vehicle in front of us. We should have enough distance
							//so as to avoid crashing into it
							MITSIM_CF_Model *mitsim_cf_model = dynamic_cast<MITSIM_CF_Model *> (cfModel);
							requiredGap = (2 * parentDriver->getVehicleLength()) + (mitsim_cf_model->getHBufferUpper() * driverUpdateParams.initialSpeed);
						}
						else
						{
							//As the gap is negative, there is a vehicle coming in from behind. We shouldn't appear right
							//in front of it, so consider it's speed to calculate required gap
							MITSIM_CF_Model *mitsim_cf_model = dynamic_cast<MITSIM_CF_Model *> (nearbyDriverMovement->cfModel);
							requiredGap = (2 * nearbyDriver->getVehicleLength())+ (mitsim_cf_model->getHBufferUpper() * nearbyDriversParams.currSpeed);

							//In case a driver is approaching from the rear, we need to reduce the reaction time, so that he/she
							//is aware of the presence of the car appearing in front.
							//But we need only the closest one
							if (driverApproachingFromRear.second > availableGap)
							{
								driverApproachingFromRear.first = nearbyDriver;
								driverApproachingFromRear.second = availableGap;
							}
						}

						if (abs(availableGap) <= abs(requiredGap))
						{
							//at least one vehicle is too close, so no need to search further
							isSpaceFound = false;

							//If any driver was added to the pair - driverApproachingFromRear, remove it
							//as we're not going to unload the vehicle from the loading queue
							driverApproachingFromRear.first = NULL;
							driverApproachingFromRear.second = DBL_MAX;

							break;
						}
					}
				}
			}
		}
	}

	//If is any driver approaching from behind (also means that we've found space on the road), 
	//reduce the reaction time
	if (driverApproachingFromRear.first != NULL)
	{
		float alert = CF_CRITICAL_TIMER_RATIO * cfModel->updateStepSize[0];
		driverApproachingFromRear.first->getParams().reactionTimeCounter = std::min<double>(alert, driverApproachingFromRear.first->getParams().reactionTimeCounter);
	}

	return isSpaceFound;
}

std::string DriverMovement::frame_tick_output()
{
	DriverUpdateParams &params = parentDriver->getParams();
	std::stringstream output;
	output << std::setprecision(8);

	//Skip
	if (parentDriver->isVehicleInLoadingQueue || fwdDriverMovement.isDoneWithEntireRoute())
	{
		return std::string();
	}

	if (ConfigManager::GetInstance().CMakeConfig().OutputDisabled())
	{
		return std::string();
	}

	double baseAngle = getAngle();

	//Inform the GUI if interactive mode is active.
	//if (ConfigManager::GetInstance().CMakeConfig().InteractiveMode())
	//{
	//	std::ostringstream stream;
	//	stream << "DriverSegment" << "," << params.now.frame() << ","
	//			<< fwdDriverMovement.getCurrSegment() << ","
	//			<< fwdDriverMovement.getCurrSegment()->getLength();
	//
	//	std::string s = stream.str();
	//	ConfigManager::GetInstance().FullConfig().getCommDataMgr().sendTrafficData(s);
	//}

	const int wayPtId = fwdDriverMovement.isInIntersection() ?
			fwdDriverMovement.getCurrTurning()->getTurningGroupId() : fwdDriverMovement.getCurrSegment()->getRoadSegmentId();

	//MPI-specific output.
	std::stringstream addLine;
	if (ConfigManager::GetInstance().FullConfig().using_MPI)
	{
		addLine << "\",\"fake\":\"" << (parentDriver->getParent()->isFake ? "true" : "false");
	}

	std::stringstream id;

	if (parentDriver->getParent()->amodId != "-1")
	{
		id << parentDriver->getParent()->amodTripId;
		params.debugInfo = params.debugInfo + "<AMOD>";
	}
	else
	{
		id << parentDriver->getParent()->GetId();
		
		//Check if the trip mode is taxi, if so append <Taxi> to debug info,
		//otherwise it means it is a private vehicle
		TripChainItem *tripChainItem = *(parentDriver->getParent()->currTripChainItem);

		if (tripChainItem->travelMode.compare("Taxi") == 0)
		{
			params.debugInfo = params.debugInfo + "<Taxi>";
		}
	}

	output << "(\"Driver\"" << "," << params.now.frame() << "," << id.str()
			<< ",{" << "\"xPos\":\"" << parentDriver->getCurrPosition().getX()
			<< "\",\"yPos\":\"" << parentDriver->getCurrPosition().getY()
			<< "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI))
			<< "\",\"length\":\"" << static_cast<int> (parentDriver->vehicle->getLengthInM())
			<< "\",\"width\":\"" << static_cast<int> (parentDriver->vehicle->getWidthInM())
			<< "\",\"curr-waypoint\":\"" << wayPtId
			<< "\",\"fwd-speed\":\"" << parentDriver->vehicle->getVelocity()
			<< "\",\"fwd-accel\":\"" << parentDriver->vehicle->getAcceleration()
			<< "\",\"info\":\"" << params.debugInfo
			<< "\",\"mandatory\":\"" << incidentPerformer.getIncidentStatus().getChangedLane()
			<< addLine.str() << "\"})" << std::endl;
	
	return output.str();
}

void DriverMovement::updateDensityMap()
{
	const RoadSegment *currSeg = fwdDriverMovement.getCurrSegment();
	
	if(currSeg)
	{
		//The density map is a static map, so all threads will want to access it. Lock before accessing.
		densityUpdateMutex.lock();

		//Find the entry for the road segment corresponding to the current vehicles segment
		map<const RoadSegment *, unsigned long>::iterator itDensityMap = rdSegDensityMap.find(currSeg);

		//Check if an entry exists
		if (itDensityMap != rdSegDensityMap.end())
		{
			//Increment the number of vehicles on the segment
			itDensityMap->second += 1;
		}
		else
		{
			//Entry not found, so create a new one
			rdSegDensityMap.insert(make_pair(currSeg, 1));
		}

		//Done with update to the map, unlock.
		densityUpdateMutex.unlock();
	}
}

void DriverMovement::outputDensityMap(unsigned int tick)
{
	const ST_Config &config = ST_Config::getInstance();
	const ConfigParams &cfg = ConfigManager::GetInstance().FullConfig();

	//Get the logger instance
	BasicLogger &logger = Logger::log(config.segDensityMap.fileName);

	//Iterator to access all elements in the map
	map<const RoadSegment *, unsigned long>::iterator itDensityMap = rdSegDensityMap.begin();

	//Iterate through all elements in the map
	while (itDensityMap != rdSegDensityMap.end())
	{
		//Get collection time
		unsigned int period = config.segDensityMap.updateInterval / cfg.baseGranMS();

		//Get the average vehicle count
		double avgVehCount = (double) itDensityMap->second / period;

		//Convert the segment length to km from cm
		double segLength = itDensityMap->first->getLength() / 100000;

		unsigned int noOfLanes = itDensityMap->first->getNoOfLanes();

		//Calculate density. The unit is no of vehicles per lane-km
		double density = avgVehCount / (noOfLanes * segLength);

		logger << tick << "," << itDensityMap->first->getRoadSegmentId() << "," << density << "\n";

		++itDensityMap;
	}

	//Clear the map
	rdSegDensityMap.clear();
}

TravelMetric& DriverMovement::startTravelTimeMetric()
{
	travelMetric.startTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
	const Node *startNode = parentDriver->getParent()->originNode.node;
	
	if (!startNode)
	{
		throw std::runtime_error("Unknown Origin Node");
	}
	
	travelMetric.origin = WayPoint(startNode);
	travelMetric.started = true;
	return travelMetric;
}

TravelMetric& DriverMovement::finalizeTravelTimeMetric()
{
	if (!travelMetric.started)
	{
		return travelMetric;
	}

	const Node *endNode = parentDriver->getParent()->destNode.node;
	travelMetric.destination = WayPoint(endNode);
	travelMetric.endTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
	travelMetric.travelTime = (travelMetric.endTime - travelMetric.startTime).getValue();
	travelMetric.finalized = true;
	//parent->addSubtripTravelMetrics(*travelMetric);

	return travelMetric;
}

bool DriverMovement::updateSensors()
{
	DriverUpdateParams& params = parentDriver->getParams();
	
	if (fwdDriverMovement.isDoneWithEntireRoute())
	{
		return false;
	}

	//Manage traffic signal behaviour if we are close to the end of the link.
	if (!fwdDriverMovement.isInIntersection())
	{
		setTrafficSignalParams(params);
	}

	//Identify the nearby drivers and their positions
	updateNearbyAgents();

	//Get the nearest car, if not making a lane change, the nearest car should be the leading car in current lane.
	//if making lane changing, adjacent car need to be taken into account.
	perceivedDataProcess(params.nvFwd, params);

	return true;
}

bool DriverMovement::updateMovement()
{
	DriverUpdateParams &params = parentDriver->getParams();

	if (fwdDriverMovement.isDoneWithEntireRoute())
	{
		return false;
	}
	
	//Store the speed
	params.currSpeed = parentDriver->vehicle->getVelocity();
	
	//Count down the reaction timer
	params.reactionTimeCounter -= params.elapsedSeconds;
	
	//Check if the reaction timer has expired, if so apply the driving models and move forward
	if(params.reactionTimeCounter < params.elapsedSeconds)
	{
		applyDrivingModels(params);		
	}
	
	params.overflowIntoIntersection = drive(params);

	//Update the road segment density map
	if (ST_Config::getInstance().segDensityMap.outputEnabled)
	{
		updateDensityMap();
	}

	//Build debugging information to be displayed on the visualiser
	params.buildDebugInfo();
	
	return true;
}

bool DriverMovement::updatePostMovement()
{
	DriverUpdateParams &params = parentDriver->getParams();

	if (fwdDriverMovement.isDoneWithEntireRoute())
	{
		return false;
	}

	if (isLastSegmentInLink())
	{
		setTrafficSignal();
	}

	params.isApproachingIntersection = false;
	parentDriver->expectedTurning_.set(NULL);
	
	//Detect if a vehicle is approaching an intersection. We do this by comparing the 
	//distance to the end of the road segment and the visibility distance

	//The distance to the end of the road segment (metre)
	double distToIntersection = fwdDriverMovement.getDistToEndOfCurrLink();
	parentDriver->distToIntersection_.set(distToIntersection);
	
	const WayPoint *nextWayPt = fwdDriverMovement.getNextWayPoint();
	
	if(nextWayPt && nextWayPt->type == WayPoint::TURNING_GROUP)
	{
		//Check if the turning group is visible
		if (distToIntersection <= nextWayPt->turningGroup->getVisibility())
		{			
			//The current lane
			const Lane *currLane = fwdDriverMovement.getCurrLane();
			
			//If we have a current lane, it means that we're approaching the intersection but not yet inside it
			if (currLane)
			{
				//The turning path we will mostly take to get across the intersection
				const TurningPath *expectedTurning = fwdDriverMovement.getNextTurning();

				if (expectedTurning)
				{
					params.isApproachingIntersection = true;

					//Set the max turning speed
					params.maxLaneSpeed = expectedTurning->getMaxSpeed();

					//Add it to the buffer, it will be available in the next tick
					parentDriver->expectedTurning_.set(expectedTurning);
				}
			}
			else
			{
				//We're in the intersection, so we can directly use the current turning
				
				//Set the max turning speed
				params.maxLaneSpeed = fwdDriverMovement.getCurrTurning()->getMaxSpeed();
			}
		}
	}

	return true;
}

void DriverMovement::checkForStoppingPoints(DriverUpdateParams &params)
{
	//Get the distance to stopping point in the current link
	double distance = getDistanceToStopPoint(params.stopVisibilityDistance);

	if (parentDriver->getParent()->amodId != "-1" && abs(distance) < 50)
	{
		parentDriver->getParent()->handleAMODPickup();
	}

	params.distanceToStoppingPt = distance;

	if (distance > -10 || params.stopPointState == DriverUpdateParams::ARRIVED_AT_STOP_POINT)
	{
		//Leaving the stopping point
		if (distance < 0 && params.stopPointState == DriverUpdateParams::LEAVING_STOP_POINT)
		{
			return;
		}
		
		//Change state to Approaching stop point
		if (params.stopPointState == DriverUpdateParams::STOP_POINT_NOT_FOUND)
		{
			params.stopPointState = DriverUpdateParams::STOP_POINT_FOUND;
		}
		
		//Change state to stopping point is close
		if (distance >= 10 && distance <= 50)
		{ 
			// 10m-50m
			params.stopPointState = DriverUpdateParams::ARRIVING_AT_STOP_POINT;
		}
		
		//Change state to arrived at stop point
		if (params.stopPointState == DriverUpdateParams::ARRIVING_AT_STOP_POINT && abs(distance) < 10)
		{ 
			// 0m-10m
			params.stopPointState = DriverUpdateParams::ARRIVED_AT_STOP_POINT;
		}

		params.distToStop = distance;
		
		return;
	}
	
	if (distance < -10 && params.stopPointState == DriverUpdateParams::LEAVING_STOP_POINT)
	{
		params.stopPointState = DriverUpdateParams::STOP_POINT_NOT_FOUND;
	}
}

void DriverMovement::applyDrivingModels(DriverUpdateParams &params)
{
	params.lcDebugStr.str(std::string());

	perceiveParameters(params);	

	//Currently on AMOD and Buses have stop points, so at the moment calls to check for stop point
	//for private cars and taxis will be a burden.
	if (parentDriver->getParent()->amodId != "-1" || parentDriver->IsBusDriver())
	{
		checkForStoppingPoints(params);
	}

	if(!fwdDriverMovement.isInIntersection())
	{
		//Apply the lane changing model to make the lane changing decision
		lcModel->makeLaneChangingDecision(params);

		//If we've decided to change the lane, execute the lane change manoeuvre
		if (params.getStatus() & STATUS_CHANGING)
		{
			params.lcDebugStr << ";CHING";

			lcModel->executeLaneChanging(params);

			if (params.flag(FLAG_LC_FAILED))
			{
				params.lcDebugStr << ";COG";
				lcModel->chooseTargetGap(params);
			}
		}
	}

	double intDrivingAcc = DBL_MAX, carFollowingAcc = DBL_MAX;

	//Check if this vehicle is approaching an unsignalised intersection.
	if (trafficSignal == NULL && (params.isApproachingIntersection || fwdDriverMovement.isInIntersection()))
	{
		parentDriver->setYieldingToInIntersection(-1);

		if (params.isApproachingIntersection)
		{
			const TurningPath *turningPath = parentDriver->expectedTurning_.get();

			if (turningPath)
			{
				//Reset the impatience timer
				params.impatienceTimer = params.impatienceTimerStart = 0;
				intModel->setCurrTurning(turningPath);
				intDrivingAcc = intModel->makeAcceleratingDecision(params);
			}
		}
		else
		{
			intModel->setCurrTurning(fwdDriverMovement.getCurrTurning());
			intDrivingAcc = intModel->makeAcceleratingDecision(params);
		}
	}

	//Apply the car following model to make the accelerating decision
	carFollowingAcc = cfModel->makeAcceleratingDecision(params);

	//Select the lower of the two accelerations
	if (carFollowingAcc < intDrivingAcc)
	{
		params.acceleration = carFollowingAcc;
		parentDriver->setYieldingToInIntersection(-1);
	}
	else
	{
		params.acc = params.acceleration = intDrivingAcc;
		params.accSelect = "aI";
	}
}

void DriverMovement::perceiveParameters(DriverUpdateParams &params)
{
	if (parentDriver->perceivedVelOfFwdCar->can_sense() && parentDriver->perceivedAccOfFwdCar->can_sense() && parentDriver->perceivedDistToFwdCar->can_sense())
	{
		params.perceivedFwdVelocityOfFwdCar = parentDriver->perceivedVelOfFwdCar->sense();
		params.perceivedAccelerationOfFwdCar = parentDriver->perceivedAccOfFwdCar->sense();
		params.perceivedDistToFwdCar = parentDriver->perceivedDistToFwdCar->sense();

	}
	else
	{
		NearestVehicle &nv = params.nvFwd;
		params.perceivedFwdVelocityOfFwdCar = nv.driver ? nv.driver->fwdVelocity_.get() : 0;
		params.perceivedLatVelocityOfFwdCar = nv.driver ? nv.driver->latVelocity_.get() : 0;
		params.perceivedAccelerationOfFwdCar = nv.driver ? nv.driver->fwdAccel_.get() : 0;
		params.perceivedDistToFwdCar = nv.distance;
	}

	if (parentDriver->perceivedTrafficColor->can_sense())
	{
		params.perceivedTrafficColor = parentDriver->perceivedTrafficColor->sense();
	}

	if (parentDriver->perceivedDistToTrafficSignal->can_sense())
	{
		params.perceivedDistToTrafficSignal = parentDriver->perceivedDistToTrafficSignal->sense();
	}
}

double DriverMovement::drive(DriverUpdateParams &params)
{
	LaneChangeTo laneChangeTo;

	if (params.getStatus(STATUS_LC_RIGHT))
	{
		laneChangeTo = LANE_CHANGE_TO_RIGHT;
	}
	else if (params.getStatus(STATUS_LC_LEFT))
	{
		laneChangeTo = LANE_CHANGE_TO_LEFT;
	}
	else
	{
		laneChangeTo = LANE_CHANGE_TO_NONE;
	}

	params.lateralVelocity = lcModel->calculateLateralVelocity(laneChangeTo);

	parentDriver->vehicle->setTurningDirection(laneChangeTo);
	parentDriver->vehicle->setLateralVelocity(params.lateralVelocity);
	parentDriver->vehicle->setAcceleration(params.acceleration);

	return updatePosition(params);
}

void DriverMovement::setParentBufferedData()
{
	parentDriver->getParent()->xPos.set(parentDriver->getCurrPosition().getX());
	parentDriver->getParent()->yPos.set(parentDriver->getCurrPosition().getY());
}

void DriverMovement::buildPath(std::vector<WayPoint> &wayPoints, int startLaneIndex, int startSegmentId)
{
	//Path containing only links
	vector<WayPoint> pathOfLinks;

	//Filter out the nodes and only add the links
	for (vector<WayPoint>::iterator itWayPts = wayPoints.begin(); itWayPts != wayPoints.end(); ++itWayPts)
	{
		if (itWayPts->type == WayPoint::LINK)
		{
			pathOfLinks.push_back(*itWayPts);
		}
	}
	
	//The path containing the links and turning groups
	vector<WayPoint> path;
	
	//Add the road segments and turning groups that lie along the links in the path
	for (vector<WayPoint>::iterator itWayPts = pathOfLinks.begin(); itWayPts != pathOfLinks.end(); ++itWayPts)
	{
		//The segments in the link
		const vector<RoadSegment *> &segments = itWayPts->link->getRoadSegments();

		//Create a way point for every segment and insert it into the path
		for (vector<RoadSegment *>::const_iterator itSegments = segments.begin(); itSegments != segments.end(); ++itSegments)
		{
			path.push_back(WayPoint(*itSegments));			
		}

		if((itWayPts + 1) != pathOfLinks.end())
		{
			unsigned int currLink = itWayPts->link->getLinkId();
			unsigned int nextLink = (itWayPts + 1)->link->getLinkId();

			//Get the turning group between this link and the next link and add it to the path

			const TurningGroup *turningGroup = itWayPts->link->getToNode()->getTurningGroup(currLink, nextLink);

			if (turningGroup)
			{
				path.push_back(WayPoint(turningGroup));
			}
			else
			{
				stringstream msg;
				msg << "No turning between the links " << currLink << " and " << nextLink << "!\nInvalid Path!!!";
				throw std::runtime_error(msg.str());
			}
		}				
	}

	fwdDriverMovement.setPath(path, startLaneIndex, startSegmentId);
}

void DriverMovement::resetPath(std::vector<WayPoint> path)
{
	buildPath(path);
}

bool DriverMovement::isLastSegmentInLink() const
{
	bool isLastSegInLink = false;
	
	if (!fwdDriverMovement.isDoneWithEntireRoute())
	{
		const WayPoint *nextWayPt = fwdDriverMovement.getNextWayPoint();
		
		//If there is no next way point (we're on the last link (therefore the last segment) in the path) or 
		//the next way point is a turning group, this is the last segment in the link
		if(nextWayPt == NULL || nextWayPt->type == WayPoint::TURNING_GROUP)
		{
			isLastSegInLink = true;
		}
	}
	
	return isLastSegInLink;
}

Point DriverMovement::getPosition()
{
	Point origPos = fwdDriverMovement.getPosition();

	if (parentDriver->vehicle->getLateralMovement() != 0 && !fwdDriverMovement.isDoneWithEntireRoute())
	{
		DynamicVector lateralMovement(0, 0, fwdDriverMovement.getNextPolyPoint().getX() - fwdDriverMovement.getCurrPolyPoint().getX(), 
									 fwdDriverMovement.getNextPolyPoint().getY() - fwdDriverMovement.getCurrPolyPoint().getY());

		lateralMovement.flipLeft();
		lateralMovement.scaleVectTo(parentDriver->vehicle->getLateralMovement()).translateVect();

		origPos.setX(origPos.getX() + lateralMovement.getX());
		origPos.setY(origPos.getY() + lateralMovement.getY());
	}

	return origPos;
}

double DriverMovement::getDistanceToStopPoint(double perceptionDistance)
{
	//Distance to stopping point
	double distance = -100;
	
	//Distance to the end of the current way-point
	double distToEndOfWayPt = fwdDriverMovement.getDistToEndOfCurrWayPt();	
	
	bool isStopPointFound = false;
	
	DriverUpdateParams &params = parentDriver->getParams();
	
	std::vector<WayPoint>::const_iterator wayPtIt = fwdDriverMovement.getCurrWayPointIt();
	std::vector<WayPoint>::const_iterator endOfPath = fwdDriverMovement.getDrivingPath().end();
	
	//Check for stop-point in the current segment
	
	//Distance along which we have scanned for the stopping points
	double scannedDist = distToEndOfWayPt;
	
	//Find the stop points in the current segment
	std::map<unsigned int, std::vector<StopPoint> >::iterator itStopPtPool = params.stopPointPool.find(wayPtIt->roadSegment->getRoadSegmentId());	
	
	//Check if the current segment has any stop-points
	if(itStopPtPool != params.stopPointPool.end())
	{
		double distCovered = fwdDriverMovement.getDistCoveredOnCurrWayPt();
		
		//Look for the first stop-point in front of us, but within the perception distance
		std::vector<StopPoint>::const_iterator itStopPts = itStopPtPool->second.begin();
		while(itStopPts != itStopPtPool->second.end() && (itStopPts->distance - distCovered) <= perceptionDistance)
		{
			if(itStopPts->distance >= distCovered)
			{
				params.currentStopPoint = *itStopPts;
				distance = params.currentStopPoint.distance - distCovered;
				isStopPointFound = true;
				break;
			}
			
			++itStopPts;
		}
	}
	
	//The next way-point to be scanned (Note: if the stop-point has already been found, the next loop won't execute)
	++wayPtIt;
	
	//Iterate through the path till the perception distance or the end (whichever is before)
	while(wayPtIt != endOfPath && scannedDist < perceptionDistance && !isStopPointFound)
	{
		//Stopping points are only on road segments, so skip the intersections (turnings)
		//but include their lengths in the scanned distance
		if(wayPtIt->type == WayPoint::ROAD_SEGMENT)
		{			
			//Look for the stop points in the road segment
			itStopPtPool = params.stopPointPool.find(wayPtIt->roadSegment->getRoadSegmentId());
			
			if(itStopPtPool != params.stopPointPool.end())
			{
				std::vector<StopPoint>::const_iterator itStopPts = itStopPtPool->second.begin();
				
				while(itStopPts != itStopPtPool->second.end())
				{
					if(scannedDist + itStopPts->distance <= perceptionDistance)
					{
						params.currentStopPoint = *itStopPts;
						distance = scannedDist + itStopPts->distance;
						isStopPointFound = true;
						break;
					}
					
					++itStopPts;
				}
			}
			
			distToEndOfWayPt = wayPtIt->roadSegment->getLength();
		}
		else
		{
			//Add the length of the turning group
			distToEndOfWayPt = wayPtIt->turningGroup->getLength();
		}
		
		scannedDist += distToEndOfWayPt;
		++wayPtIt;
	}
	
	return distance;
}

bool DriverMovement::isLaneConnectedToSegment(const Lane *fromLane, const RoadSegment *toSegment)
{
	bool isLaneConnected = false;
	const RoadSegment *fromSegment = fromLane->getParentSegment();
	
	//Check if the the from and to segments are in the same link
	if(fromSegment->getLinkId() == toSegment->getLinkId())
	{
		//They're in the same link. Use the lane connector to check the connection
		
		std::vector<const LaneConnector *> connectors;
		fromLane->getPhysicalConnectors(connectors);
		
		std::vector<const LaneConnector *>::const_iterator itConnectors = connectors.begin();
		
		while(itConnectors != connectors.end())
		{
			if ((*itConnectors)->getFromRoadSegmentId() == toSegment->getRoadSegmentId())
			{
				isLaneConnected = true;
				break;
			}
			++itConnectors;
		}
	}
	else
	{
		//They're in different links.
		//Get the turning group and check if there is a turning path from the given lane to the given segment
		
		unsigned int fromLink = fromSegment->getLinkId();
		unsigned int toLink = toSegment->getLinkId();
		const TurningGroup *turningGroup = fromSegment->getParentLink()->getToNode()->getTurningGroup(fromLink, toLink);
		
		if(turningGroup)
		{
			//The turning path from the given lane
			const std::map<unsigned int, TurningPath *> *turnings = turningGroup->getTurningPaths(fromLane->getLaneId());
			
			if(turnings)
			{
				for(std::map<unsigned int, TurningPath *>::const_iterator itTurnings = turnings->begin(); itTurnings != turnings->end(); ++itTurnings)
				{
					if(itTurnings->second->getToLane()->getRoadSegmentId() == toSegment->getRoadSegmentId())
					{
						isLaneConnected = true;
						break;
					}
				}
			}
		}
	}

	return isLaneConnected;
}

double DriverMovement::dwellTimeCalculation(int A, int B, int delta_bay, int delta_full, int Pfront, int no_of_passengers)
{
	//assume single channel passenger movement
	double alpha1 = 2.1; //alighting passenger service time,assuming payment by smart card
	double alpha2 = 3.5; //boarding passenger service time,assuming alighting through rear door
	double alpha3 = 3.5; //door opening and closing times
	double alpha4 = 1.0;
	double beta1 = 0.7; //fixed parameters
	double beta2 = 0.7;
	double beta3 = 5;
	double DTijk = 0.0;
	bool bus_crowdness_factor;
	int no_of_seats = 40;

	//People are standing
	if (no_of_passengers > no_of_seats)
	{
		//boarding time increase if people are standing
		alpha1 += 0.5;
	}

	if (no_of_passengers > no_of_seats)
	{
		bus_crowdness_factor = 1;
	}
	else
	{
		bus_crowdness_factor = 0;
	}

	double PTijk_front = alpha1 * Pfront * A + alpha2 * B + alpha3 * bus_crowdness_factor * B;
	double PTijk_rear = alpha4 * (1 - Pfront) * A;
	double PT;

	PT = std::max(PTijk_front, PTijk_rear);
	DTijk = beta1 + PT + beta2 * delta_bay + beta3 * delta_full;
	Print() << "Dwell__time " << DTijk << std::endl;

	return DTijk;
}

void DriverMovement::identifyAdjacentLanes(DriverUpdateParams &params)
{
	params.leftLane = NULL;
	params.rightLane = NULL;
	params.leftLane2 = NULL;
	params.rightLane2 = NULL;

	params.currLane = fwdDriverMovement.getCurrLane();	
	
	if (fwdDriverMovement.isInIntersection())
	{
		return;
	}
	
	params.currLaneIndex = params.currLane->getLaneIndex();
	const unsigned int numOfLanes = params.currLane->getParentSegment()->getNoOfLanes();

	//Only 1 lane in the segment, so no adjacent lanes
	if (numOfLanes == 1)
	{
		return;
	}

	if (params.currLaneIndex > 0)
	{
		const Lane *temp = params.currLane->getParentSegment()->getLane(params.currLaneIndex - 1);

		if (!temp->isPedestrianLane())
		{
			params.leftLane = temp;
		}
	}

	if (params.currLaneIndex > 1)
	{
		const Lane *temp = params.currLane->getParentSegment()->getLane(params.currLaneIndex - 2);

		if (!temp->isPedestrianLane())
		{
			params.leftLane2 = temp;
		}
	}

	if (params.currLaneIndex + 1 < numOfLanes)
	{
		const Lane *temp = params.currLane->getParentSegment()->getLane(params.currLaneIndex + 1);

		if (!temp->isPedestrianLane())
		{
			params.rightLane = temp;
		}
	}

	if (params.currLaneIndex + 2 < numOfLanes)
	{
		const Lane *temp = params.currLane->getParentSegment()->getLane(params.currLaneIndex + 2);

		if (!temp->isPedestrianLane())
		{
			params.rightLane2 = temp;
		}
	}
	
	//Update max lane speed
	params.maxLaneSpeed = fwdDriverMovement.getCurrSegment()->getMaxSpeed();
}

Vehicle* DriverMovement::initializePath(bool createVehicle)
{
	Vehicle *vehicle = NULL;

	//Only initialise if the next path has not been planned for yet.
	if (!parentDriver->getParent()->getNextPathPlanned())
	{
		//Save local copies of the parent's origin/destination nodes.
		parentDriver->origin = parentDriver->getParent()->originNode.node;
		parentDriver->destination = parentDriver->getParent()->destNode.node;

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> path;

		//Get the path from the path-set manager if we're using route-choice, else find the shortest path
		if (ConfigManager::GetInstance().FullConfig().PathSetMode())
		{
			path = PrivateTrafficRouteChoice::getInstance()->getPath(*(parentDriver->getParent()->currSubTrip), false, nullptr);
		}
		else
		{
			const StreetDirectory& stdir = StreetDirectory::Instance();
			path = stdir.SearchShortestDrivingPath(*(parentDriver->origin), *(parentDriver->destination));
		}

		const double length = 4.0;
		const double width = 2.0;

		if (createVehicle)
		{
			vehicle = new Vehicle(VehicleBase::CAR, length, width);
			buildPath(path, parentDriver->getParent()->startLaneIndex, parentDriver->getParent()->startSegmentId);
		}
	}

	//Indicate that the path to next activity is planned
	parentDriver->getParent()->setNextPathPlanned(true);
	return vehicle;
}

void DriverMovement::rerouteWithPath(const std::vector<WayPoint> &path)
{
	//Pre-pend the current segment, and reset path
	//NOTE: This will put the current driver back onto the start of the current Segment
	std::vector<WayPoint> prependedPath = path;
	prependedPath.insert(prependedPath.begin(), fwdDriverMovement.getCurrWayPoint());
	resetPath(prependedPath);
}

void DriverMovement::rerouteWithBlacklist(const std::vector<const Link *> &blacklisted)
{
	if (!(parentDriver && parentDriver->vehicle && fwdDriverMovement.getCurrWayPoint().type != WayPoint::INVALID))
	{
		return;
	}

	//Retrieve the shortest path from the current intersection node to destination and save all RoadSegments in this path.
	//NOTE: This path may be invalid, if there is no turning from the current link to the first link of the resultant path.
	WayPoint currWayPt = fwdDriverMovement.getCurrWayPoint();
	const Node *node;
	
	if(currWayPt.type == WayPoint::ROAD_SEGMENT)
	{
		node = currWayPt.roadSegment->getParentLink()->getToNode();
	}
	else
	{
		const RoadNetwork *network = RoadNetwork::getInstance();
		node = network->getById(network->getMapOfIdvsNodes(), currWayPt.turningGroup->getNodeId());
	}
	
	const StreetDirectory& stdir = StreetDirectory::Instance();
	vector<WayPoint> path = stdir.SearchShortestDrivingPath(*node, *(parentDriver->destination), blacklisted);	

	//Pre-pend the current segment, and reset the current driver.
	//NOTE: This will put the current driver back onto the start of the segment.
	path.insert(path.begin(), fwdDriverMovement.getCurrWayPoint());
	resetPath(path);
}

void DriverMovement::setOrigin(DriverUpdateParams &params)
{
	//Set the max speed and target speed
	params.maxLaneSpeed = fwdDriverMovement.getCurrSegment()->getMaxSpeed();
	params.desiredSpeed = targetSpeed = params.maxLaneSpeed;

	//Set the current and target lanes.
	params.currLane = fwdDriverMovement.getCurrLane();
	
	if (params.currLane)
	{
		targetLaneIndex = params.currLaneIndex = params.currLane->getLaneIndex();
	}

	//Vehicles start at rest (or may be given initial speed in configuration file)
	parentDriver->vehicle->setVelocity(parentDriver->getParent()->initialSpeed);
	parentDriver->vehicle->setLateralVelocity(0);
	parentDriver->vehicle->setAcceleration(0);

	setTrafficSignal();
}

double DriverMovement::updatePosition(DriverUpdateParams &params)
{
	//Determine the distance covered using the equation of motion
	//s = ut + (1/2)at^2
	//where, s = displacement, u = initial velocity, a = acceleration, t = time
	double distCovered = (params.currSpeed * params.elapsedSeconds) + (0.5 * params.acceleration *	params.elapsedSeconds * params.elapsedSeconds);

	if (distCovered < 0)
	{
		distCovered = 0;
	}

	//Update the vehicle's velocity based on its acceleration using the equation of motion
	//v = u + at
	//where, v = final velocity, u = initial velocity, a = acceleration, t = time
	double updatedVelocity = params.currSpeed + params.acceleration * params.elapsedSeconds;

	if (updatedVelocity < 0)
	{
		updatedVelocity = 0;
	}

	parentDriver->vehicle->setVelocity(updatedVelocity);

	//Move the vehicle forward
	double overflow = fwdDriverMovement.advance(distCovered);

	identifyAdjacentLanes(params);

	//Check if the lane which we want to move into exists
	if ((parentDriver->vehicle->getTurningDirection() == LANE_CHANGE_TO_LEFT && !params.leftLane) 
			|| (parentDriver->vehicle->getTurningDirection() == LANE_CHANGE_TO_RIGHT && !params.rightLane))
	{
		parentDriver->vehicle->setLateralVelocity(0);
		params.lateralVelocity = 0.0;
	}

	//Lateral movement
	if (!(fwdDriverMovement.isInIntersection()))
	{
		updateLateralMovement(params);
	}

	return overflow;
}

void DriverMovement::setNearestVehicle(NearestVehicle &nearestVeh, double distance, const Driver *otherDriver)
{
	//Subtract the size of the car from the distance between them
	distance = fabs(distance) - parentDriver->getVehicleLength() / 2 - otherDriver->getVehicleLength() / 2;

	if (distance <= nearestVeh.distance)
	{
		nearestVeh.driver = otherDriver;
		nearestVeh.distance = distance;
	}
}

bool DriverMovement::updateNearbyAgent(const Agent *nearbyAgent, const Driver *nearbyDriver)
{
	DriverUpdateParams &params = parentDriver->getParams();

	//Only update if passed a valid pointer which is not a pointer back to us
	if (!nearbyDriver || this->parentDriver == nearbyDriver)
	{
		return false;
	}

	//1.0 Get the current turnings of both, the current driver and the nearby drivers
	
	const TurningPath *otherTurning = nearbyDriver->isInIntersection_.get() ? nearbyDriver->currTurning_.get() : nearbyDriver->expectedTurning_.get();
	const TurningPath *currTurning = fwdDriverMovement.isInIntersection() ? fwdDriverMovement.getCurrTurning() : parentDriver->expectedTurning_.get();

	//Check if both drivers have a valid turning - meaning they are approaching or in the intersection
	if (currTurning && otherTurning)
	{
		//1.1 Get the turning conflict - if there is no conflict, it either means they are 
		//on the same turning or are on / approaching different intersections
		
		const TurningConflict *conflict = currTurning->getTurningConflict(otherTurning);
		
		if (conflict)
		{
			double conflictDist = (otherTurning == conflict->getFirstTurning()) ? conflict->getFirstConflictDistance() : conflict->getSecondConflictDistance();
			double distance = 0;

			if (nearbyDriver->isInIntersection_.get())
			{
				//2.0 Get distance covered by the other vehicle on the turning and calculate how far it
				//is from the conflict point
				
				double distCoveredOnTurning = nearbyDriver->distCoveredOnCurrWayPt_.get();
				distance = distCoveredOnTurning - conflictDist;
			}
			else
			{
				//2.0 Get distance from the conflict point
				//As the distance covered on turning is smaller than conflict distance, the value calculated above is 
				//negative. To keep consistency, we negate the distance to intersection
				//This means the distance to conflict point will always be negative, with smaller value indicating that it is 
				//further away
				
				distance = -nearbyDriver->distToIntersection_.get() - conflictDist;
			}

			// 2.1 Store the vehicle in the map of conflicting vehicles along with the distance from conflict point
			params.insertConflictTurningDriver(conflict, distance, nearbyDriver);
		}
		else if (currTurning == otherTurning)
		{
			//If the vehicles are on the same turning, then one is following the other
			
			double distance = 0;

			//Check if both drivers are in an intersection
			if (parentDriver->isInIntersection_.get() && nearbyDriver->isInIntersection_.get())
			{
				//As both vehicles are in the intersection, compare the distances covered on the turning
				//and identify which of us is in the front
				
				distance = nearbyDriver->distCoveredOnCurrWayPt_.get() - parentDriver->distCoveredOnCurrWayPt_.get();
				bool isForward = distance > 0;
				setNearestVehicle(isForward ? params.nvFwd : params.nvBack, distance, nearbyDriver);
			}
			else if (nearbyDriver->isInIntersection_.get())
			{
				//As we are not in the intersection, but the other vehicle is, it is obviously in front of us
				
				distance = fwdDriverMovement.getDistToEndOfCurrWayPt() + nearbyDriver->distCoveredOnCurrWayPt_.get();
				setNearestVehicle(params.nvFwd, distance, nearbyDriver);
			}
			else if (parentDriver->isInIntersection_.get())
			{
				//As we are in the intersection, but the other vehicle isn't, it is obviously in behind us
				
				distance = nearbyDriver->distToIntersection_.get() + parentDriver->distCoveredOnCurrWayPt_.get();
				setNearestVehicle(params.nvBack, distance, nearbyDriver);
			}
		}
		else if (currTurning->getFromLaneId() == otherTurning->getFromLaneId() &&
				currTurning->getToLaneId() != otherTurning->getToLaneId())
		{
			//Both turnings originate at the same lane, but diverge
			
			double distance = 0;

			//The other driver can be the front driver only for a few meters, say 10m
			//till the turning has some common area
			
			const double turningOverlapDist = 10;
			
			if (nearbyDriver->isInIntersection_.get() && nearbyDriver->distCoveredOnCurrWayPt_.get() <= turningOverlapDist)
			{
				distance = fwdDriverMovement.getDistToEndOfCurrWayPt() + nearbyDriver->distCoveredOnCurrWayPt_.get();
				setNearestVehicle(params.nvFwd, distance, nearbyDriver);
			}
			else if (parentDriver->isInIntersection_.get() && parentDriver->distCoveredOnCurrWayPt_.get() <= turningOverlapDist)
			{
				distance = nearbyDriver->distToIntersection_.get() + parentDriver->distCoveredOnCurrWayPt_.get();
				setNearestVehicle(params.nvBack, distance, nearbyDriver);
			}
		}
	}

	//The other driver's lane
	const Lane *otherLane = nearbyDriver->currLane_.get();
	
	//Either we or the other driver are in the intersection, we've already done the required updates above so return
	if (otherLane == nullptr || fwdDriverMovement.isInIntersection() || nearbyDriver->isInIntersection_.get())
	{
		return true;
	}

	//Retrieve the other driver's road segment, and distance covered on the segment.
	
	const RoadSegment* otherSegment = otherLane->getParentSegment();
	double otherDistCoveredOnCurrWayPt = nearbyDriver->distCoveredOnCurrWayPt_.get();

	//We need the length of the link while calculating the lane level density
	//as we will be considering the vehicles on a particular lane of a link.
	double linkLength = fwdDriverMovement.getCurrLink()->getLength();
	
	//If the vehicle is in the same Road segment
	if (fwdDriverMovement.getCurrSegment() == otherSegment)
	{
		//The distance between these two vehicles.
		double distance = otherDistCoveredOnCurrWayPt - fwdDriverMovement.getDistCoveredOnCurrWayPt();

		//Is the vehicle ahead of us
		bool fwd = distance > 0;

		//Set different variables depending on where the car is.

		//The vehicle is on the current lane
		if (otherLane == params.currLane)
		{
			//Increment the lane level density as the other car is in the same lane
			params.density = params.density + (1.0f / linkLength);

			setNearestVehicle((fwd ? params.nvFwd : params.nvBack), distance, nearbyDriver);
		}
		else if (otherLane == params.leftLane)
		{
			//The vehicle is on the left lane
			setNearestVehicle((fwd ? params.nvLeftFwd : params.nvLeftBack), distance, nearbyDriver);
		}
		else if (otherLane == params.rightLane)
		{
			//The vehicle is on the right lane
			setNearestVehicle((fwd ? params.nvRightFwd : params.nvRightBack), distance, nearbyDriver);
		}
		else if (otherLane == params.leftLane2)
		{
			//The vehicle is on the second Left lane
			setNearestVehicle((fwd ? params.nvLeftFwd2 : params.nvLeftBack2), distance, nearbyDriver);
		}
		else if (otherLane == params.rightLane2)
		{
			//The vehicle is on the second right lane
			setNearestVehicle((fwd ? params.nvRightFwd2 : params.nvRightBack2), distance, nearbyDriver);
		}
	}
	else if (otherSegment->getParentLink() == fwdDriverMovement.getCurrLink())
	{
		//We are in the same link.
		
		if (fwdDriverMovement.getCurrSegment()->getSequenceNumber() == otherSegment->getSequenceNumber() - 1)
		{
			//Vehicle is on the next segment
			
			const Lane *currLane = fwdDriverMovement.getCurrLane();
			const Lane *nextLane = fwdDriverMovement.getNextLane();
			unsigned int nextLaneIndex = nextLane->getLaneIndex();
			
			const Lane *leftOfNextLane = NULL;
			const Lane *rightOfNextLane = NULL;
			const Lane *leftOfNextLane2 = NULL;
			const Lane *rightOfNextLane2 = NULL;

			if (nextLaneIndex > 0 && nextLaneIndex - 1 < otherSegment->getNoOfLanes())
			{
				leftOfNextLane = otherSegment->getLane(nextLaneIndex - 1);
			}

			if (nextLaneIndex + 1 < otherSegment->getNoOfLanes())
			{
				rightOfNextLane = otherSegment->getLane(nextLaneIndex + 1);
			}

			if (nextLaneIndex > 1 && nextLaneIndex - 2 < otherSegment->getNoOfLanes())
			{
				leftOfNextLane2 = otherSegment->getLane(nextLaneIndex - 2);
			}

			if (nextLaneIndex + 2 < otherSegment->getNoOfLanes())
			{
				rightOfNextLane2 = otherSegment->getLane(nextLaneIndex + 2);
			}

			//Distance between the drivers
			int distance = (currLane->getLength() - fwdDriverMovement.getDistCoveredOnCurrWayPt()) + otherDistCoveredOnCurrWayPt;

			if (otherLane == nextLane)
			{		
				//Increment the lane level density as the other car is in the same lane
				//as we want to get into
				params.density = params.density + (1.0f / linkLength);

				setNearestVehicle(params.nvFwd, distance, nearbyDriver);
			}
			else if (otherLane == leftOfNextLane)
			{
				//The vehicle is on the left lane
				setNearestVehicle(params.nvLeftFwd, distance, nearbyDriver);
			}
			else if (otherLane == rightOfNextLane)
			{
				//The vehicle is on the right lane
				setNearestVehicle(params.nvRightFwd, distance, nearbyDriver);
			}
			else if (otherLane == leftOfNextLane2)
			{
				//The vehicle is on the second left lane
				setNearestVehicle(params.nvLeftFwd2, distance, nearbyDriver);
			}
			else if (otherLane == rightOfNextLane2)
			{
				//The vehicle is on the second right lane
				setNearestVehicle(params.nvRightFwd2, distance, nearbyDriver);
			}
		}
		else if (fwdDriverMovement.getCurrSegment()->getSequenceNumber() - 1 == otherSegment->getSequenceNumber())
		{
			//Vehicle is on the previous segment.
			
			unsigned int currLaneIndex = fwdDriverMovement.getCurrLane()->getLaneIndex();			
			
			const Lane *prevLane = NULL;			
			const Lane *leftOfPrevLane = NULL;
			const Lane *rightOfPrevLane = NULL;
			const Lane *leftOfPrevLane2 = NULL;
			const Lane *rightOfPrevLane2 = NULL;
			
			//If the current lane index is less than the number of lanes in the previous segment, then the previous lane had the same index
			if (currLaneIndex < otherSegment->getNoOfLanes())
			{
				prevLane = otherSegment->getLane(currLaneIndex);

				//Check if there are any lanes to the right of the previous lane
				if (currLaneIndex + 1 < otherSegment->getNoOfLanes())
				{
					rightOfPrevLane = otherSegment->getLane(currLaneIndex + 1);
				}

				if (currLaneIndex + 2 < otherSegment->getNoOfLanes())
				{
					rightOfPrevLane2 = otherSegment->getLane(currLaneIndex + 2);
				}
			}
			else if(fwdDriverMovement.getCurrSegment()->getNoOfLanes() > otherSegment->getNoOfLanes())
			{
				//Since the currLaneIndex is >= the number of lanes of the other segment, these lanes are to our left
				if (currLaneIndex >= 1 && currLaneIndex - 1 < otherSegment->getNoOfLanes())
				{
					leftOfPrevLane = otherSegment->getLane(currLaneIndex - 1);
				}

				if (currLaneIndex >= 2 && currLaneIndex - 2 < otherSegment->getNoOfLanes())
				{
					leftOfPrevLane2 = otherSegment->getLane(currLaneIndex - 2);
				}
			}

			//Distance between the drivers
			int distance = (otherLane->getLength() - otherDistCoveredOnCurrWayPt) + fwdDriverMovement.getDistCoveredOnCurrWayPt();

			if (otherLane == prevLane)
			{
				setNearestVehicle(params.nvBack, distance, nearbyDriver);
			}
			else if (otherLane == leftOfPrevLane)
			{
				//The vehicle is on the left lane
				setNearestVehicle(params.nvLeftBack, distance, nearbyDriver);
			}				
			else if (otherLane == rightOfPrevLane)
			{
				//The vehicle is on the right lane
				setNearestVehicle(params.nvRightBack, distance, nearbyDriver);
			}
			else if (otherLane == leftOfPrevLane2)
			{
				//The vehicle is on the second Left lane
				setNearestVehicle(params.nvLeftBack2, distance, nearbyDriver);
			}
			else if (otherLane == rightOfPrevLane2)
			{
				//The vehicle is on the second right lane				
				setNearestVehicle(params.nvRightBack2, distance, nearbyDriver);
			}
		}
	}
	else if (otherSegment->getParentLink() != fwdDriverMovement.getCurrLink())
	{
		//We are in different links.
		
		const WayPoint *nextWayPt = fwdDriverMovement.getNextWayPoint();
		unsigned int nextLink = 0;
		
		if(nextWayPt && nextWayPt->type == WayPoint::TURNING_GROUP)
		{
			nextLink = nextWayPt->turningGroup->getToLinkId();
		}
		
		if (nextLink == otherSegment->getLinkId())
		{
			//Vehicle is on the upcoming link, which is the link after the intersection.
			
			if (parentDriver->expectedTurning_.get() && parentDriver->expectedTurning_.get()->getToLane() == otherLane)
			{
				//The next vehicle is in the lane we're heading to
				
				if (params.nvFwd.driver == NULL)
				{
					//Distance between the drivers
					double distance = fwdDriverMovement.getDistToEndOfCurrLink() + nearbyDriver->distCoveredOnCurrWayPt_.get();
					setNearestVehicle(params.nvFwdNextLink, distance, nearbyDriver);
				}
			}
		}
	}

	return true;
}

double DriverMovement::getAngle() const
{
	if (fwdDriverMovement.isDoneWithEntireRoute())
	{
		return 0; //Shouldn't matter.
	}
	
	DynamicVector vector(fwdDriverMovement.getCurrPolyPoint(), fwdDriverMovement.getNextPolyPoint());

	return vector.getAngle();
}

void DriverMovement::updateNearbyAgents()
{
	DriverUpdateParams& params = parentDriver->getParams();
	vector<const Agent *> nearbyAgentsList;

	if (parentDriver->getCurrPosition().getX() > 0 && parentDriver->getCurrPosition().getY() > 0)
	{
		//Retrieve a list of nearby agents
		
		//Depending on whether we are on a turning or a lane, send the way-point with the corresponding object to 
		//th aura manager
		if(fwdDriverMovement.isInIntersection())
		{
			nearbyAgentsList = AuraManager::instance().nearbyAgents(parentDriver->getCurrPosition(), WayPoint(fwdDriverMovement.getCurrTurning()),
																	distanceInFront, distanceBehind, parentDriver->getParent());
		}
		else
		{
			nearbyAgentsList = AuraManager::instance().nearbyAgents(parentDriver->getCurrPosition(), WayPoint(fwdDriverMovement.getCurrLane()),
																	distanceInFront, distanceBehind, parentDriver->getParent());
		}		
	}
	else
	{
		Warn() << "A driver's location (x or y) is < 0, X:"
				<< parentDriver->getCurrPosition().getX() << ",Y:"
				<< parentDriver->getCurrPosition().getY() << std::endl;
	}

	//Update each nearby Pedestrian/Driver
	params.nvFwdNextLink.driver = NULL;
	params.nvFwdNextLink.distance = DBL_MAX;
	params.nvLeadFreeway.driver = NULL;
	params.nvLeadFreeway.distance = DBL_MAX;
	params.nvLagFreeway.driver = NULL;
	params.nvLagFreeway.distance = DBL_MAX;
	params.nvFwd.driver = NULL;
	params.nvFwd.distance = DBL_MAX;

	for (vector<const Agent *>::iterator it = nearbyAgentsList.begin(); it != nearbyAgentsList.end(); ++it)
	{
		//Perform no action on non-Persons
		const Person_ST *nearbyAgent = dynamic_cast<const Person_ST *> (*it);

		if (!nearbyAgent)
		{
			continue;
		}

		if (!nearbyAgent->getRole())
		{
			continue;
		}

		//Perform a different action depending on whether or not this is a Pedestrian/Driver/etc.
		/* Note:
		 * In the following methods(updateNearbyDriver and updateNearbyPedestrian), the variable "other"
		 * is the target which is being analysed, and the current object is the one who i object is the analyser.
		 */
		nearbyAgent->getRole()->handleUpdateRequest(this);
	}
}

void DriverMovement::perceivedDataProcess(NearestVehicle &nearestVehicle, DriverUpdateParams &params)
{
	//Update your perceptions for leading vehicle and gap
	if (nearestVehicle.exists())
	{
		if (parentDriver->reactionTime == 0)
		{
			params.perceivedFwdVelocityOfFwdCar = nearestVehicle.driver ? nearestVehicle.driver->fwdVelocity_.get() : 0;
			params.perceivedLatVelocityOfFwdCar = nearestVehicle.driver ? nearestVehicle.driver->latVelocity_.get() : 0;
			params.perceivedAccelerationOfFwdCar = nearestVehicle.driver ? nearestVehicle.driver->fwdAccel_.get() : 0;
			params.perceivedDistToFwdCar = nearestVehicle.distance;
			return;
		}

		parentDriver->perceivedDistToFwdCar->delay(nearestVehicle.distance);
		parentDriver->perceivedVelOfFwdCar->delay(nearestVehicle.driver->fwdVelocity_.get());
		parentDriver->perceivedAccOfFwdCar->delay(nearestVehicle.driver->fwdAccel_.get());
	}
	else
	{
		params.perceivedDistToFwdCar = maxVisibleDis;
		params.perceivedFwdVelocityOfFwdCar = params.maxLaneSpeed / KILOMETER_PER_HOUR_TO_METER_PER_SEC;
		params.perceivedAccelerationOfFwdCar = params.maxAcceleration;
	}
}

void DriverMovement::updateLateralMovement(DriverUpdateParams &params)
{
	//Lateral movement distance of current tick
	double lateralMovement = params.lateralVelocity * params.elapsedSeconds;

	//Update the lateral movement
	parentDriver->vehicle->moveLat(lateralMovement);

	//Check if lane changing operation completed
	lateralMovement = parentDriver->vehicle->getLateralMovement();
	lateralMovement = abs(lateralMovement);

	double halfLaneWidth = params.currLane->getWidth() * 0.8;

	if (lateralMovement > halfLaneWidth)
	{
		//Movement beyond the middle of the lane indicates that the vehicle has moved to the target lane
		//update lane related variables
		syncLaneInfoPostLateralMove(params);

		if (params.currLane->isPedestrianLane())
		{
			std::stringstream msg;
			msg << "Error: Car has moved onto sidewalk. Agent ID: "
				<< parentDriver->getParent()->getId();
			throw std::runtime_error(msg.str().c_str());
		}

		parentDriver->vehicle->resetLateralMovement();

		//Complete lane change movement
		params.unsetFlag(FLAG_PREV_LC);

		if (params.getStatus(STATUS_LEFT))
		{
			params.setFlag(FLAG_PREV_LC_LEFT);
		}
		else
		{
			params.setFlag(FLAG_PREV_LC_RIGHT);
		}

		params.unsetStatus(STATUS_CHANGING);
		params.laneChangeTime = params.now.ms();

		//Lane change complete, unset the "performing lane change" status
		params.unsetStatus(STATUS_LC_CHANGING);
		params.unsetStatus(STATUS_MANDATORY);
		params.unsetFlag(FLAG_NOSING | FLAG_YIELDING | FLAG_LC_FAILED);
		params.unsetFlag(FLAG_VMS_LANE_USE_BITS | FLAG_ESCAPE | FLAG_AVOID);
		params.unsetFlag(FLAG_STUCK_AT_END | FLAG_NOSING_FEASIBLE);
		params.unsetStatus(STATUS_TARGET_GAP);
	}
}

void DriverMovement::syncLaneInfoPostLateralMove(DriverUpdateParams &params)
{
	if (params.getStatus(STATUS_LC_RIGHT))
	{
		if (params.rightLane)
		{
			params.currLane = params.rightLane;
		}
	}
	else if (params.getStatus(STATUS_LC_LEFT))
	{
		if (params.leftLane)
		{
			params.currLane = params.leftLane;
		}
	}
	else
	{
		std::stringstream msg;
		msg << "syncInfoLateralMove (" << parentDriver->getParent()->GetId();
		msg << ") is attempting to change lane when no lane changing decision made";
		throw std::runtime_error(msg.str().c_str());
	}
		
	//Update the driver path mover
	fwdDriverMovement.updateLateralMovement(params.currLane);

	//Update which lanes are adjacent.
	identifyAdjacentLanes(params);	
}

void DriverMovement::setTrafficSignal()
{
	const Node *node = NULL;
	const WayPoint currWayPt = fwdDriverMovement.getCurrWayPoint();
	
	if(currWayPt.type == WayPoint::ROAD_SEGMENT)
	{
		node = currWayPt.roadSegment->getParentLink()->getToNode();
		trafficSignal = Signal::getSignal(node->getTrafficLightId());
	}
}

void DriverMovement::setTrafficSignalParams(DriverUpdateParams &params)
{
	if (!trafficSignal)
	{
		params.trafficColor = TRAFFIC_COLOUR_GREEN;
		parentDriver->perceivedTrafficColor->delay(params.trafficColor);
	}
	else
	{
		TrafficColor colour = TRAFFIC_COLOUR_INVALID;
		
		const Link *fromLink = fwdDriverMovement.getCurrLink();
		const Link *toLink = fwdDriverMovement.getNextLink();
		
		//Check if we have a next link in the path
		if(toLink)
		{
			colour = trafficSignal->getDriverLight(fromLink->getLinkId(), toLink->getLinkId());
		}
		else
		{
			colour = TRAFFIC_COLOUR_GREEN;
		}

		if (!parentDriver->perceivedTrafficColor->can_sense())
		{
			params.perceivedTrafficColor = colour;
		}

		parentDriver->perceivedTrafficColor->delay(params.trafficColor);

		params.trafficSignalStopDistance = fwdDriverMovement.getDistToEndOfCurrLink() - parentDriver->getVehicleLength();

		if (!parentDriver->perceivedDistToTrafficSignal->can_sense())
		{
			params.perceivedDistToTrafficSignal = params.trafficSignalStopDistance;
		}

		parentDriver->perceivedDistToTrafficSignal->delay(params.trafficSignalStopDistance);
	}
}
