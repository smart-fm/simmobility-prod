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
#include "entities/AuraManager.hpp"
#include "entities/Person.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "entities/UpdateParams.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Node.hpp"
#include "path/PathSetManager.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/RoadRunnerRegion.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/TurningSection.hpp"
#include "geospatial/UniNode.hpp"
#include "IncidentPerformer.hpp"
#include "network/CommunicationDataManager.hpp"

using namespace sim_mob;
using std::vector;
using std::set;
using std::string;
using std::endl;

//Helper functions
namespace {
//Helpful constants
const int distanceCheckToChangeLane = 150;
// meter conversion unit from centimeter
const double METER_TO_CENTIMETER_CONVERT_UNIT = 100;
// millisecs conversion unit from seconds
const double MILLISECS_CONVERT_UNIT = 1000.0;
// meters conversion unit from kilometers
const double KILOMETER_TO_METER_CONVERT_UNIT = 1000.0;
// secs conversion unit from hours
const double HOUR_TO_SEC_CONVERT_UNIT = 3600.0;
// km/h to m/s conversion unit
const double KILOMETER_PER_HOUR_TO_METER_PER_SEC = 3.6;
// default dis2stop meter(refer to MITSIM model)
const double DEFAULT_DIS_TO_STOP = 1000;
// default intersection speed, cm/s
const double DEFAULT_INTERSECTION_SPEED_CM_PER_SEC = 1000;

//Output helper
string PrintLCS(LANE_CHANGE_SIDE s)
{
    if (s == LCS_LEFT)
    {
        return "LCS_LEFT";
    }
    else if (s == LCS_RIGHT)
    {
        return "LCS_RIGHT";
    }
    return "LCS_SAME";
}

//used in lane changing, find the start index and end index of polyline in the target lane
size_t updateStartEndIndex(const std::vector<sim_mob::Point2D> *const currLanePolyLine,
    double currLaneOffset, size_t defaultValue)
{
    double offset = 0;
    for (size_t i = 0; i < currLanePolyLine->size() - 1; i++)
    {
        double xOffset = currLanePolyLine->at(i + 1).getX()
        - currLanePolyLine->at(i).getX();
        double yOffset = currLanePolyLine->at(i + 1).getY()
        - currLanePolyLine->at(i).getY();
        offset += sqrt(xOffset * xOffset + yOffset * yOffset);
        if (offset >= currLaneOffset)
        {
            return i;
        }
    }
    return defaultValue;
}

size_t getLaneIndex(const Lane * l)
{
    if (l)
    {
        const RoadSegment *r = l->getRoadSegment();
        for (size_t i = 0; i < r->getLanes().size(); i++)
        {
            if (r->getLanes().at(i) == l)
            {
                return i;
            }
        }
    }
    return -1; //NOTE: This might not do what you expect! ~Seth
}

} //End anon namespace

namespace sim_mob {
	
map<const RoadSegment *, unsigned long> DriverMovement::rdSegDensityMap;
boost::mutex DriverMovement::densityUpdateMutex;

DriverBehavior::DriverBehavior(sim_mob::Person* parentAgent) :
				BehaviorFacet(parentAgent), parentDriver(nullptr) {
}

DriverBehavior::~DriverBehavior() {
}

void DriverBehavior::frame_init() {
	throw std::runtime_error(
			"DriverBehavior::frame_init is not implemented yet");
}

void DriverBehavior::frame_tick() {
	throw std::runtime_error(
			"DriverBehavior::frame_tick is not implemented yet");
}

void DriverBehavior::frame_tick_output() {
	throw std::runtime_error(
			"DriverBehavior::frame_tick_output is not implemented yet");
}

sim_mob::DriverMovement::DriverMovement(sim_mob::Person* parentAgent, Driver* parentDriver) : 
	MovementFacet(parentAgent), parentDriver(parentDriver)
{
	if (Debug::Drivers) 
	{
		DebugStream << "Driver starting: ";
		
		if (parentAgent) 
		{
			DebugStream << parentAgent->getId();
		}
		else
		{
			DebugStream << "<null>";
		}
		
		DebugStream << endl;
	}
	
	trafficSignal = nullptr;
	nextLaneInNextLink = nullptr;
	cfModel = nullptr;
	lcModel = nullptr;
	intModel = nullptr;
	intModelBkUp = nullptr;
}

void sim_mob::DriverMovement::init() 
{
	if (!parentDriver) 
	{
		Warn() << "ERROR: no parentDriver, cannot initialise driver models!" << std::endl;
	}
	
	DriverUpdateParams& params = parentDriver->getParams();
	params.parentId = parent->getId();

	//Initialise our models.
	lcModel = new MITSIM_LC_Model(params);
	cfModel = new MITSIM_CF_Model(params);
	
	//Get the specified intersection driving model from the configuration
	const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();	
	std::map<std::string,std::string>::const_iterator itProp = configParams.system.genericProps.find("intersection_driving_model");
	
	//Set the default model as 'conflict-based'
	std::string intersectionModel = "conflict-based";
	
	//Check if a model is specified
	if(itProp != configParams.system.genericProps.end())
	{
		intersectionModel = itProp->second;
	}
	
	if(intersectionModel == "slot-based")
	{
		intModel = new SlotBased_IntDriving_Model();
	}
	else
	{
		intModel = new MITSIM_IntDriving_Model(params);
	}

	parentDriver->initReactionTime();
}

sim_mob::DriverMovement::~DriverMovement() 
{
	//Our movement models.
	safe_delete_item(lcModel);
	safe_delete_item(cfModel);
	safe_delete_item(intModel);
	safe_delete_item(intModelBkUp);
	
	//Usually the metrics for the last sub-trip is not manually finalised
	/*
	if(!travelMetric.finalized)
	{
		finalizeTravelTimeMetric();
	}
	*/
}

void sim_mob::DriverMovement::frame_init() 
{
	//Save the path from orign to next activity location in allRoadSegments
	parentDriver->getParams().initSegId = parent->initSegId;
	parentDriver->getParams().initDis = parent->initDis;
	parentDriver->getParams().initSpeed = parent->initSpeed;

	Vehicle* newVeh = initializePath(true);
	
	if (newVeh) 
	{
		safe_delete_item(parentDriver->vehicle);
		parent->getRole()->setResource(nullptr);
		parentDriver->vehicle = newVeh;
		parent->getRole()->setResource(newVeh);
	}

	//Set some properties about the current path, such as the current polyline, etc.
	if (parentDriver->vehicle && fwdDriverMovement.isPathSet()) 
	{
		setOrigin(parentDriver->getParams());
	} 
	else 
	{
		Warn()	<< "ERROR: Vehicle[short] could not be created for driver; no route!" << std::endl;
	}
}

void sim_mob::DriverMovement::setRR_RegionsFromCurrentPath() {
	if (parent->getRegionSupportStruct().isEnabled()) {
		if (parentDriver->vehicle) {
			std::vector<const sim_mob::RoadSegment*> path =
					fwdDriverMovement.fullPath;
			if (!path.empty()) {
				//We may be partly along this route, but it is unlikely. Still, just to be safe...
				const sim_mob::RoadSegment* currseg =
						fwdDriverMovement.getCurrSegment();

				//Now save it, taking into account the "current segment"
				rrPathToSend.clear();
				for (std::vector<const sim_mob::RoadSegment*>::const_iterator it =
						path.begin(); it != path.end(); ++it) {
					//Have we reached our starting segment yet?
					if (currseg) {
						if (currseg == *it) {
							//Signal this by setting currseg to null.
							currseg = nullptr;
						} else {
							continue;
						}
					}

					//Add it; we've cleared our current segment check one way or another.
					rrPathToSend.push_back(*it);
				}
			}
		}
	}
}

void sim_mob::DriverMovement::frame_tick() 
{
	// lost some params
	DriverUpdateParams& params = parentDriver->getParams();
	
	if (!(parentDriver->vehicle)) 
	{
		throw std::runtime_error("Something wrong, Vehicle is NULL");
	}

	//Are we done already?
	if (fwdDriverMovement.isDoneWithEntireRoute()) 
	{
		if (parent->amodId != "-1") 
		{
			parent->handleAMODArrival(); //handle AMOD arrival (if necessary)
		}
		parent->setToBeRemoved();
		return;
	}

	//Specific for Region support.
	if (parent->getRegionSupportStruct().isEnabled())
	{
		//Currently all_regions only needs to be sent once.
		if (sentAllRegions.check())
		{
			//Send the Regions.
			std::vector<RoadRunnerRegion> allRegions;
			const RoadNetwork& net = ConfigManager::GetInstance().FullConfig().getNetwork();

			for (std::map<int, RoadRunnerRegion>::const_iterator it = net.roadRunnerRegions.begin();
				 it != net.roadRunnerRegions.end(); ++it)
			{
				allRegions.push_back(it->second);
			}
			
			parent->getRegionSupportStruct().setNewAllRegionsSet(allRegions);

			//If a path has already been set, we will need to transmit it.
			setRR_RegionsFromCurrentPath();
		}

		//We always need to send a path if one is available.
		if (!rrPathToSend.empty())
		{
			std::vector<RoadRunnerRegion> regPath;
			for (std::vector<const RoadSegment*>::const_iterator it = rrPathToSend.begin(); it != rrPathToSend.end(); ++it)
			{
				//Determine if this road segment is within a Region.
				std::pair<RoadRunnerRegion, bool> rReg = StreetDirectory::instance().getRoadRunnerRegion(*it);
				
				if (rReg.second)
				{
					//Don't add if it's the last item in the list.
					if (regPath.empty()
						|| (regPath.back().id != rReg.first.id))
					{
						regPath.push_back(rReg.first);
					}
				}
			}
			parent->getRegionSupportStruct().setNewRegionPath(regPath);
			rrPathToSend.clear();
		}
	}

	//If the vehicle is in the loading queue, we need to check if some empty space has opened up.
	if (parentDriver->isVehicleInLoadingQueue && parentDriver->isVehiclePositionDefined)
	{
		//Use the aura manager to find out nearby vehicles. If none of the nearby vehicles on the same lane
		//take up the position that is with-in a particular tolerance of the origin of the current vehicle,
		//then we can set isVehicleInLoadingQueue to false.
		bool isEmptySpaceFound = findEmptySpaceAhead();

		//If an empty space has opened up, remove the vehicle from the queue
		if(isEmptySpaceFound)
		{
			parentDriver->isVehicleInLoadingQueue = false;
		}
	}

	//Just a bit glitchy...
	updateAdjacentLanes(params);

	//Update "current" time
	parentDriver->perceivedFwdVel->update(parentDriver->getParams().now.ms());	
	parentDriver->perceivedFwdAcc->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedDistToFwdCar->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedVelOfFwdCar->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedAccOfFwdCar->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedTrafficColor->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedDistToTrafficSignal->update(parentDriver->getParams().now.ms());

	//retrieved their current "sensed" values.
	if (parentDriver->perceivedFwdVel->can_sense())
	{
		params.perceivedFwdVelocity = parentDriver->perceivedFwdVel->sense();
	} 
	else
	{
		params.perceivedFwdVelocity = parentDriver->vehicle->getVelocity();
	}
	
	//General update behaviour.
	//Note: For now, most updates cannot take place unless there is a Lane and vehicle.
	if (parentDriver->isVehicleInLoadingQueue == false && params.currLane && parentDriver->vehicle) 
	{
		if (updateSensors(params.now) && updateMovement(params.now)
				&& updatePostMovement(params.now)) 
		{
			//Update parent data. Only works if we're not "done" for a bad reason.
			setParentBufferedData();
		}
	}

	//Update our Buffered types
	//TODO: Update parent buffered properties, or perhaps delegate this.
	if (!(fwdDriverMovement.isInIntersection())) 
	{
		parentDriver->currLane_.set(fwdDriverMovement.getCurrLane());
		parentDriver->currLaneOffset_.set(fwdDriverMovement.getCurrDistAlongRoadSegmentCM());
		parentDriver->currLaneLength_.set(fwdDriverMovement.getTotalRoadSegmentLengthCM());
	}

	//Indicate if we're in an intersection
	parentDriver->isInIntersection_.set(fwdDriverMovement.isInIntersection());
	
	//Set the current turning - if we're approaching or in an intersection, it is a valid value,
	//else it'll be null
	parentDriver->currTurning_.set(fwdDriverMovement.currTurning);
	
	//Set the distance covered within the intersection
	if(fwdDriverMovement.isInIntersection()) 
	{
		parentDriver->moveDisOnTurning_.set(intModel->getMoveDistance());
	}
	
	parentDriver->latMovement_.set(parentDriver->vehicle->getLateralMovement());
	parentDriver->fwdVelocity_.set(parentDriver->vehicle->getVelocity());
	parentDriver->latVelocity_.set(parentDriver->vehicle->getLatVelocity());
	parentDriver->fwdAccel_.set(parentDriver->vehicle->getAcceleration());
	parentDriver->turningDirection_.set(parentDriver->vehicle->getTurningDirection());
	
	if(!fwdDriverMovement.isDoneWithEntireRoute())
	{
		parentDriver->distToCurrSegmentEnd_.set(fwdDriverMovement.getDisToCurrSegEnd()); 
	}
	
	//Update your perceptions
	parentDriver->perceivedFwdVel->delay(parentDriver->vehicle->getVelocity());
	parentDriver->perceivedFwdAcc->delay(parentDriver->vehicle->getAcceleration());

	//Print output for this frame.
	parentDriver->currDistAlongRoadSegment = fwdDriverMovement.getCurrDistAlongRoadSegmentCM();
	DPoint position = getPosition();
	parentDriver->setCurrPosition(position);
	parentDriver->vehicle->setCurrPosition(position);

	setParentBufferedData();
	parentDriver->isVehiclePositionDefined = true;
	
	//Clear the NearestVehicles list in the conflictTurnings
	params.conflictVehicles.clear();
}

/*
 * This method is used to check if there is enough space on the lane where a vehicle from the
 * loading queue wants to start its journey.
 * Return value - true if empty space is found, else false
 */
bool sim_mob::DriverMovement::findEmptySpaceAhead()
{
	bool isSpaceFound = true;

	DriverUpdateParams &driverUpdateParams = parentDriver->getParams();

	//To store the agents that are in the nearby region
	vector<const Agent *> nearby_agents;

	//To store the closest driver approaching from the rear, if any
	//This is a pair of the driver object and his/her gap from the driver looking to exit the loading
	//queue
	pair<Driver *,double> driverApproachingFromRear(nullptr, DBL_MAX);

	//We need to find agents in front and those that may be coming in from behind!
	const int lookAheadDistance = parentDriver->distanceInFront, lookBehindDistance = parentDriver->distanceBehind;

	//Get the agents in nearby the current vehicle 
	nearby_agents = AuraManager::instance().nearbyAgents(Point2D(parentDriver->getCurrPosition().x,
			parentDriver->getCurrPosition().y),	*driverUpdateParams.currLane, lookAheadDistance, lookBehindDistance, nullptr);

	//Now if a particular agent is a vehicle and is in the same lane as the one we want to get into
	//then we have to check if it's occupying the space we need
	for(vector<const Agent *>::iterator itAgents = nearby_agents.begin(); itAgents != nearby_agents.end(); ++itAgents)
	{
		//We only need to only process agents those are vehicle drivers - this means that they are of type Person
		//and have role as driver or bus driver
		const Person *person = dynamic_cast<const Person *>(*itAgents);

		if(person != nullptr)
		{
			Role *role = person->getRole();
			if(role != nullptr)
			{
				if(role->roleType == Role::RL_DRIVER || role->roleType == Role::RL_BUSDRIVER)
				{
					Driver *nearbyDriver = dynamic_cast<Driver *>(role);
					DriverUpdateParams &nearbyDriversParams = nearbyDriver->getParams();

					//Make sure we're not checking distance from ourselves or someone in the loading queue
					//also ensure that the other vehicle is in our lane
					if (parentDriver != nearbyDriver && nearbyDriver->isVehicleInLoadingQueue == false &&
							driverUpdateParams.currLane == nearbyDriversParams.currLane)
					{
						DriverMovement *nearbyDriverMovement = dynamic_cast<DriverMovement *>(nearbyDriver->Movement());											

						//Get the gap to the nearby driver (in cm)
						//double availableGapInCM = fwdDriverMovement.getDisToCurrSegEnd() - nearbyDriverMovement->fwdDriverMovement.getDisToCurrSegEnd();
						double availableGapInCM = parentDriver->distToCurrSegmentEnd_.get() - nearbyDriver->distToCurrSegmentEnd_.get();

						//The gap between current driver and the one in front (or the one coming from behind) should be greater than
						//length(in cm) + (headway(in s) * initial speed(in cm/s))
						double requiredGapInCM = 0;
						if(availableGapInCM > 0)
						{
							//As the gap is positive, there is a vehicle in front of us. We should have enough distance
							//so as to avoid crashing into it
							MITSIM_CF_Model *mitsim_cf_model = dynamic_cast<MITSIM_CF_Model *>(cfModel);
							requiredGapInCM = (2 * parentDriver->vehicle->getLengthCm()) + (mitsim_cf_model->hBufferUpper * (driverUpdateParams.initSpeed * 100));
						}
						else
						{
							//As the gap is negative, there is a vehicle coming in from behind. We shouldn't appear right
							//in front of it, so consider it's speed to calculate required gap
							MITSIM_CF_Model *mitsim_cf_model = dynamic_cast<MITSIM_CF_Model *>(nearbyDriverMovement->cfModel);
							requiredGapInCM = (2 * nearbyDriver->vehicle->getLengthCm())+ (mitsim_cf_model->hBufferUpper)* (nearbyDriversParams.currSpeed * 100);

							//In case a driver is approaching from the rear, we need to reduce the reaction time, so that he/she
							//is aware of the presence of the car appearing in front.
							//But we need only the closest one
							if(driverApproachingFromRear.second > availableGapInCM)
							{
								driverApproachingFromRear.first = nearbyDriver;
								driverApproachingFromRear.second = availableGapInCM;
							}
						}

						if(abs(availableGapInCM) <= abs(requiredGapInCM))
						{
							//at least one vehicle is too close, so no need to search further
							isSpaceFound = false;

							//If any driver was added to the pair - driverApproachingFromRear, remove it
							//as we're not going to unload the vehicle from the loading queue
							driverApproachingFromRear.first = nullptr;
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
	if(driverApproachingFromRear.first != nullptr)
	{
		float alert = CF_CRITICAL_TIMER_RATIO * cfModel->updateStepSize[0];
		driverApproachingFromRear.first->getParams().cftimer = std::min<double>(alert, driverApproachingFromRear.first->getParams().cftimer);
	}

	return isSpaceFound;
}

void sim_mob::DriverMovement::frame_tick_output() 
{
	DriverUpdateParams &p = parentDriver->getParams();
	
	//Skip
	if (parentDriver->isVehicleInLoadingQueue || fwdDriverMovement.isDoneWithEntireRoute()) 
	{
		return;
	}

	if (ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) 
	{
		return;
	}

	double baseAngle = fwdDriverMovement.isInIntersection() ? intModel->getCurrentAngle() : getAngle();

	//Inform the GUI if interactive mode is active.
	if (ConfigManager::GetInstance().CMakeConfig().InteractiveMode()) 
	{
		std::ostringstream stream;
		stream << "DriverSegment" << "," << p.now.frame() << ","
					<< fwdDriverMovement.getCurrSegment() << ","
					<< fwdDriverMovement.getCurrentSegmentLengthCM() / METER_TO_CENTIMETER_CONVERT_UNIT;
		
		std::string s = stream.str();		
		ConfigManager::GetInstance().FullConfig().getCommDataMgr().sendTrafficData(s);
	}

	const bool inLane = parentDriver->vehicle && (!fwdDriverMovement.isInIntersection());

	//MPI-specific output.
	std::stringstream addLine;
	if (ConfigManager::GetInstance().FullConfig().using_MPI) 
	{
		addLine << "\",\"fake\":\""
				<< (this->getParent()->isFake ? "true" : "false");
	}

	int simid = parent->getId();
	std::stringstream res;
	res<<simid;
	std::string id = res.str();

	if(parent->amodId != "-1")
	{
		id = parent->amdoTripId;
		p.debugInfo = p.debugInfo+"<AMOD>";
	}
	else
	{
		id = res.str();

		//Check if the trip mode is taxi, if so append <Taxi> to debug info,
		//otherwise it means it is a private vehicle
		TripChainItem *tripChainItem = *(parent->currTripChainItem);
		
		if(tripChainItem->travelMode.compare("Taxi") == 0)
		{
			p.debugInfo = p.debugInfo+"<Taxi>";
		}
	}
	
	LogOut(
		"(\"Driver\"" << "," <<
		p.now.frame() << "," <<
		id << ",{" <<
		"\"xPos\":\"" << static_cast<int> (parentDriver->getCurrPosition().x) <<
		"\",\"yPos\":\"" << static_cast<int> (parentDriver->getCurrPosition().y) <<
		"\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI)) <<
		"\",\"length\":\"" << static_cast<int> (parentDriver->vehicle->getLengthCm()) <<
		"\",\"width\":\"" << static_cast<int> (parentDriver->vehicle->getWidthCm()) <<
		"\",\"curr-segment\":\"" << (inLane ? fwdDriverMovement.getCurrLane()->getRoadSegment() : 0x0) <<
		"\",\"fwd-speed\":\"" << parentDriver->vehicle->getVelocity() <<
		"\",\"fwd-accel\":\"" << parentDriver->vehicle->getAcceleration() <<
		"\",\"info\":\"" << p.debugInfo <<
		"\",\"mandatory\":\"" << incidentPerformer.getIncidentStatus().getChangedLane() << addLine.str() <<
		"\"})" << std::endl);
}

/*
 * This method simply increments the vehicle count for the vehicle's current road segment in the RdSegDensityMap  
 */
void sim_mob::DriverMovement::updateDensityMap()
{
	//The density map is a static map, so all threads will want to access it. Lock before accessing.
	densityUpdateMutex.lock();
	
	const RoadSegment *currSeg = fwdDriverMovement.getCurrSegment();
	
	//Find the entry for the road segment corresponding to the current vehicles segment
	map<const RoadSegment *, unsigned long>::iterator itDensityMap = rdSegDensityMap.find(currSeg);
	
	//Check if an entry exists
	if(itDensityMap != rdSegDensityMap.end())
	{
		itDensityMap->second += 1;
	}
	//Entry not found, so create a new one
	else
	{
		rdSegDensityMap.insert(make_pair(currSeg, 1));
	}
	
	//Done with update to the map, unlock.
	densityUpdateMutex.unlock();
}

/*
 * This method computes the density at every road segment and outputs it to file
 */
void sim_mob::DriverMovement::outputDensityMap(unsigned int tick)
{
	const ConfigParams &config = ConfigManager::GetInstance().FullConfig();
	
	//Get the logger instance
	sim_mob::BasicLogger &logger = sim_mob::Logger::log(config.segDensityMap.fileName);
	
	//Iterator to access all elements in the map
	map<const RoadSegment *, unsigned long>::iterator itDensityMap = rdSegDensityMap.begin();
	
	//Iterate through all elements in the map
	while(itDensityMap != rdSegDensityMap.end())
	{
		//Get collection time
		unsigned int period = config.segDensityMap.updateInterval / config.baseGranMS();
		
		//Get the average vehicle count
		double avgVehCount = (double)itDensityMap->second / period;
		
		//Convert the segment length to km from cm
		double segLength = itDensityMap->first->getLengthOfSegment() / 100000;
		
		unsigned int noOfLanes = itDensityMap->first->getLanes().size();
		
		//Calculate density. The unit is no of vehicles per lane-km
		double density = avgVehCount / (noOfLanes * segLength);

		logger << tick << "," << itDensityMap->first->getId() << "," << density << "\n"; 
		
		++itDensityMap;
	}
	
	//Clear the map
	rdSegDensityMap.clear();
}

// mark startTimeand origin
TravelMetric& sim_mob::DriverMovement::startTravelTimeMetric()
{
	travelMetric.startTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
	const Node* startNode = (*(fwdDriverMovement.fullPath.begin()))->getStart();
	if(!startNode)
	{
		throw std::runtime_error("Unknown Origin Node");
	}
	travelMetric.origin = WayPoint(startNode);
	travelMetric.started = true;
	return  travelMetric;
}

//	mark the destination and end time and travel time
TravelMetric& sim_mob::DriverMovement::finalizeTravelTimeMetric()
{
	if(!travelMetric.started)
	{
		return  travelMetric;
	}
	const sim_mob::RoadSegment * currRS = (fwdDriverMovement.currSegmentIt == fwdDriverMovement.fullPath.end() ?
			(*(fwdDriverMovement.fullPath.rbegin())) : (*(fwdDriverMovement.currSegmentIt)));
	if(!currRS)
	{
		throw std::runtime_error("Unknown Current Segment");
	}
	const Node* endNode = currRS->getEnd();
	travelMetric.destination = WayPoint(endNode);
	travelMetric.endTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
	travelMetric.travelTime = (travelMetric.endTime - travelMetric.startTime).getValue();
	travelMetric.finalized = true;
	//parent->addSubtripTravelMetrics(*travelMetric);

	return  travelMetric;
}

bool sim_mob::DriverMovement::updateSensors(timeslice now) 
{
	DriverUpdateParams& params = parentDriver->getParams();
	//Are we done?
	if (fwdDriverMovement.isDoneWithEntireRoute()) 
	{
		return false;
	}

	//Manage traffic signal behaviour if we are close to the end of the link.
	if(!fwdDriverMovement.isInIntersection()) 
	{
		setTrafficSignalParams(params);
	}

	//Save the nearest agents in your lane and the surrounding lanes, stored by their
	// position before/behind you. Save nearest fwd pedestrian too.
	updateNearbyAgents();

	//get nearest car, if not making lane changing, the nearest car should be the leading car in current lane.
	//if making lane changing, adjacent car need to be taken into account.
	NearestVehicle & nv = params.nvFwd;
	perceivedDataProcess(nv, params);

	return true;
}

bool sim_mob::DriverMovement::updateMovement(timeslice now) 
{	
	DriverUpdateParams& params = parentDriver->getParams();
	
	//If reach the goal, get back to the origin
	if (fwdDriverMovement.isDoneWithEntireRoute()) 
	{
		//Output
		if (Debug::Drivers && !DebugStream.str().empty()) 
		{
			if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) 
			{
				DebugStream << ">>>Vehicle done." << endl;
				PrintOut(DebugStream.str());
				DebugStream.str("");
			}
		}

		return false;
	}

	//Save some values which might not be available later.
	const RoadSegment* prevSegment = fwdDriverMovement.getCurrSegment();
	
	//Store the current speed
	params.currSpeed = parentDriver->getVehicle()->getVelocity() / 100;

	params.TEMP_lastKnownPolypoint = DPoint(getCurrPolylineVector().getEndX(),
			getCurrPolylineVector().getEndY());
	
	//Handle driving within an intersection
	if (fwdDriverMovement.isInIntersection())
	{
		parentDriver->perceivedDistToTrafficSignal->clear();
		parentDriver->perceivedTrafficColor->clear();		
		performIntersectionDriving(params);
	}
	
	// Next, handle driving on links.
	// Note that a vehicle may leave an intersection during intersectionDriving(), so the conditional check is necessary.
	// Note that there is no need to chain this back to intersectionDriving.
	if (!fwdDriverMovement.isInIntersection() && !fwdDriverMovement.isDoneWithEntireRoute()) 
	{				
		params.cftimer -= params.elapsedSeconds;
		if (params.cftimer < params.elapsedSeconds) 
		{
			// make lc decision and check if can do lc
			calcVehicleStates(params);
		}

		// perform lc ,if status is STATUS_LC_CHANGING
		params.overflowIntoIntersection = move(params);
		
		//Did our last move forward bring us into an intersection?
		if (fwdDriverMovement.isInIntersection()) 
		{
			params.justMovedIntoIntersection = true;
			parentDriver->vehicle->setLatVelocity(0);
			parentDriver->vehicle->setTurningDirection(LCS_SAME);
			chooseNextLaneForNextLink(params);
			
			//We've reached the intersection, but we don't have the next lane in the next link.
			//This means that we were not able to change lanes in time to reach the lane with the
			//desired turning
			if(nextLaneInNextLink == nullptr)
			{
				parent->setToBeRemoved();
			}
		}
	}

	if ((!(fwdDriverMovement.isDoneWithEntireRoute())) && ((fwdDriverMovement.isPathSet())))
	{
		//Update the road segment density map
		if (ConfigManager::GetInstance().FullConfig().segDensityMap.outputEnabled)
		{
			updateDensityMap();
		}
		
		//Has the segment changed?
		params.justChangedToNewSegment = ((fwdDriverMovement.getCurrSegment() != prevSegment));
	}

	//The segment has changed, calculate link travel time and road segment travel time
	/*if (params.justChangedToNewSegment == true)
	{
		//Agent* parentAgent = parent;
		const Link* prevLink = prevSegment->getLink();
		double actualTime = parentDriver->getParams().elapsedSeconds
					+ (parentDriver->getParams().now.ms() / MILLISECS_CONVERT_UNIT);
		
		//Check if the link has changed
		if(prevLink != fwdDriverMovement.getCurrLink())
		{
			//if prevLink is already in travelStats, update it's linkTT and add to travelStatsMap
			if (prevLink == parent->getLinkTravelStats().link_)
			{
				parent->addToLinkTravelStatsMap(parent->getLinkTravelStats(), actualTime); //in seconds
			}

			//creating a new entry in agent's travelStats for the new link, with entry time
			parent->initLinkTravelStats(fwdDriverMovement.getCurrSegment()->getLink(), actualTime);
		}
		
		//If previous segment is already in the travel stats, update the exit time
		if(prevSegment == parent->getCurrRdSegTravelStats().rs)
		{
			const std::string &travelMode = parent->getRole()->getMode();
			Agent::RdSegTravelStat &currStats = parent->finalizeCurrRdSegTravelStat(prevSegment, actualTime, travelMode);
			PathSetManager::getInstance()->addSegTT(currStats);
		}
		
		//creating a new entry in agent's travelStats for the new road segment, with entry time
		parent->getCurrRdSegTravelStats().reset();
		parent->startCurrRdSegTravelStat(fwdDriverMovement.getCurrSegment(), actualTime);
	}
	
	//Finalise the travel times for the last link and segment
	if (fwdDriverMovement.isDoneWithEntireRoute())
	{
		double actualTime = parentDriver->getParams().elapsedSeconds
					+ (parentDriver->getParams().now.ms() / MILLISECS_CONVERT_UNIT);
		
		const std::string &travelMode = parent->getRole()->getMode();
		Agent::RdSegTravelStat &currStats = parent->finalizeCurrRdSegTravelStat(prevSegment, actualTime, travelMode);
		PathSetManager::getInstance()->addSegTT(currStats);
		
		//Update the link travel time only if the person completes the journey at the end of a link
		const MultiNode* endNode = dynamic_cast<const MultiNode *>(prevSegment->getEnd());
		if(endNode)
		{
			parent->addToLinkTravelStatsMap(parent->getLinkTravelStats(), actualTime);
		}
	}*/

	if (!fwdDriverMovement.isDoneWithEntireRoute())
	{
		params.TEMP_lastKnownPolypoint = DPoint(getCurrPolylineVector().getEndX(), getCurrPolylineVector().getEndY());
	}
	
	params.buildDebugInfo();
	return true;
}

bool sim_mob::DriverMovement::updatePostMovement(timeslice now) 
{
	DriverUpdateParams& params = parentDriver->getParams();
	
	//If we're done with the route, there is nothing to be done.
	if (fwdDriverMovement.isDoneWithEntireRoute()) 
	{
		return false;
	}

	//Has the segment changed?
	if (!(fwdDriverMovement.isInIntersection()) && params.justChangedToNewSegment) 
	{
		if (!(hasNextSegment(true))) 
		{
			setTrafficSignal();
		}
		fwdDriverMovement.currTurning = nullptr;
	}

	params.isApproachingIntersection = false;
	
	if (!(fwdDriverMovement.isInIntersection()) && !(hasNextSegment(true))
			&& hasNextSegment(false)) 
	{
		chooseNextLaneForNextLink(params);
		
		//Detect if a vehicle is approaching an intersection. We do this by comparing the 
		//distance to the end of the road segment and the visibility distance
	
		//The distance to the end of the road segment (metre)
		double distToIntersection = fwdDriverMovement.getDistToLinkEndM();
		
		//Visibility of the intersection (metre). This should be retrieved from the corresponding conflict 
		//section once it has been added there
		if (distToIntersection < intModel->getIntersectionVisbility())
		{
			params.isApproachingIntersection = true;
			
			//Set the distance to the intersection 
			parentDriver->distToIntersection_.set(distToIntersection);
			
			if(intModel->getIntModelType() == Int_Model_SlotBased)
			{
				//We send a request to the intersection manager of the approaching intersection, asking for 'permission' to enter the 
				//intersection
				SlotBased_IntDriving_Model *model = dynamic_cast<SlotBased_IntDriving_Model *>(intModel);
				model->sendAccessRequest(params);
			}
		}
		else
		{
			parentDriver->distToIntersection_.set(-1);
		}
	}

	//Check if we are in an intersection
	if(fwdDriverMovement.isInIntersection())
	{
		//Update the speed and acceleration of the vehicle in an intersection
		updateIntersectionVelocity();
		
		//Have we just entered into an intersection?
		if (params.justMovedIntoIntersection) 
		{
			//Calculate a trajectory and init movement on that intersection.
			calculateIntersectionTrajectory(params.TEMP_lastKnownPolypoint,
											params.overflowIntoIntersection);

			//Fix: We need to perform this calculation at least once or we won't have a heading within the intersection.
			DPoint res = intModel->continueDriving(0, params);
			parentDriver->vehicle->setPositionInIntersection(res.x, res.y);
		}
	}
	
	return true;
}

/*
 This method helps define the driver behaviour when approaching an unsignalised intersection
 It looks for conflict drivers, in order to help decide the approach speed
*/
double sim_mob::DriverMovement::performIntersectionApproach()
{
	DriverUpdateParams& params = parentDriver->getParams();
	double accInt = params.maxAcceleration;
	
	//The multi-node which houses the turning must be the end node of current segment and the start node
	//of the next segment. If the two nodes are not the same, means we're in a different segment (we're close to
	//the intersection, but a short segment is likely ahead of us)
	
	//The current RoadSegment the vehicle is on
	const RoadSegment *currSegment = fwdDriverMovement.getCurrSegment();

	//The RoadSegment the vehicle will move to after the intersection
	const RoadSegment *nextSegment = fwdDriverMovement.getNextSegment(false);

	//No next segment, that means we're at the end of our path
	if(nextSegment)
	{
		//The turning section that will be used by the vehicle to move from the current segment to the next segment
		const TurningSection *turningSection = NULL;

		if (currSegment->getEnd() == nextSegment->getStart())
		{
			const Lane *currentLane = fwdDriverMovement.getCurrLane();
			const MultiNode *node = dynamic_cast<const MultiNode *> (currSegment->getEnd());

			//Get the turning section that will be used by the vehicle
			turningSection = node->getTurningSection(currentLane, nextLaneInNextLink);
		}

		//Check if we have a turning section. Absence of a turning section indicates that either 
		//we have not yet entered the required from lane or that we're close to the intersection,
		//but there's a short segment ahead of us - so, we can only defer processing till later
		if (turningSection)
		{
			//Set the current turning
			fwdDriverMovement.currTurning = turningSection;			
			
			//Reset the impatience timer
			params.impatienceTimer = params.impatienceTimerStart = 0;
			
			//Set the max turning speed
			params.maxLaneSpeed = turningSection->getTurningSpeed() / KILOMETER_PER_HOUR_TO_METER_PER_SEC;			
			
			//Scan for conflicts in intersection only in un-signalised intersections
			if (!trafficSignal)
			{
				//Set the current turning
				intModel->setCurrTurning(turningSection);
				
				//Calculate the acceleration based on the vehicles in the intersection
				accInt = intModel->makeAcceleratingDecision(params);
			}
			
			return accInt;
		}
		else
		{
			return accInt;
		}
	}
	else
	{
		return accInt;
	}
}

//responsible for vehicle behaviour inside intersection
//the movement is based on absolute position
void sim_mob::DriverMovement::performIntersectionDriving(DriverUpdateParams& p) 
{
	//Don't move if we have no target
	if (!nextLaneInNextLink) 
	{
		return;
	}	
	
	perceiveParameters(p);
	
	//Convert to m/s
	p.currSpeed = parentDriver->vehicle->getVelocity() / METER_TO_CENTIMETER_CONVERT_UNIT;

	p.cftimer -= p.elapsedSeconds;	
	if (p.cftimer < p.elapsedSeconds)
	{
		double cfAcc = DBL_MAX, intAcc = DBL_MAX;
		
		//Clear the flag 
		parentDriver->setYieldingToInIntersection(-1);		
		
		//Scan for conflicts only in un-signalised intersections
		if (!trafficSignal)
		{
			//Set the current turning
			intModel->setCurrTurning(fwdDriverMovement.currTurning);	
			
			//Call the intersection driving model
			intAcc = intModel->makeAcceleratingDecision(p);
		}
		//In case we've moved forward into an intersection then stopped, when there was a red light		
		//The "aC" indicates that previously acceleration due to traffic signal was selected		
		else if(p.accSelect == "aC" && parentDriver->getVehicle()->getAcceleration() <= 0)
		{
			//Get the traffic light colour
			p.perceivedTrafficColor = trafficSignal->getDriverLight(*p.currLane, *nextLaneInNextLink);
		}
		
		//Call the car following model
		cfAcc = cfModel->makeAcceleratingDecision(p);
		
		//Select the lower of the two accelerations
		if(cfAcc < intAcc && !p.useIntAcc)
		{
			p.newFwdAcc = cfAcc;
			parentDriver->setYieldingToInIntersection(-1);
		}
		else
		{
			p.newFwdAcc = intAcc;
			
			//For debugging on visualiser
			p.acc = intAcc;
			p.accSelect = "aInt";
		}
		
		//We need to reduce reaction time in the intersection if we're using the MITSIM model
		if(intModel->getIntModelType() == Int_Model_MITSIM)
		{
			MITSIM_IntDriving_Model *intersectionModel = dynamic_cast<MITSIM_IntDriving_Model *>(intModel);
			
			//Reduce the reaction time in intersection			
			p.cftimer = p.cftimer * Utils::generateFloat(intersectionModel->getIntersectionAttentivenessFactorMin(),
														 intersectionModel->getIntersectionAttentivenessFactorMax());
		}
	}
	
	//Calculate the distance travelled
	//s = ut + (1/2)at^2
	double distanceTravelled = (p.currSpeed * p.elapsedSeconds) + (0.5 * p.newFwdAcc * p.elapsedSeconds * p.elapsedSeconds);
	
	//Distance can't be negative, just ensuring
	if(distanceTravelled < 0)
	{
		distanceTravelled = 0;
	}
	
	//update movement along the vector.
	DPoint res = intModel->continueDriving(distanceTravelled * METER_TO_CENTIMETER_CONVERT_UNIT,p);
	parentDriver->vehicle->setPositionInIntersection(res.x, res.y);

	//Next, detect if we've just left the intersection. Otherwise, perform regular intersection driving.
	if (intModel->isDone())
	{
		parentDriver->vehicle->setPositionInIntersection(0, 0);
		p.currLane = fwdDriverMovement.leaveIntersection();
		postIntersectionDriving(p);
	}
}

void sim_mob::DriverMovement::calcDistanceToStoppingPoint(DriverUpdateParams& p) {
	// check state machine
	// 1.0 find nearest forward stop point
	DriverMovement *driverMvt = dynamic_cast<DriverMovement*>(p.driver->Movement());
	// get dis to stop point of current link
	double distance = driverMvt->getDisToStopPoint(p.stopPointPerDis);

	if (abs(distance) < 50) {
		if (parent->amodId != "-1") {
			parent->handleAMODPickup(); //handle AMOD arrival (if necessary)
		}
	}

	p.disToSP = distance;

	if(distance>-10 || p.stopPointState == DriverUpdateParams::JUST_ARRIVE_STOP_POINT){// in case car stop just bit ahead of the stop point
		if(distance < 0 && p.stopPointState == DriverUpdateParams::LEAVING_STOP_POINT){
			return ;
		}
		// has stop point ahead
		if(p.stopPointState == DriverUpdateParams::NO_FOUND_STOP_POINT){
			p.stopPointState = DriverUpdateParams::APPROACHING_STOP_POINT;
		}
		if(distance >= 10 && distance <= 50){ // 10m-50m
			p.stopPointState = DriverUpdateParams::CLOSE_STOP_POINT;
		}
		if(p.stopPointState == DriverUpdateParams::CLOSE_STOP_POINT && abs(distance) < 10){ // 0m-10m
			std::cout<<p.now.frame()<<" JUST_ARRIVE_STOP_POINT"<<std::endl;
			p.stopPointState = DriverUpdateParams::JUST_ARRIVE_STOP_POINT;
		}
		
		p.dis2stop = distance;
		return ;
	}//end of dis
	if(distance<-10 && p.stopPointState == DriverUpdateParams::LEAVING_STOP_POINT){
		p.stopPointState = DriverUpdateParams::NO_FOUND_STOP_POINT;
	}

	p.stopPointState = DriverUpdateParams::NO_FOUND_STOP_POINT;
	return ;
}

void sim_mob::DriverMovement::calcVehicleStates(DriverUpdateParams& p) 
{
	// TODO: if STATUS_LC_CHANGING ,means "perform lane changing",just return
	p.lcDebugStr.str(std::string());

	perceiveParameters(p);
	
	//Currently on AMOD and Buses have stop points, so at the moment calls to check for stop point
	//for private cars and taxis will be a burden.
	if (parent->amodId != "-1" || parentDriver->isBus())
	{
		calcDistanceToStoppingPoint(p);
	}
	
	// make lc decision
	LANE_CHANGE_SIDE lcs = lcModel->makeLaneChangingDecision(p);

	if (p.getStatus() & STATUS_CHANGING) 
	{
		p.lcDebugStr<<";CHING";
		
		// if need change lane, check left,right gap to do lane change or to do nosing
		lcModel->executeLaneChanging(p);

		// if left,right gap not ok, choose ADJACENT ,BACKWARD, FORWARD gap
		if (p.flag(FLAG_LC_FAILED)) 
		{
			p.lcDebugStr<<";COG";
			lcModel->chooseTargetGap(p);
		}
	} //end if STATUS_CHANGING

	//Convert back to m/s
	//TODO: Is this always m/s? We should rename the variable then...
	p.currSpeed = parentDriver->vehicle->getVelocity() / METER_TO_CENTIMETER_CONVERT_UNIT;

	double intApproachAcc = DBL_MAX, cfAcc = DBL_MAX;
	
	//Check if this vehicle is approaching an unsignalised intersection.
	if (p.isApproachingIntersection)
	{
		parentDriver->setYieldingToInIntersection(-1);
		
		//Calculate the approaching intersection acceleration
		intApproachAcc = performIntersectionApproach();
	}
	
	//Call car following model
	cfAcc = cfModel->makeAcceleratingDecision(p);
	
	//Select the lower of the two accelerations
	if(cfAcc < intApproachAcc && !p.useIntAcc)
	{
		p.newFwdAcc = cfAcc;
		parentDriver->setYieldingToInIntersection(-1);
	}
	else
	{
		p.acc = p.newFwdAcc = intApproachAcc;
		p.accSelect = "aI";
	}
}

void sim_mob::DriverMovement::perceiveParameters(DriverUpdateParams& p)
{
	if (parentDriver->perceivedVelOfFwdCar->can_sense()
		&& parentDriver->perceivedAccOfFwdCar->can_sense()
		&& parentDriver->perceivedDistToFwdCar->can_sense())
	{
		p.perceivedFwdVelocityOfFwdCar = parentDriver->perceivedVelOfFwdCar->sense();
		p.perceivedAccelerationOfFwdCar = parentDriver->perceivedAccOfFwdCar->sense();
		p.perceivedDistToFwdCar = parentDriver->perceivedDistToFwdCar->sense();

	}
	else
	{
		NearestVehicle & nv = p.nvFwd;
		p.perceivedFwdVelocityOfFwdCar = nv.driver ? nv.driver->fwdVelocity_.get() : 0;
		p.perceivedLatVelocityOfFwdCar = nv.driver ? nv.driver->latVelocity_.get() : 0;
		p.perceivedAccelerationOfFwdCar = nv.driver ? nv.driver->fwdAccel_.get() : 0;
		p.perceivedDistToFwdCar = nv.distance;
	}

	if (parentDriver->perceivedTrafficColor->can_sense())
	{
		p.perceivedTrafficColor = parentDriver->perceivedTrafficColor->sense();
	}

	if (parentDriver->perceivedDistToTrafficSignal->can_sense())
	{
		p.perceivedDistToTrafficSignal = parentDriver->perceivedDistToTrafficSignal->sense();
	}
}

double sim_mob::DriverMovement::move(DriverUpdateParams& p) 
{
	double newLatVel = 0.0; // m/s
	LANE_CHANGE_SIDE lcs;
	
	if (p.getStatus(STATUS_LC_RIGHT)) 
	{
		lcs = LCS_RIGHT;
	}
	else if (p.getStatus(STATUS_LC_LEFT)) 
	{
		lcs = LCS_LEFT;
	}
	else 
	{
		lcs = LCS_SAME;
	}

	newLatVel = lcModel->executeLaterVel(lcs);

	p.newLatVelM = newLatVel;

	parentDriver->vehicle->setTurningDirection(lcs);
	parentDriver->vehicle->setLatVelocity(newLatVel * METER_TO_CENTIMETER_CONVERT_UNIT);

	double acc = p.newFwdAcc;

	//Update our chosen acceleration; update our position on the link.
	parentDriver->vehicle->setAcceleration(acc * METER_TO_CENTIMETER_CONVERT_UNIT);

	return updatePositionOnLink(p);
}

double sim_mob::DriverMovement::getDistanceToSegmentEnd() const 
{
	DynamicVector dis(parentDriver->getCurrPosition().x,
					parentDriver->getCurrPosition().y,
					fwdDriverMovement.getCurrSegment()->getEnd()->location.getX(),
					fwdDriverMovement.getCurrSegment()->getEnd()->location.getY());
	
	return dis.getMagnitude();
}

void sim_mob::DriverMovement::setParentBufferedData() 
{
	parent->xPos.set(parentDriver->getCurrPosition().x);
	parent->yPos.set(parentDriver->getCurrPosition().y);

	//TODO: Need to see how the parent agent uses its velocity vector.
	parent->fwdVel.set(parentDriver->vehicle->getVelocity());
	parent->latVel.set(parentDriver->vehicle->getLatVelocity());
}

void sim_mob::DriverMovement::buildAndSetPath(std::vector<sim_mob::WayPoint> wp_path, int startLaneID) 
{
	//Construct a list of RoadSegments.
	vector<const RoadSegment*> path;
	
	for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); ++it) 
	{
		if (it->type_ == WayPoint::ROAD_SEGMENT) 
		{
			path.push_back(it->roadSegment_);
		}
	}

	fwdDriverMovement.setPath(path, startLaneID);
}

void sim_mob::DriverMovement::buildAndSetPathWithInitSeg(std::vector<sim_mob::WayPoint> wp_path, int startLaneID, int segId, int initPer, int initSpeed) 
{
	//Construct a list of RoadSegments.
	vector<const RoadSegment*> path;
	for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); ++it) 
	{
		if (it->type_ == WayPoint::ROAD_SEGMENT) 
		{
			path.push_back(it->roadSegment_);
		}
	}

	fwdDriverMovement.setPathWithInitSeg(path, startLaneID, segId, initPer, initSpeed);
}

void sim_mob::DriverMovement::resetPath(std::vector<sim_mob::WayPoint> wp_path) 
{
	//Construct a list of RoadSegments.
	vector<const RoadSegment*> path;	
	for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); ++it) 
	{
		if (it->type_ == WayPoint::ROAD_SEGMENT) {
			path.push_back(it->roadSegment_);
		}
	}
	
	fwdDriverMovement.resetPath(path);
}

const RoadSegment* sim_mob::DriverMovement::hasNextSegment(bool inSameLink) const 
{
	if (!fwdDriverMovement.isDoneWithEntireRoute()) 
	{
		return fwdDriverMovement.getNextSegment(inSameLink);
	}
	return nullptr;
}

DPoint sim_mob::DriverMovement::getPosition() 
{
	//Temp
	if (fwdDriverMovement.isInIntersection()
		&& (parentDriver->vehicle->getPositionInIntersection().x == 0
		|| parentDriver->vehicle->getPositionInIntersection().y == 0)) 
	{
		Warn() << "WARNING: Vehicle is in intersection without a position!"
				<< std::endl;
	}

	parentDriver->getParams().disAlongPolyline = fwdDriverMovement.getCurrDistAlongPolylineCM();
	DPoint origPos = fwdDriverMovement.getPosition();
	parentDriver->getParams().movementVectx = fwdDriverMovement.movementVect.getX();
	parentDriver->getParams().movementVecty = fwdDriverMovement.movementVect.getY();

	if (fwdDriverMovement.isInIntersection()
		&& parentDriver->vehicle->getPositionInIntersection().x != 0
		&& parentDriver->vehicle->getPositionInIntersection().y != 0) 
	{
		//Override: Intersection driving
		origPos.x = parentDriver->vehicle->getPositionInIntersection().x;
		origPos.y = parentDriver->vehicle->getPositionInIntersection().y;
	}
	else if (parentDriver->vehicle->getLateralMovement() != 0
			 && !fwdDriverMovement.isDoneWithEntireRoute()) 
	{
		DynamicVector latMv(0, 0, fwdDriverMovement.getNextPolypoint().getX() - fwdDriverMovement.getCurrPolypoint().getX(),
							fwdDriverMovement.getNextPolypoint().getY() - fwdDriverMovement.getCurrPolypoint().getY());
		
		latMv.flipLeft();
		latMv.scaleVectTo(parentDriver->vehicle->getLateralMovement()).translateVect();
		parentDriver->getParams().latMv_ = latMv;
		
		origPos.x += latMv.getX();
		origPos.y += latMv.getY();
	}

	parentDriver->getParams().dorigPosx = origPos.x - parentDriver->getParams().lastOrigPos_.x;
	parentDriver->getParams().dorigPosy = origPos.y - parentDriver->getParams().lastOrigPos_.y;
	parentDriver->getParams().lastOrigPos_ = origPos;

	return origPos;
}

const sim_mob::RoadItem* sim_mob::DriverMovement::getRoadItemByDistance(sim_mob::RoadItemType type, double &itemDis, double perceptionDis, bool isInSameLink) 
{	
	const sim_mob::RoadItem* res = nullptr;
	itemDis = 0.0;

	if (type != sim_mob::INCIDENT) 
	{
		return res;
	}

	std::vector<const sim_mob::RoadSegment*>::iterator currentSegIt = fwdDriverMovement.currSegmentIt;
	std::vector<const sim_mob::RoadSegment*>::iterator currentSegItEnd = fwdDriverMovement.fullPath.end();

	for (; currentSegIt != currentSegItEnd; ++currentSegIt) 
	{
		if (currentSegIt == currentSegItEnd) 
		{
			break;
		}

		const RoadSegment* rs = *currentSegIt;
		
		if (!rs)
		{
			break;
		}

		const std::map<centimeter_t, const RoadItem*> obstacles = rs->getObstacles();
		std::map<centimeter_t, const RoadItem*>::const_iterator obsIt;

		if (obstacles.empty()) 
		{
			if (rs == fwdDriverMovement.getCurrSegment()) 
			{
				itemDis = fwdDriverMovement.getCurrentSegmentLengthCM() - fwdDriverMovement.getCurrDistAlongRoadSegmentCM();
			}
			else 
			{
				itemDis += rs->getLengthOfSegment();
			}
		}

		for (obsIt = obstacles.begin(); obsIt != obstacles.end(); ++obsIt) 
		{
			const Incident* inc = dynamic_cast<const Incident*>((*obsIt).second);

			if (rs == fwdDriverMovement.getCurrSegment()) 
			{
				//1. in current seg
				if (inc) 
				{
					//1.1 find incident
					double incidentDis = (*obsIt).first;
					double moveDis = fwdDriverMovement.getCurrDistAlongRoadSegmentCM();
					
					//1.2 incident in forward
					if (moveDis <= incidentDis) 
					{
						itemDis = incidentDis - moveDis;
						
						if (itemDis < 0) 
						{
							std::cout
							<< "getRoadItemByDistance: getDistanceMovedInSegment not right"
							<< std::endl;
						}
						
						if (itemDis <= perceptionDis) 
						{
							res = inc;
							return res;
						}
						else 
						{
							// the incident already out of perception, no need check far more
							return nullptr;
						}
					} // end if moveDis
				} //end if inc
				
				itemDis = fwdDriverMovement.getCurrentSegmentLengthCM() - fwdDriverMovement.getCurrDistAlongRoadSegmentCM();
			} //end rs==
			else 
			{
				//2.0 in forword seg
				if (isInSameLink == true) 
				{
					// seg not in current link
					if (fwdDriverMovement.getCurrSegment()->getLink() != rs->getLink()) 
					{
						return res;
					}
				}
				
				if (inc) 
				{
					//2.1 find incident
					double incidentDis = (*obsIt).first;
					itemDis += incidentDis;
					
					if (itemDis <= perceptionDis) 
					{
						res = inc;
						return res;
					}
					else 
					{
						// the incident already out of perception, no need check far more
						return nullptr;
					}
				} //end inc
				
				itemDis += rs->getLengthOfSegment();
			}
		} //end for obstacles
	} //end for segs

	return res;
}

double sim_mob::DriverMovement::getDisToStopPoint(double perceptionDis)
{
	double distance=-100;
	std::vector<const sim_mob::RoadSegment*>::iterator currentSegIt = fwdDriverMovement.currSegmentIt;
	std::vector<const sim_mob::RoadSegment*>::iterator currentSegItEnd = fwdDriverMovement.fullPath.end();

	// get moved distancd in current segment
	double movedis =  fwdDriverMovement.getCurrDistAlongRoadSegmentCM() / 100.0;
	
	double itemDis = fwdDriverMovement.getCurrentSegmentLengthCM() / 100.0 -
				fwdDriverMovement.getCurrDistAlongRoadSegmentCM() / 100.0;

	int i=0;
	
	for (; currentSegIt != currentSegItEnd; ++currentSegIt) 
	{
		if (currentSegIt == currentSegItEnd) 
		{
			break;
		}
		
		// get segment
		const RoadSegment* rs = *currentSegIt;
		if (!rs) 
		{
			break;
		}

		// get segment aimsun id
		std::stringstream segmentID("");
		segmentID << rs->getId();
		std::string id = segmentID.str();
		
		// get move distance in current seg
		// get param
		DriverUpdateParams& p = parentDriver->getParams();

		// check if has stop point of the segment
		std::map<std::string,std::vector<StopPoint> >::iterator it = p.stopPointPool.find(id);
		
		if(it!=p.stopPointPool.end())
		{
			std::vector<StopPoint> &v = it->second;
			
			for (int i = 0; i < v.size(); ++i)
			{
				if (rs == fwdDriverMovement.getCurrSegment()) 
				{
					if(v[i].distance<=perceptionDis)
					{
						distance = v[i].distance - movedis;

						if (distance<-10)
						{
							return -100;
						}

						if (distance > perceptionDis)
						{
							return -100;
						}
						
						p.currentStopPoint = v[i];

						return distance;// same segment
					}
				}// end of getCurrSegment
				else
				{
					// in forward segment
					if(rs->getLink() == fwdDriverMovement.getCurrSegment()->getLink())
					{
						// in same link
						distance = itemDis + v[i].distance;

						if (distance > perceptionDis)
						{
							return -100;
						}
						
						p.currentStopPoint = v[i];

						return distance;// same segment
					}//end if link
					else
					{
						// already in next link
						return -100;
					}
				}//end else
			}//end for
		}

		// rs has no stop point , check next segment
		if(i != 0)
		{
			itemDis += rs->getLengthOfSegment()/100.0;
		}

		i++;

		if (itemDis > perceptionDis)
		{
			return -100;
		}
	}//end of for
	
	return distance;
}

void sim_mob::DriverMovement::getLanesConnectToLookAheadDis(double distance, std::vector<sim_mob::Lane*>& lanePool) 
{
	std::vector<const sim_mob::RoadSegment*>::iterator currentSegIt = fwdDriverMovement.currSegmentIt;
	
	++currentSegIt; // next segment
	
	std::vector<const sim_mob::RoadSegment*>::iterator currentSegItEnd = fwdDriverMovement.fullPath.end();

	const sim_mob::RoadSegment* currentSeg = fwdDriverMovement.getCurrSegment();
	const std::vector<sim_mob::Lane*> lanes = currentSeg->getLanes();

	//check each lanes of current segment
	int maxLaneNumber = 8;
	for (int i = 0; i < maxLaneNumber; ++i) 
	{
		double x = fwdDriverMovement.getDisToCurrSegEnd() / 100.0;
		sim_mob::Lane* l = NULL;
		
		if (i < lanes.size())
		{
			l = lanes[i];
		} 
		else
		{
			// use most left lane
			if (lanes.at(lanes.size() - 1)->is_pedestrian_lane())
			{
				l = lanes.at(lanes.size() - 2);
			} else
			{
				l = lanes.at(lanes.size() - 1);
			}
		}

		currentSegIt = fwdDriverMovement.currSegmentIt;
		++currentSegIt; // current's next segment
		
		size_t landIdx = i;

		for (; currentSegIt != currentSegItEnd; ++currentSegIt)
		{

			bool isLaneOK = true;
			// already reach end of path
			if (currentSegIt + 1 == currentSegItEnd)
			{
				if (l)
				{
					lanePool.push_back(l);
				}
				break;
			}

			const RoadSegment* rs = *currentSegIt;

			x += rs->getLengthOfSegment() / 100.0;
			
			if (!rs)
			{
				break;
			}

			// find last segment
			// check lane landIdx of rs 's previous segment can connect to rs
			if (rs != fwdDriverMovement.fullPath[0])
			{
				std::vector<const sim_mob::RoadSegment*>::iterator it = currentSegIt - 1;
				const RoadSegment* lastSeg = *it;
				
				if (lastSeg->getLanes().size() < landIdx) // target segment lane size smaller than index
				{
					isLaneOK = false;
				}
				else
				{
					if (landIdx < lastSeg->getLanes().size())
					{
						sim_mob::Lane* lane = lastSeg->getLanes().at(landIdx);
						isLaneOK = isLaneConnectedToSegment(lane, rs);
					} 
					else
					{
						isLaneOK = false;
					}
				}
			} 
			else
			{
				if (l)
				{
					isLaneOK = isLaneConnectedToSegment(l, rs);
				} 
				else
				{
					isLaneOK = false;
				}
			}

			if (!isLaneOK)
			{
				break;
			}
			
			// l can connect to next segment
			if (x > distance)
			{
				// if this lane index is ok, but is pedestrian lane, then use its right lane
				if (l->is_pedestrian_lane())
				{
					if (i != 0)
					{
						l = lanes[i - 1];
					}
					else
					{
						l=NULL;
					}
				}
				// push to pool
				bool ff = false;
				
				for (int jj=0; jj < lanePool.size(); ++jj)
				{
					if (lanePool[jj] == l)
					{
						ff = true;
					}
				}
				
				if (!ff)
				{
					if (l)
					{
						lanePool.push_back(l);
					}
				}

				break;
			}
		} //end of for currentSegIt
	} //end for lanes
}

bool sim_mob::DriverMovement::isLaneConnectedToSegment(sim_mob::Lane* lane, const sim_mob::RoadSegment* rs) 
{
	bool isLaneOK = false;
	size_t landIdx = getLaneIndex(lane);
	const RoadSegment* from = lane->getRoadSegment();
	//check if segment end node is intersection
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*>(from->getEnd());

	if (currEndNode) 
	{
		// if intersection get turnings
		const std::set<TurningSection *>& turnings = currEndNode->getTurnings(from);

		if (!turnings.empty()) 
		{
			for (std::set<TurningSection *>::const_iterator it = turnings.begin(); it != turnings.end(); ++it) 
			{	
				if ((*it)->getLaneTo()->getRoadSegment() == rs && (*it)->getLaneFrom() == lane) 
				{
					// current lane connected to next link
					isLaneOK = true;
					break;
				}
			} //end for
		} 
		else 
		{
			// Turnings not found, defaulting to lane connectors
			Warn() << "\nTurnings not found for Node: " << from->getEnd()->getID();
			Warn() << "\nDefaulting to Lane Connectors\n";
			
			//Get the outgoing lane connectors
			const std::set<LaneConnector *>& lcs = currEndNode->getOutgoingLanes(from);
			
			if(!lcs.empty())
			{
				for (std::set<LaneConnector *>::const_iterator it = lcs.begin(); it != lcs.end(); ++it)
				{
					if ((*it)->getLaneTo()->getRoadSegment() == rs && (*it)->getLaneFrom() == lane)
					{
						// current lane connected to next link
						isLaneOK = true;
						break;
					}
				}
			}
			else
			{
				isLaneOK = false;
			}
		}
	} 
	else 
	{
		// uni node
		// TODO use uni node lane connector to check if lane connect to next segment
		if (rs->getLanes().size() > landIdx) {
			isLaneOK = true;
		}
	}

	return isLaneOK;
}

bool sim_mob::DriverMovement::isPedestrianOnTargetCrossing() const 
{
	if ((!trafficSignal) || (!(fwdDriverMovement.getNextSegment(true)))) 
	{
		return false;
	}

	sim_mob::Link * targetLink = fwdDriverMovement.getNextSegment(true)->getLink();
	const Crossing* crossing = nullptr;
	const LinkAndCrossingC& LAC = trafficSignal->getLinkAndCrossing();
	LinkAndCrossingC::iterator it = LAC.begin();
	
	for (; it != LAC.end(); ++it) 
	{
		if (it->link == targetLink) 
		{
			break;
		}
	}

	if (it != LAC.end()) 
	{
		crossing = (*it).crossing;
	}
	else 
	{
		return false;
	}
	
	//Have we found a relevant crossing?
	if (!crossing) 
	{
		return false;
	}

	return false;
}

double sim_mob::DriverMovement::dwellTimeCalculation(int A, int B, int delta_bay, int delta_full, int Pfront, int no_of_passengers) 
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
	std::cout << "Dwell__time " << DTijk << std::endl;
	
	return DTijk;
}

//update left and right lanes of the current lane
//if there is no left or right lane, it will be null
void sim_mob::DriverMovement::updateAdjacentLanes(DriverUpdateParams& p) 
{
	//Need to reset, we can call this after DriverUpdateParams is initialised.
	p.leftLane = nullptr;
	p.rightLane = nullptr;
	p.leftLane2 = nullptr;
	p.rightLane2 = nullptr;
	
	if (!p.currLane) 
	{
		return; //Can't do anything without a lane to reference.
	}
	
	const size_t numLanes = p.currLane->getRoadSegment()->getLanes().size();
	
	if (numLanes == 1) 
	{
		return; 
	}

	if (p.currLaneIndex > 0) 
	{
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex - 1);
		
		if (!temp->is_pedestrian_lane())
		{
			p.rightLane = temp;
		}
	}
	
	if (p.currLaneIndex > 1) 
	{
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex - 2);
		
		if (!temp->is_pedestrian_lane())
		{
			p.rightLane2 = temp;
		}
	}

	if (p.currLaneIndex < numLanes - 1) 
	{
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex + 1);
		
		if (!temp->is_pedestrian_lane())
		{
			p.leftLane = temp;
		}
	}

	if (p.currLaneIndex < numLanes - 2) 
	{
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex + 2);
		
		if (!temp->is_pedestrian_lane())
		{
			p.leftLane2 = temp;
		}
	}
}

//General update information for whenever a Segment may have changed.
void sim_mob::DriverMovement::syncCurrLaneCachedInfo(DriverUpdateParams& p) 
{
	//The lane may have changed; reset the current lane index.
	p.currLaneIndex = getLaneIndex(p.currLane);

	//Update which lanes are adjacent.
	updateAdjacentLanes(p);

	//Update the length of the current road segment.
	p.currLaneLength = fwdDriverMovement.getTotalRoadSegmentLengthCM();

	//Finally, update target/max speed to match the new Lane's rules.
	p.maxLaneSpeed = fwdDriverMovement.getCurrSegment()->maxSpeed / KILOMETER_PER_HOUR_TO_METER_PER_SEC;
	
	targetSpeed = p.maxLaneSpeed;
	p.desiredSpeed = targetSpeed;
}

//Chooses the next lane in the next link based on the current lane and the available turnings
//Note that this also sets the target lane so that we (hopefully) merge before the intersection.
void sim_mob::DriverMovement::chooseNextLaneForNextLink(DriverUpdateParams& p) 
{
	//The current segment
	const RoadSegment *currSeg = fwdDriverMovement.getCurrSegment();
	
	//The next segment
	const RoadSegment *nextSeg = fwdDriverMovement.getNextSegment(false);
	
	//The current lane
	const Lane *currLane = fwdDriverMovement.getCurrLane();
	
	nextLaneInNextLink = nullptr;
	
	//Ensure they are connected by the same multi-node 
	if(currSeg->getEnd() == nextSeg->getStart())
	{
		//Get the approaching multi-node
		const MultiNode *currEndNode = dynamic_cast<const MultiNode *> (currSeg->getEnd());
		
		//Get the set of turnings from the current segment
		const std::set<TurningSection *> turnings = currEndNode->getTurnings(currSeg);
		
		if (!turnings.empty())
		{
			//Look for the turning that has the 'to' RoadSegment as the next RoadSegment
			for (std::set<TurningSection*>::const_iterator itTurnings = turnings.begin(); itTurnings != turnings.end(); ++itTurnings)
			{
				//Check if this turning has a from lane that is same as the current lane and the to segment
				//that is the same as the next segment
				if (currLane == (*itTurnings)->getLaneFrom() && nextSeg == (*itTurnings)->getToSeg())
				{
					targetLaneIndex = p.nextLaneIndex = (*itTurnings)->getTo_lane_index();
					nextLaneInNextLink = (*itTurnings)->getLaneTo();
					break;
				}
			}
		}
		//No turnings, default to using lane connectors
		else
		{
			Warn() << "\nTurnings not found for Node: " << currSeg->getEnd()->getID();
			Warn() << "\nDefaulting to Lane Connectors\n";
			
			const std::set<LaneConnector *> lcs = currEndNode->getOutgoingLanes(currSeg);
			vector<const Lane*> targetLanes;
			
			//Look for the lane connector that has the 'to' RoadSegment as the next RoadSegment
			for(std::set<LaneConnector*>::const_iterator itLCS = lcs.begin(); itLCS != lcs.end(); ++itLCS)
			{
				if ((*itLCS)->getLaneFrom() == currLane && (*itLCS)->getLaneTo()->getRoadSegment() == nextSeg
					&& !((*itLCS)->getLaneTo()->is_pedestrian_lane()))
				{
					//It's a valid lane.
					targetLanes.push_back((*itLCS)->getLaneTo());					
				}
			}
			
			//Try to stay in the same lane
			if (!targetLanes.empty())
			{	
				std::vector<const Lane *>::const_iterator itTargetLanes = targetLanes.begin();
				while(itTargetLanes != targetLanes.end())
				{					
					if(getLaneIndex(*itTargetLanes) == getLaneIndex(currLane))
					{
						nextLaneInNextLink = *itTargetLanes;
						targetLaneIndex = getLaneIndex(nextLaneInNextLink);
						break;
					}
					++itTargetLanes;
				}
				
				if(!nextLaneInNextLink)
				{
					nextLaneInNextLink = *(targetLanes.begin());
					targetLaneIndex = getLaneIndex(nextLaneInNextLink);
				}
			}
		}
		
		/*It is possible that the nextLaneInNextLink is null at this point. The lane changing model will
		 find the lane that a vehicle needs to change to in order to get a valid nextLaneInNextLink at a later
		 stage.
		*/
	}
	else
	{
		throw std::runtime_error("Road Segments not connected to the same Multi-node");
	}
}

//TODO: For now, we're just using a simple trajectory model. Complex curves may be added later.
void sim_mob::DriverMovement::calculateIntersectionTrajectory(DPoint movingFrom, double overflow) 
{
	//If we have no target link, we have no target trajectory.
	if (!nextLaneInNextLink) 
	{
		Warn() << "WARNING: nextLaneInNextLink has not been set; can't calculate intersection trajectory." << std::endl;
		return;
	}
	
	Point2D entry = nextLaneInNextLink->getPolyline().at(0);
	
	//Check if we are currently using the MITSIM or slot-based model
	if(intModel->getIntModelType() != Int_Model_Simple)
	{
		//Ensure that we have a turning
		if(fwdDriverMovement.currTurning)
		{
			intModel->setCurrTurning(fwdDriverMovement.currTurning);
		}
		//We do not have a turning, so switch to the simple intersection driving model
		else
		{
			if(intModelBkUp)
			{
				//Swap the in use model with the back-up
				IntersectionDrivingModel *temporary = intModel;
				intModel = intModelBkUp;
				intModelBkUp = temporary;
			}
			else
			{
				//Create the simple intersection driving model and use it
				intModelBkUp = intModel;
				intModel = new SimpleIntDrivingModel();
			}
		}
	}
	
	intModel->startDriving(movingFrom, DPoint(entry.getX(), entry.getY()), overflow);
}

//Try to initialise only the path from the current location to the next activity location
///Returns the new vehicle, if requested to build one.
Vehicle* sim_mob::DriverMovement::initializePath(bool allocateVehicle) 
{
	Vehicle* res = nullptr;

	//Only initialise if the next path has not been planned for yet.
	if (!parent->getNextPathPlanned()) 
	{
		//Save local copies of the parent's origin/destination nodes.
		parentDriver->origin.node = parent->originNode.node_;
		parentDriver->origin.point = parentDriver->origin.node->location;
		parentDriver->goal.node = parent->destNode.node_;
		parentDriver->goal.point = parentDriver->goal.node->location;

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> path;

		sim_mob::SubTrip* subTrip = (&(*(parent->currSubTrip)));

		if(!parent->amodPath.empty())
		{
			path = parent->amodPath;
			// set the stop point and dwell time
			std::string stopSegmentStr = parent->amodPickUpSegmentStr;

			double dwelltime = 5; //in sec
			double segl = parent->amodSegmLength /100.0; //length of the segment in m
			double fd = (segl - segl/5); //distance where the vh will stop counting from the beginning of the segment

			StopPoint stopPoint(stopSegmentStr,fd,dwelltime);
			parentDriver->getParams().insertStopPoint(stopPoint);

			// set the stop point and dwell time for dropping off the passenger
			std::string dropOffSegmentStr = parentDriver->getParent()->amodDropOffSegmentStr;
	
			double segld = parentDriver->getParent()->amodSegmLength2 /100.0; //length of the segment in m
			double fd2 = (segld - segld/5); //distance where the vh will stop counting from the beginning of the segment
			StopPoint stopPoint2(dropOffSegmentStr,fd2,dwelltime);
			parentDriver->getParams().insertStopPoint(stopPoint2);
		}
		else
		{
			const StreetDirectory& stdir = StreetDirectory::instance();

			if (subTrip->schedule == nullptr)
			{
				// if use path set
				if (ConfigManager::GetInstance().FullConfig().PathSetMode())
				{
					path = PathSetManager::getInstance()->getPath(*(parent->currSubTrip), false, nullptr);
				}
				else
				{
					const StreetDirectory& stdir = StreetDirectory::instance();
					path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*(parentDriver->origin).node),
														   stdir.DrivingVertex(*(parentDriver->goal).node));
				}
			}
			else
			{
				std::vector<Node*>& routes = subTrip->schedule->routes;
				std::vector<Node*>::iterator first = routes.begin();
				std::vector<Node*>::iterator second = first;

				path.clear();
				
				for (++second; first != routes.end() && second != routes.end(); ++first, ++second)
				{
					vector<WayPoint> subPath = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(**first), stdir.DrivingVertex(**second));
					path.insert(path.end(), subPath.begin(), subPath.end());
				}
			}
		}

		//Empty paths aren't supported.
		if (path.empty()) 
		{
			throw std::runtime_error("Can't initializePath(); path is empty.");
		}

		//RoadRunner may need to know of our path, but it can't be send inevitably.
		if (parent->getRegionSupportStruct().isEnabled()) 
		{
			rrPathToSend.clear();
			for (std::vector<WayPoint>::const_iterator it = path.begin(); it != path.end(); ++it) 
			{
				if (it->type_ == WayPoint::ROAD_SEGMENT) 
				{
					rrPathToSend.push_back(it->roadSegment_);
				}
			}
		}

		int startLaneId = -1;

		// path[1] is currently the starting segment from the shortest driving path algorithm
		if (path[1].type_ == WayPoint::ROAD_SEGMENT) 
		{
			//Check if the desired lane is a valid lane for driving
			if(parent->laneID != -1)
			{
				//Ensure that the desired lane is not pedestrian lane
				if(! path[1].roadSegment_->getLane(parent->laneID)->is_pedestrian_lane())
				{
					startLaneId = parent->laneID;					
				}
			}
			
			//If we haven't found a valid lane or if it was not specified,
			//look for one
			if(startLaneId == -1)
			{
				for(parent->laneID = 0; parent->laneID < path[1].roadSegment_->getLanes().size(); parent->laneID++)
				{
					//Ensure that the lane is not pedestrian lane
					if(! path[1].roadSegment_->getLane(parent->laneID)->is_pedestrian_lane())
					{
						startLaneId = parent->laneID;
						break;
					}
				}
			}
		}
		else
		{
			parent->laneID = -1;
		}

		// Bus should be at least 1200 to be displayed on Visualiser
		const double length = dynamic_cast<BusDriver*>(this->getParentDriver()) ? 1200 : 400;
		const double width = 200;

		//A non-null vehicle means we are moving.
		if (allocateVehicle) 
		{
			res = new Vehicle(VehicleBase::CAR, length, width);
			buildAndSetPath(path, startLaneId);
		}

		if (subTrip->schedule && res) 
		{
			res->setFMOD_Schedule(subTrip->schedule);
		}

	}

	//to indicate that the path to next activity is already planned
	parent->setNextPathPlanned(true);
	return res;
}
void sim_mob::DriverMovement::rerouteWithPath(const std::vector<sim_mob::WayPoint>& path)
{
	//Else, pre-pend the current segment, and reset the current driver.
	//NOTE: This will put the current driver back onto the start of the current Segment, but since this is only
	//      used in Road Runner, it doesn't matter right now.
	//TODO: This *might* work if we save the current advance on the current segment and reset it.
	vector<WayPoint> newpath = path;
	vector<WayPoint>::iterator it = newpath.begin();
	newpath.insert(it, WayPoint(parentDriver->vehicle->getCurrSegment()));
	parentDriver->vehicle->resetPath(newpath);
}

void sim_mob::DriverMovement::rerouteWithBlacklist(
		const std::vector<const sim_mob::RoadSegment*>& blacklisted) {
	//Skip if we're somehow not driving on a road.
	if (!(parentDriver && parentDriver->vehicle
			&& fwdDriverMovement.getCurrSegment())) {
		return;
	}

	//Retrieve the shortest path from the current intersection node to destination and save all RoadSegments in this path.
	//NOTE: This path may be invalid, is there is no LaneConnector from the current Segment to the first segment of the result path.
	const RoadSegment* currSeg = fwdDriverMovement.getCurrSegment();
	const Node* node = currSeg->getEnd();
	const StreetDirectory& stdir = StreetDirectory::instance();
	vector<WayPoint> path = stdir.SearchShortestDrivingPath(
			stdir.DrivingVertex(*node),
			stdir.DrivingVertex(*(parentDriver->goal.node)), blacklisted);

	//Given this (tentative) path, we still have to transition from the current Segment.
	//At the moment this is a bit tedious (since we can't search mid-segment in the StreetDir), but
	// the following heuristic should work well enough.
	const RoadSegment* nextSeg = nullptr;
	if (path.size() > 1) { //Node, Segment.
		vector<WayPoint>::iterator it = path.begin();
		if ((it->type_ == WayPoint::NODE) && (it->node_ == node)) {
			++it;
			if (it->type_ == WayPoint::ROAD_SEGMENT) {
				nextSeg = it->roadSegment_;

			}
		}
	}

	//Now find the LaneConnectors. For a UniNode, this is trivial. For a MultiNode, we have to check.
	//NOTE: If a lane connector is NOT found, there may still be an alternate route... but we can't check this without
	// blacklisting the "found" segment and repeating. I think a far better solution would be to modify the
	// shortest-path algorithm to allow searching from Segments (the data structure already can handle it),
	// but for now we will have to deal with the rare true negative in cities with lots of one-way streets.
	if (nextSeg) {
		bool found = false;
		const MultiNode* mn = dynamic_cast<const MultiNode*>(node);
		if (mn) {
			const set<LaneConnector*> lcs = mn->getOutgoingLanes(currSeg);
			for (set<LaneConnector*>::const_iterator it = lcs.begin();
					it != lcs.end(); ++it) {
				if (((*it)->getLaneFrom()->getRoadSegment() == currSeg)
						&& ((*it)->getLaneTo()->getRoadSegment() == nextSeg)) {
					found = true;
					break;
				}
			}
		}
		if (!found) {
			path.clear();
		}
	}

	//If there's no path, keep the current one.
	if (path.empty()) {
		return;
	}

	//Else, pre-pend the current segment, and reset the current driver.
	//NOTE: This will put the current driver back onto the start of the current Segment, but since this is only
	// used in Road Runner, it doesn't matter right now.
	//TODO: This *might* work if we save the current advance on the current segment and reset it.
	vector<WayPoint>::iterator it = path.begin();
	path.insert(it, WayPoint(fwdDriverMovement.getCurrSegment()));
	resetPath(path);

	//Finally, update the client with the new list of Region tokens it must acquire.
	setRR_RegionsFromCurrentPath();
}

void sim_mob::DriverMovement::setOrigin(DriverUpdateParams& p) 
{
	//Set the max speed and target speed.res
	p.maxLaneSpeed = fwdDriverMovement.getCurrSegment()->maxSpeed / KILOMETER_PER_HOUR_TO_METER_PER_SEC;
	targetSpeed = p.maxLaneSpeed;

	p.desiredSpeed = targetSpeed;

	//Set our current and target lanes.
	p.currLane = fwdDriverMovement.getCurrLane();
	p.currLaneIndex = getLaneIndex(p.currLane);
	targetLaneIndex = p.currLaneIndex;

	//Vehicles start at rest (or may be given initial speed in configuration file)
	parentDriver->vehicle->setVelocity(p.initSpeed * 100); //Convert m/s to cm/s
	parentDriver->vehicle->setLatVelocity(0);
	parentDriver->vehicle->setAcceleration(0);

	//Calculate and save the total length of the current poly-line.
	p.currLaneLength = fwdDriverMovement.getTotalRoadSegmentLengthCM();

	setTrafficSignal();
	if (!(hasNextSegment(true)) && hasNextSegment(false)) 
	{
		//Don't do this if there is no next link.
		chooseNextLaneForNextLink(p);
	}
}

sim_mob::DynamicVector sim_mob::DriverMovement::getCurrPolylineVector() const
{
	return DynamicVector(fwdDriverMovement.getCurrPolypoint().getX(),
						 fwdDriverMovement.getCurrPolypoint().getY(),
						 fwdDriverMovement.getNextPolypointNew().getX(),
						 fwdDriverMovement.getNextPolypointNew().getY());
}

double sim_mob::DriverMovement::updatePositionOnLink(DriverUpdateParams& p) 
{
	//Determine how far forward we've moved.
	double fwdDistance = parentDriver->vehicle->getVelocity() * p.elapsedSeconds
			+ 0.5 * parentDriver->vehicle->getAcceleration() * p.elapsedSeconds
			* p.elapsedSeconds;
	
	if (fwdDistance < 0) 
	{
		fwdDistance = 0;
	}

	if (incidentPerformer.getIncidentStatus().getCurrentStatus() == IncidentStatus::INCIDENT_CLEARANCE
		&& incidentPerformer.getIncidentStatus().getCurrentIncidentLength() > 0)
	{
		incidentPerformer.getIncidentStatus().reduceIncidentLength(fwdDistance);
	}

	//Increase the vehicle's velocity based on its acceleration.
	double vel = parentDriver->vehicle->getVelocity() + parentDriver->vehicle->getAcceleration() * p.elapsedSeconds;

	if (vel < 0)
	{
		vel = 0;
	} 

	parentDriver->vehicle->setVelocity(vel);

	//Move the vehicle forward.
	double res = fwdDriverMovement.advance(fwdDistance);

	//Retrieve what direction we're moving in, since it will "flip" if we cross the relative X axis.
	//LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();
	//after forwarding, adjacent lanes might be changed
	updateAdjacentLanes(p);
	
	//there is no left lane when turning left
	//or there is no right lane when turning right
	if ((parentDriver->vehicle->getTurningDirection() == LCS_LEFT && !p.leftLane)
			|| (parentDriver->vehicle->getTurningDirection() == LCS_RIGHT && !p.rightLane)) 
	{
		parentDriver->vehicle->setLatVelocity(0);
		p.newLatVelM = 0.0;
	}

	//Lateral movement
	if (!(fwdDriverMovement.isInIntersection())) 
	{
		updateLateralMovement(p);
	}

	//Update our offset in the current lane.
	if (!(fwdDriverMovement.isInIntersection())) 
	{
		p.currLaneOffset = fwdDriverMovement.getCurrDistAlongRoadSegmentCM();
	}
	
	return res;
}

void sim_mob::DriverMovement::setNearestVehicle(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other) 
{
	//Subtract the size of the car from the distance between them
	distance = fabs(distance) - veh->getLengthCm() / 2 - other->getVehicleLengthCM() / 2;

	if (distance <= res.distance) 
	{
		res.driver = other;
		res.distance = distance;
	}
}

//TODO: I have the feeling that this process of detecting nearby drivers in front of/behind you and saving them to
// the various CFD/CBD/LFD/LBD variables can be generalised somewhat. I shortened it a little and added a
// helper function; perhaps more cleanup can be done later? ~Seth
bool sim_mob::DriverMovement::updateNearbyAgent(const Agent* other, const Driver* other_driver) 
{	
	DriverUpdateParams& params = parentDriver->getParams();
	
	//Only update if passed a valid pointer which is not a pointer back to us
	if(!other_driver || this->parentDriver == other_driver) 
	{
		return false;
	}
	
	//1.0 Get the current turning the other vehicle is on
	const TurningSection* otherTurning = other_driver->currTurning_.get();
	
	//Check if both drivers have a valid turning - meaning they are approaching or in the intersection
	if (fwdDriverMovement.currTurning && otherTurning) 
	{
		//1.1 Get the turning conflict - if there is no conflict, it either means they are 
		//on the same turning or are on / approaching different intersections
		TurningConflict* conflict = fwdDriverMovement.currTurning->getTurningConflict(otherTurning);
		if(conflict) 
		{
			double conflictDist = (otherTurning == conflict->getFirstTurning()) ? conflict->getFirst_cd() : conflict->getSecond_cd();
			double distance = 0;
			
			if(other_driver->isInIntersection_.get())
			{
				//2.0 Get distance covered by the other vehicle on the turning and calculate how far it
				//is from the conflict point
				double distCoveredOnTurning = other_driver->moveDisOnTurning_.get() / 100.0;
				distance = distCoveredOnTurning - conflictDist;
			}
			else
			{
				//2.0 Get distance from the conflict point
				//As the distance covered on turning is smaller than conflict distance, the value calculated above is 
				//negative. To keep consistency, we negate the distance to intersection
				//This means the distance to conflict point will always be negative, with smaller value indicating that it is 
				//further away
				distance = -other_driver->distToIntersection_.get() - conflictDist;
			}
			
			// 2.1 Store the vehicle in the map of conflicting vehicles along with the distance from conflict point
			params.insertConflictTurningDriver(conflict, distance, other_driver);
		}
		//If the vehicles are on the same turning, then one is following the other
		else if(fwdDriverMovement.currTurning == otherTurning)
		{
			double distance = 0;
			
			//Check if we are in an intersection
			if(parentDriver->isInIntersection_.get() && other_driver->isInIntersection_.get())
			{
				//As both vehicles are in the intersection, compare the distances covered on the turning
				//and identify which of us is in the front
				distance = other_driver->moveDisOnTurning_.get() - parentDriver->moveDisOnTurning_.get();
				bool isForward = distance > 0;
				setNearestVehicle(isForward ? params.nvFwd : params.nvBack, distance, parentDriver->vehicle, other_driver);
			}
			else if(other_driver->isInIntersection_.get())
			{
				//As we are not in the intersection, but the other vehicle is, it is obviously in front of us
				distance = fwdDriverMovement.getDisToCurrSegEnd() + other_driver->moveDisOnTurning_.get();
				setNearestVehicle(params.nvFwd, distance, parentDriver->vehicle, other_driver);
			}
			else if(parentDriver->isInIntersection_.get())
			{
				//As we are in the intersection, but the other vehicle isn't, it is obviously in behind us
				distance = other_driver->distToIntersection_.get() * 100 + parentDriver->moveDisOnTurning_.get();
				setNearestVehicle(params.nvBack, distance, parentDriver->vehicle, other_driver);
			}
		}
		//Both turnings originate at the same lane, but diverge
		else if (fwdDriverMovement.currTurning->getLaneFrom() == otherTurning->getLaneFrom() &&
				 fwdDriverMovement.currTurning->getLaneTo() != otherTurning->getLaneTo())
		{
			double distance = 0;
			
			//The other driver can be the front driver only for a few meters, say 5m
			//till the turning has some common area
			if(other_driver->isInIntersection_.get() && other_driver->moveDisOnTurning_.get() <= 5000)
			{
				distance = fwdDriverMovement.getDisToCurrSegEnd() + other_driver->moveDisOnTurning_.get();
				setNearestVehicle(params.nvFwd, distance, parentDriver->vehicle, other_driver);
			}
			else if(parentDriver->isInIntersection_.get() && parentDriver->moveDisOnTurning_.get() <= 5000)
			{
				distance = other_driver->distToIntersection_.get() * 100 + parentDriver->moveDisOnTurning_.get();
				setNearestVehicle(params.nvBack, distance, parentDriver->vehicle, other_driver);
			}
		}
	}
	
	//The other driver is in the intersection, we've already done the required updates above so return
	//But if he's approaching, we still may need to do other updates
	if(other_driver->isInIntersection_.get())
	{
		return true;
	}

	//Retrieve the other driver's lane, road segment, and lane offset.
	const Lane* other_lane = other_driver->currLane_.get();
	
	if (!other_lane) 
	{
		return false;
	}
	
	const RoadSegment* otherRoadSegment = other_lane->getRoadSegment();

	//we need the length of the link while calculating the lane level density
	//as we will be considering the vehicles on a particular lane of a link.
	double lengthInM = (double)fwdDriverMovement.getCurrLink()->getLength() / 100;

	double other_offset = other_driver->currDistAlongRoadSegment;

	//If the vehicle is in the same Road segment
	if (fwdDriverMovement.getCurrSegment() == otherRoadSegment) 
	{
		//Set distance equal to the _forward_ distance between these two vehicles.
		double distance = other_offset - fwdDriverMovement.getCurrDistAlongRoadSegmentCM();

		//Is the vehicle ahead of us
		bool fwd = distance > 0;

		//Set different variables depending on where the car is.
		
		//The vehicle is on the current lane
		if (other_lane == params.currLane) 
		{ 
			//Increment the lane level density as the other car is in the same lane
			params.density = params.density + (1.0f / lengthInM);

			setNearestVehicle((fwd ? params.nvFwd : params.nvBack), distance, parentDriver->vehicle, other_driver);
		} 
		//The vehicle is on the left lane
		else if (other_lane == params.leftLane) 
		{ 
			setNearestVehicle((fwd ? params.nvLeftFwd : params.nvLeftBack), distance, parentDriver->vehicle, other_driver);
		} 
		//The vehicle is on the right lane
		else if (other_lane == params.rightLane) 
		{ 
			setNearestVehicle((fwd ? params.nvRightFwd : params.nvRightBack), distance, parentDriver->vehicle, other_driver);
		}
		//The vehicle is on the second Left lane
		else if (other_lane == params.leftLane2) 
		{ 
			setNearestVehicle((fwd ? params.nvLeftFwd2 : params.nvLeftBack2), distance, parentDriver->vehicle, other_driver);
		}
		//The vehicle is on the second right lane
		else if (other_lane == params.rightLane2) 
		{ 
			setNearestVehicle((fwd ? params.nvRightFwd2 : params.nvRightBack2), distance, parentDriver->vehicle, other_driver);
		}
	} 
	//We are in the same link.
	else if (otherRoadSegment->getLink() == fwdDriverMovement.getCurrLink()) 
	{ 
		//Vehicle is on the next segment.
		if (fwdDriverMovement.getNextSegment(true) == otherRoadSegment) 
		{ 
			//Retrieve the next node we are moving to, cast it to a UniNode.
			const Node* nextNode = fwdDriverMovement.getCurrSegment()->getEnd();
			const UniNode* uNode = dynamic_cast<const UniNode*>(nextNode);
			
			const Lane* nextLane = nullptr;
			const Lane* nextLeftLane = nullptr;
			const Lane* nextRightLane = nullptr;
			const Lane* nextLeftLane2 = nullptr;
			const Lane* nextRightLane2 = nullptr;

			if (uNode) 
			{
				nextLane = uNode->getForwardDrivingLane(*params.currLane);
			}

			//Make sure next lane exists and is in the next road segment, although it should be true
			if (nextLane && nextLane->getRoadSegment() == otherRoadSegment) 
			{
				//Assign next left/right lane based on lane ID.
				size_t nextLaneIndex = getLaneIndex(nextLane);
				
				if (nextLaneIndex > 0) 
				{
					nextRightLane = otherRoadSegment->getLanes().at(nextLaneIndex - 1);
				}
				
				if (nextLaneIndex < otherRoadSegment->getLanes().size() - 1) 
				{
					nextLeftLane = otherRoadSegment->getLanes().at(nextLaneIndex + 1);
				}
				
				if (nextLaneIndex > 1) 
				{
					nextRightLane2 = otherRoadSegment->getLanes().at(nextLaneIndex - 2);
				}
				
				if (nextLaneIndex < otherRoadSegment->getLanes().size() - 2) 
				{
					nextLeftLane2 = otherRoadSegment->getLanes().at(nextLaneIndex + 2);
				}
			}

			//Modified distance.
			int distance = other_offset + params.currLaneLength - params.currLaneOffset;

			//The vehicle is on the current lane
			if (other_lane == nextLane) 
			{
				//Increment the lane level density as the other car is in the same lane
				//as we want to get into
				params.density = params.density + (1.0f / lengthInM);

				setNearestVehicle(params.nvFwd, distance, parentDriver->vehicle, other_driver);
			} 
			//The vehicle is on the left lane
			else if (other_lane == nextLeftLane) 
			{ 
				setNearestVehicle(params.nvLeftFwd, distance, parentDriver->vehicle, other_driver);
			} 
			//The vehicle is in front
			else if (other_lane == nextRightLane) 
			{ 
				setNearestVehicle(params.nvRightFwd, distance, parentDriver->vehicle, other_driver);
			}
			//The vehicle is on the second Left lane
			else if (other_lane == nextLeftLane2) 
			{ 
				setNearestVehicle(params.nvLeftFwd2, distance, parentDriver->vehicle, other_driver);
			}
			//The vehicle is on the second right lane
			else if (other_lane == nextRightLane2) 
			{
				setNearestVehicle(params.nvRightFwd2, distance, parentDriver->vehicle, other_driver);
			}
		} 
		//Vehicle is on the previous segment.
		else if (fwdDriverMovement.getPrevSegment(true) == otherRoadSegment) 
		{ 	
			const Lane* preLane = nullptr;
			const Lane* preLeftLane = nullptr;
			const Lane* preRightLane = nullptr;
			const Lane* preLeftLane2 = nullptr;
			const Lane* preRightLane2 = nullptr;

			//Find the node which leads to this one from the UniNode. (Requires some searching; should probably
			// migrate this to the UniNode class later).
			const vector<Lane*>& lanes = otherRoadSegment->getLanes();

			if(params.currLaneIndex < lanes.size())
			{
				preLane = lanes.at(params.currLaneIndex);
			}
			else
			{
				preLane = nullptr;
				preLeftLane = nullptr;
				preRightLane = lanes.at(params.currLaneIndex-1);
			}

			//Make sure next lane is in the next road segment, although it should be true
			if (preLane) 
			{
				//Save the new left/right lanes
				size_t preLaneIndex = getLaneIndex(preLane);
				
				if (preLaneIndex > 0) 
				{
					preRightLane = otherRoadSegment->getLanes().at(preLaneIndex - 1);
				}
				
				if (preLaneIndex < otherRoadSegment->getLanes().size() - 1) 
				{
					preLeftLane = otherRoadSegment->getLanes().at(preLaneIndex + 1);
				}
			}

			//Modified distance.
			int distance = other_driver->currLaneLength_.get() - other_offset + params.currLaneOffset;

			//The vehicle is on the current lane
			if (other_lane == preLane) 
			{ 
				setNearestVehicle(params.nvBack, distance, parentDriver->vehicle, other_driver);
			} 
			//The vehicle is on the left lane
			else if (other_lane == preLeftLane) 
			{ 
				setNearestVehicle(params.nvLeftBack, distance,
						parentDriver->vehicle, other_driver);
			} 
			//The vehicle is on the right lane
			else if (other_lane == preRightLane) 
			{ 
				setNearestVehicle(params.nvRightBack, distance,
						parentDriver->vehicle, other_driver);
			} 
			//The vehicle is on the second Left lane
			else if (other_lane == preLeftLane2) 
			{ 
				setNearestVehicle(params.nvLeftBack2, distance,
						parentDriver->vehicle, other_driver);
			} 
			//The vehicle is on the second right lane
			else if (other_lane == preRightLane2) 
			{ 
				setNearestVehicle(params.nvRightBack2, distance,
						parentDriver->vehicle, other_driver);
			}
		}
	}

	//We are in the different link.
	if (otherRoadSegment->getLink() != fwdDriverMovement.getCurrLink()) 
	{
		if (fwdDriverMovement.getNextSegment(false) == otherRoadSegment) 
		{
			//Vehicle is on the next segment,which is in next link after intersection.
			// 1. host vh's target lane is == other_driver's lane
			if(fwdDriverMovement.currTurning && fwdDriverMovement.currTurning->getLaneTo() == other_lane)
			{
				if (params.nvFwd.driver == NULL) 
				{
					// 2. other_driver's distance move in the segment, it is also the distance vh to intersection
					double currSegLen = fwdDriverMovement.getCurrentSegmentLengthCM();
					double distCoveredOnCurrSeg =	fwdDriverMovement.getCurrDistAlongRoadSegmentCM();
					double turningLen = fwdDriverMovement.currTurning->getLength();
					double otherdis = other_driver->currDistAlongRoadSegment;
					
					double distance = currSegLen + turningLen - distCoveredOnCurrSeg - parentDriver->moveDisOnTurning_.get() + otherdis;
					
					// 3. compare the distance and set params.nvFwdNextLink
					setNearestVehicle(params.nvFwdNextLink, distance, parentDriver->vehicle, other_driver);
				}
			}
		}
		
		// for CF acceleration merge
		// 1.0 check other driver's segment's end node
		if (fwdDriverMovement.getCurrSegment()->getEnd() == otherRoadSegment->getEnd()) 
		{
			size_t targetLaneIndex = params.nextLaneIndex; // target lane
			size_t otherVhLaneIndex = getLaneIndex(other_lane); // other vh's lane
			
			if (targetLaneIndex == otherVhLaneIndex) 
			{
				// 2.0 check current link's end node type and current segment type
				if (fwdDriverMovement.getCurrLink()->getEnd()->type == sim_mob::PRIORITY_MERGE_NODE
						&& fwdDriverMovement.getCurrSegment()->type == sim_mob::LINK_TYPE_RAMP) 					
				{
					// subject drive distance to priority merge node
					double currSL = fwdDriverMovement.getCurrentSegmentLengthCM();
					double disMIS = fwdDriverMovement.getCurrDistAlongRoadSegmentCM();
					double dis = currSL - disMIS;
					
					// other drive distance to priority merge node
					double otherDis = otherRoadSegment->length - other_driver->currDistAlongRoadSegment;
					
					// calculate distance of two vh
					double distance = dis - otherDis;
					
					if (distance >= 0) 
					{
						setNearestVehicle(params.nvLeadFreeway, distance, parentDriver->vehicle, other_driver);
					} 
					else 
					{
						setNearestVehicle(params.nvLagFreeway, -distance, parentDriver->vehicle, other_driver);
					}
				}
			}
		} 

		if (fwdDriverMovement.getCurrSegment()->getEnd() == otherRoadSegment->getStart()) 
		{
			// 3.0 check current link's end node type
			if (fwdDriverMovement.getCurrLink()->getEnd()->type == sim_mob::PRIORITY_MERGE_NODE && // toward priority merge node
				(fwdDriverMovement.getCurrSegment()->type == sim_mob::LINK_TYPE_RAMP || // either on ramp or freeway
				fwdDriverMovement.getCurrSegment()->type == sim_mob::LINK_TYPE_FREEWAY)) 
			{
				// subject drive distance to priority merge node
				double currSL = fwdDriverMovement.getCurrentSegmentLengthCM();
				double disMIS = fwdDriverMovement.getCurrDistAlongRoadSegmentCM();
				double dis = currSL - disMIS;
				
				// other drive distance moved on outgoing freeway
				double otherDis = other_driver->currDistAlongRoadSegment;
				
				// calculate distance of two vh
				double distance = dis + otherDis;
				
				setNearestVehicle(params.nvLeadFreeway, distance, parentDriver->vehicle, other_driver);
			} // end rampseg
		}
	} // end of in different link
	
	return true;
}

void sim_mob::DriverMovement::updateNearbyAgent(const Agent* other, const Pedestrian* pedestrian) 
{
	DriverUpdateParams& params = parentDriver->getParams();
	//Only update if passed a valid pointer and this is on a crossing.

	if (!(pedestrian && pedestrian->isOnCrossing())) 
	{
		return;
	}

	//TODO: We are using a vector to check the angle to the Pedestrian. There are other ways of doing this which may be more accurate.
	const std::vector<sim_mob::Point2D>& polyLine = fwdDriverMovement.getCurrSegment()->getLanes().front()->getPolyline();
	DynamicVector otherVect(polyLine.front().getX(), polyLine.front().getY(), other->xPos.get(), other->yPos.get());

	//Calculate the distance between these two vehicles and the distance between the angle of the
	// car's forward movement and the pedestrian.
	//NOTE: I am changing this slightly, since cars were stopping for pedestrians on the opposite side of
	// the road for no reason (traffic light was green). ~Seth
	//double distance = otherVect.getMagnitude();
	double angleDiff = 0.0;
	{
		//Retrieve
		DynamicVector fwdVector(getCurrPolylineVector());
		fwdVector.scaleVectTo(100);

		//Calculate the difference
		//NOTE: I may be over-complicating this... we can probably use the dot product but that can be done later. ~Seth
		double angle1 = atan2(fwdVector.getEndY() - fwdVector.getY(), fwdVector.getEndX() - fwdVector.getX());
		double angle2 = atan2(otherVect.getEndY() - otherVect.getY(), otherVect.getEndX() - otherVect.getX());
		double diff = fabs(angle1 - angle2);
		
		angleDiff = std::min(diff, fabs(diff - 2 * M_PI));
	}

	//If the pedestrian is not behind us, then set our flag to true and update the minimum pedestrian distance.
	//30 degrees +/-
	if (angleDiff < 0.5236) 
	{
		params.npedFwd.distance = std::min(params.npedFwd.distance, otherVect.getMagnitude() - parentDriver->vehicle->getLengthCm() / 2 - 300);
	}
}

double sim_mob::DriverMovement::getAngle() const 
{
	if (fwdDriverMovement.isDoneWithEntireRoute()) 
	{
		return 0; //Shouldn't matter.
	}

	DynamicVector temp(fwdDriverMovement.getCurrPolypoint().getX(),
					fwdDriverMovement.getCurrPolypoint().getY(),
					fwdDriverMovement.getNextPolypoint().getX(),
					fwdDriverMovement.getNextPolypoint().getY());

	return temp.getAngle();
}

void sim_mob::DriverMovement::updateNearbyAgents() 
{
	DriverUpdateParams& params = parentDriver->getParams();

	PROFILE_LOG_QUERY_START(parent->currWorkerProvider, parent, params.now);

	//NOTE: Let the AuraManager handle dispatching to the "advanced" function.
	vector<const Agent*> nearby_agents;
	
	if (parentDriver->getCurrPosition().x > 0 && parentDriver->getCurrPosition().y > 0) 
	{
		double distance = 10000.0;
		const Agent* parentAgent = (parentDriver ? parentDriver->getParent() : nullptr);
		
		//Retrieve a list of nearby agents
		nearby_agents = AuraManager::instance().nearbyAgents(Point2D(parentDriver->getCurrPosition().x,
																	parentDriver->getCurrPosition().y),
															*params.currLane, distance,
															parentDriver->distanceBehind,
															parentAgent);
	}
	else 
	{
		Warn()	<< "A driver's location (x or y) is < 0, X:"
				<< parentDriver->getCurrPosition().x << ",Y:"
				<< parentDriver->getCurrPosition().y << std::endl;
	}

	PROFILE_LOG_QUERY_END(parent->currWorkerProvider, parent, params.now);

	//Update each nearby Pedestrian/Driver
	params.nvFwdNextLink.driver = NULL;
	params.nvFwdNextLink.distance = DBL_MAX;
	params.nvLeadFreeway.driver = NULL;
	params.nvLeadFreeway.distance = DBL_MAX;
	params.nvLagFreeway.driver = NULL;
	params.nvLagFreeway.distance = DBL_MAX;
	params.nvFwd.driver = NULL;
	params.nvFwd.distance = DBL_MAX;

	for (vector<const Agent*>::iterator it = nearby_agents.begin(); it != nearby_agents.end(); ++it) 
	{
		//Perform no action on non-Persons
		const Person* other = dynamic_cast<const Person *>(*it);
		
		if (!other) 
		{
			continue;
		}

		if (!other->getRole()) 
		{
			continue;
		}

		//Perform a different action depending on whether or not this is a Pedestrian/Driver/etc.
		/*Note:
		 * In the following methods(updateNearbyDriver and updateNearbyPedestrian), the variable "other"
		 * is the target which is being analyzed, and the current object is the one who i object is the analyzer.
		 *
		 * In order to remove the ugly dynamic_cast s passed into the following method,the analyzed and anlayzer
		 * should switch their place and, consequently, the following methods and some of their sub-methods
		 * need to be rewritten. for now, we reduce the number of dynamic_casts by calling only one of the functions.
		 * It originally had to be like this(only one of them need to be called).
		 */
		other->getRole()->handleUpdateRequest(this);
	}
}

void sim_mob::DriverMovement::perceivedDataProcess(NearestVehicle& nv, DriverUpdateParams& params) 
{
	//Update your perceptions for leading vehicle and gap
	if (nv.exists()) 
	{
		if (parentDriver->reactionTime == 0) 
		{
			params.perceivedFwdVelocityOfFwdCar = nv.driver ? nv.driver->fwdVelocity_.get() : 0;
			params.perceivedLatVelocityOfFwdCar = nv.driver ? nv.driver->latVelocity_.get() : 0;
			params.perceivedAccelerationOfFwdCar = nv.driver ? nv.driver->fwdAccel_.get() : 0;
			params.perceivedDistToFwdCar = nv.distance;
			return;
		}
		
		parentDriver->perceivedDistToFwdCar->delay(nv.distance);
		parentDriver->perceivedVelOfFwdCar->delay(nv.driver->fwdVelocity_.get());
		parentDriver->perceivedAccOfFwdCar->delay(nv.driver->fwdAccel_.get());
	}
	else
	{
		params.perceivedDistToFwdCar = Driver::maxVisibleDis;
		params.perceivedFwdVelocityOfFwdCar = 1900;
		params.perceivedAccelerationOfFwdCar = 500;
	}
}

void sim_mob::DriverMovement::updateIntersectionVelocity() 
{
	DriverUpdateParams& params = parentDriver->getParams();
	
	//Update the acceleration
	parentDriver->vehicle->setAcceleration(params.newFwdAcc * METER_TO_CENTIMETER_CONVERT_UNIT);
	
	//Calculate the new speed
	double inter_speed = parentDriver->vehicle->getVelocity() +
			(params.newFwdAcc * METER_TO_CENTIMETER_CONVERT_UNIT * params.elapsedSeconds);
	
	//Ensuring speed is non-negative
	if(inter_speed <= 0)
	{
		inter_speed = 0;
		
		if(params.impatienceTimerStart > 0)
		{
			params.impatienceTimer = params.now.frame() - params.impatienceTimerStart;
		}
		else
		{
			params.impatienceTimerStart = params.now.frame();
		}
	}
	else
	{
		//reset the timer, we're not waiting
		params.impatienceTimer = params.impatienceTimerStart = 0;
	}

	//Set velocity for intersection movement
	parentDriver->vehicle->setVelocity(inter_speed);
}

void sim_mob::DriverMovement::postIntersectionDriving(DriverUpdateParams& p) 
{
	p.currLaneIndex = getLaneIndex(nextLaneInNextLink);
	fwdDriverMovement.moveToNewPolyline(p.currLaneIndex);
	
	syncCurrLaneCachedInfo(p);
	
	p.currLaneOffset = fwdDriverMovement.getCurrDistAlongRoadSegmentCM();
	targetLaneIndex = p.currLaneIndex;

	//Reset lateral movement/velocity to zero.
	parentDriver->vehicle->setLatVelocity(0);
	parentDriver->vehicle->resetLateralMovement();
}

void sim_mob::DriverMovement::updateLateralMovement(DriverUpdateParams& p)
{
	// TODO check if STATUS_LC_CHANGING

	// 1.0 get lateral speed
	double lateralSpeedM = p.newLatVelM;

	// 1.1 calculate lateral movement distance of current tick
	double lateralMoveDisTickM = lateralSpeedM * p.elapsedSeconds;

	// 1.2 update vehicle's latMovement
	parentDriver->vehicle->moveLat(lateralMoveDisTickM * 100);

	// 2.0 check if lane changing operation completed
	double lateralMovementCM = parentDriver->vehicle->getLateralMovement();
	lateralMovementCM = abs(lateralMovementCM);

	double halfLaneWidthCM = p.currLane->getWidth() *0.8;
	
	if(lateralMovementCM > halfLaneWidthCM)
	{
		//    move beyond of mid line of the lane
		//    means vh moved to target lane
		//2.1 Update Lanes, polylines, RoadSegments, etc.
		syncInfoLateralMove(p);

		if (p.currLane->is_pedestrian_lane()) 
		{
			//Flush debug output (we are debugging this error).
			if (Debug::Drivers) 
			{
				if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) 
				{
					DebugStream << ">>>Exception: Moved to sidewalk."
								<< endl;
					PrintOut(DebugStream.str());
				}
			}

			std::stringstream msg;
			msg << "Error: Car has moved onto sidewalk. Agent ID: "
				<< parent->getId();
			throw std::runtime_error(msg.str().c_str());
		}

		parentDriver->vehicle->resetLateralMovement();

		// complete lane change
		p.unsetFlag(FLAG_PREV_LC); // clean bits

		if (p.getStatus(STATUS_LEFT)) 
		{
			p.setFlag(FLAG_PREV_LC_LEFT);
		}
		else 
		{
			p.setFlag(FLAG_PREV_LC_RIGHT);
		}
		
		p.unsetStatus(STATUS_CHANGING);
		p.lcTimeTag = p.now.ms();

		// lane change complete, unset the "performing lane change" status
		p.unsetStatus(STATUS_LC_CHANGING);
		p.unsetStatus(STATUS_MANDATORY);
		p.unsetFlag(FLAG_NOSING | FLAG_YIELDING | FLAG_LC_FAILED);
		p.unsetFlag(FLAG_VMS_LANE_USE_BITS | FLAG_ESCAPE | FLAG_AVOID);
		p.unsetFlag(FLAG_STUCK_AT_END | FLAG_NOSING_FEASIBLE);
		p.unsetStatus(STATUS_TARGET_GAP);
	}
}

void sim_mob::DriverMovement::syncInfoLateralMove(DriverUpdateParams& p)
{
	if (p.getStatus(STATUS_LC_RIGHT)) 
	{
		if(p.rightLane)
		{
			p.currLane = p.rightLane;
		}
	}
	else if (p.getStatus(STATUS_LC_LEFT)) 
	{
		if(p.leftLane)
		{
			p.currLane = p.leftLane;
		}
	}
	else 
	{
		std::stringstream msg;
		msg << "syncInfoLateralMove (" << parent->getId()
			<< ") is attempting to change lane when no lane changing decision made";
		throw std::runtime_error(msg.str().c_str());
	}

	//The lane may have changed; reset the current lane index.
	p.currLaneIndex = getLaneIndex(p.currLane);

	//Update which lanes are adjacent.
	updateAdjacentLanes(p);

	//Update the length of the current road segment.
	p.currLaneLength = fwdDriverMovement.getTotalRoadSegmentLengthCM();

	//update max speed of Lane's rules.
	p.maxLaneSpeed = fwdDriverMovement.getCurrSegment()->maxSpeed
			/ KILOMETER_PER_HOUR_TO_METER_PER_SEC;

	// update lane polyline data;
	// is it necessary? as when calculate lateral position only use lane zero poly-line and current lane index
	fwdDriverMovement.moveToNewPolyline(p.currLaneIndex);
}

//Retrieve the current traffic signal based on our RoadSegment's end node.
void sim_mob::DriverMovement::setTrafficSignal() 
{
	const Node *node = nullptr;
	
	if (fwdDriverMovement.isMovingForwardsInLink)
	{
		node = fwdDriverMovement.getCurrLink()->getEnd();
	}
	else
	{
		node = fwdDriverMovement.getCurrLink()->getStart();
	}
	
	trafficSignal = node ? StreetDirectory::instance().signalAt(*node) : nullptr;
}

void sim_mob::DriverMovement::setTrafficSignalParams(DriverUpdateParams& p) 
{
	if (!trafficSignal) 
	{
		p.trafficColor = sim_mob::Green;
		parentDriver->perceivedTrafficColor->delay(p.trafficColor);
	}
	else 
	{
		sim_mob::TrafficColor color;

		if (hasNextSegment(false)) 
		{
			const Lane *nextLinkLane = hasNextSegment(false)->getLane(0);
			color = trafficSignal->getDriverLight(*p.currLane, *nextLinkLane);
		}
		else 
		{
			/*vahid:
			 * Basically,there is no notion of left, right forward any more.
			 * (I said "Basically" coz I can think of at least one "if" :left turn in singapore, right turn in US...)
			 * so it is omitted by If you insist on having this type of function, I can give you a vector/container
			 * of a map between lane/link and their corresponding current color with respect to the currLane
			 */
			color = sim_mob::Green;
		}
		
		switch (color) 
		{
		case sim_mob::Red:
			p.trafficColor = color;
			break;
		case sim_mob::Amber:
		case sim_mob::Green:
			if (!isPedestrianOnTargetCrossing())
				p.trafficColor = color;
			else
				p.trafficColor = sim_mob::Red;
			break;
		default:
			Warn() << "Unknown signal color[" << color << "]\n";
			break;
		}

		if (!parentDriver->perceivedTrafficColor->can_sense()) 
		{
			p.perceivedTrafficColor = p.trafficColor;
		}

		parentDriver->perceivedTrafficColor->delay(p.trafficColor);

		p.trafficSignalStopDistance =
				fwdDriverMovement.getAllRestRoadSegmentsLengthCM()
				- fwdDriverMovement.getCurrDistAlongRoadSegmentCM()
				- parentDriver->vehicle->getLengthCm() / 2;

		if (!parentDriver->perceivedDistToTrafficSignal->can_sense()) 
		{
			p.perceivedDistToTrafficSignal = p.trafficSignalStopDistance;
		}
		
		parentDriver->perceivedDistToTrafficSignal->delay(p.trafficSignalStopDistance);
	}
}

}
