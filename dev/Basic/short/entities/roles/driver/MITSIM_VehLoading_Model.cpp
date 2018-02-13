//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <boost/algorithm/string.hpp>

#include "entities/AuraManager.hpp"
#include "entities/roles/driver/models/VehicleLoadingModel.hpp"
#include "util/Utils.hpp"

using namespace std;
using namespace sim_mob;

VehicleLoadingModel::VehicleLoadingModel(DriverUpdateParams &params)
{
	modelName = "general_driver_model";
	splitDelimiter = " ,";

	readDriverParameters(params);
}

VehicleLoadingModel::~VehicleLoadingModel()
{
}

void VehicleLoadingModel::readDriverParameters(DriverUpdateParams &params)
{
	bool isAMOD = false;

	if (params.driver->getParent()->amodId != "-1")
	{
		isAMOD = true;
	}

	string speedAssignmentThresholdsStr;
	ParameterManager *parameterMgr = ParameterManager::Instance(isAMOD);
	parameterMgr->param(modelName, "Initial_Speed_Assignment_Thresholds", speedAssignmentThresholdsStr,
	                    string("4 14 40"));
	createInitialSpeedAssignmentThresholds(speedAssignmentThresholdsStr, speedAssignmentThresholds);
}

void VehicleLoadingModel::createInitialSpeedAssignmentThresholds(string &strParams,
                                                                 InitialSpeedAssignmentThresholds &thresholds)
{
	std::vector<std::string> arrayStr;
	vector<double> params;

	boost::trim(strParams);
	boost::split(arrayStr, strParams, boost::is_any_of(splitDelimiter), boost::token_compress_on);

	for (int i = 0; i < arrayStr.size(); ++i)
	{
		try
		{
			params.push_back(boost::lexical_cast<double>(arrayStr[i].c_str()));
		}
		catch (boost::bad_lexical_cast &)
		{
			std::stringstream str;
			str << __func__ << ": Cannot convert " << strParams << " to type double";
			throw std::runtime_error(str.str());
		}
	}

	thresholds.distanceBoundedLow = params[0];
	thresholds.distanceBoundedUpper = params[1];
	thresholds.distanceUnbounded = params[2];
}

void VehicleLoadingModel::chooseStartingLaneAndSpeed(vector<WayPoint> &path, int *laneIdx, const int segmentId,
                                                     int *initialSpeed, DriverUpdateParams &params)
{
	//1. Validate whether the given start segment id is actually in the path
	if (segmentId != 0)
	{
		setPathStartSegment(path, segmentId);
	}

	//2. Check if a valid lane index has been specified

	auto currSegment = path.front().roadSegment;
	unsigned int noOfLanes = currSegment->getNoOfLanes();
	std::set<const Lane *> candidateLanes;
	bool isLaneSelected = (*laneIdx >= 0) && (*laneIdx < noOfLanes);
	const Lane *selectedLane = nullptr;

	if (!isLaneSelected)
	{
		//2.1 Short-list lanes that are connected to the next link
		chooseConnectedLanes(path, candidateLanes, params);

		if (candidateLanes.size() == 1)
		{
			//There is only 1 lane that is connected, no point in looking at available space
			selectedLane = *(candidateLanes.begin());
			*laneIdx = selectedLane->getLaneIndex();
			isLaneSelected = true;
		}
	}
	else
	{
		selectedLane = currSegment->getLane(*laneIdx);
		candidateLanes.insert(selectedLane);
	}

	//Map of the candidate lane vs the leader vehicle on that lane
	unordered_map<const Lane *, NearestVehicle> leadVehicles;

	//Map of the candidate lane vs the follower vehicle on that lane
	unordered_map<const Lane *, NearestVehicle> followerVehicles;

	getLeadAndFollowerVehicles(params, candidateLanes, leadVehicles, followerVehicles);

	if (!isLaneSelected)
	{
		//2.2 Choose the lane with the most amount of space at the entry point
		selectedLane = chooseLaneWithMostSpace(candidateLanes, leadVehicles, followerVehicles);
		*laneIdx = selectedLane->getLaneIndex();
	}

	//Set initial speed based on the leader vehicle
	auto leader = leadVehicles[selectedLane];

	if (speedAssignmentThresholds.distanceBoundedLow < leader.distance &&
	    leader.distance < speedAssignmentThresholds.distanceBoundedUpper)
	{
		*initialSpeed = leader.driver->getFwdVelocity();
	}
	else if (speedAssignmentThresholds.distanceBoundedUpper < leader.distance &&
	         leader.distance <= speedAssignmentThresholds.distanceUnbounded)
	{
		auto alpha = (leader.distance - speedAssignmentThresholds.distanceBoundedUpper) /
		             (speedAssignmentThresholds.distanceUnbounded - speedAssignmentThresholds.distanceBoundedUpper);
		*initialSpeed = (alpha * selectedLane->getParentSegment()->getMaxSpeed()) +
		                (1 - alpha) * leader.driver->getFwdVelocity();
	}
	else if (leader.distance > speedAssignmentThresholds.distanceUnbounded)
	{
		*initialSpeed = selectedLane->getParentSegment()->getMaxSpeed();
	}
}

void VehicleLoadingModel::setPathStartSegment(vector<WayPoint> &path, const int segmentId) const
{
	auto it = path.begin();

	//Look for the given start segment in the path
	while (it != path.end())
	{
		if (it->type == WayPoint::ROAD_SEGMENT)
		{
			if (segmentId == it->roadSegment->getRoadSegmentId())
			{
				//Valid segment id, update the path by removing segments before this
				path.erase(path.begin(), it);
				break;
			}
		}

		++it;
	}
}

void VehicleLoadingModel::chooseConnectedLanes(vector<WayPoint> &path, set<const Lane *> &connectedLanes,
                                               DriverUpdateParams &params)
{
	//The first link in the path
	auto *firstLink = path.front().roadSegment->getParentLink();
	
	//Check if we have stop points defined in the first link. As stop points will require vehicles to be
	//on the slowest lane, we must consider them while checking lane connectivity
	auto segmentsInLink = firstLink->getRoadSegments();
	for(auto segment : segmentsInLink)
	{
		auto itStopPt = params.stopPointPool.find(segment->getRoadSegmentId());

		if(itStopPt != params.stopPointPool.end())
		{
			//Stop point has been defined, so we can simply select the slowest lane - as it will be the only
			//lane connected to the stop point - and return
			connectedLanes.insert(path.front().roadSegment->getLane(0));
			return;
		}
	}

	//The first turning group in the path
	const TurningGroup *tGroup = nullptr;

	//The first turning group in the path will be after all the segments in the first link
	auto itPath = path.begin() + segmentsInLink.size();

	if(itPath != path.end())
	{
		tGroup = itPath->turningGroup;
		const std::map<unsigned int, std::map<unsigned int, TurningPath *> > &tPaths = tGroup->getTurningPaths();

		if (!tPaths.empty())
		{
			//Set all lanes that are connected to the turning paths as candidate lanes
			//tPaths is a map of <from lane, <to lane, turning path> >, we can use the from lane
			//as the candidate lanes
			for (const auto &tPath : tPaths)
			{
				//Add the lane of the from lane
				connectedLanes.insert(tPath.second.begin()->second->getFromLane());
			}
		}
		else
		{
			std::stringstream msg;
			msg << __func__ << ": No turning paths in the turning group " << tGroup->getTurningGroupId();
			throw runtime_error(msg.str());
		}
	}
	else
	{
		//Path contains only a single link. Set all lanes as candidate lanes
		auto lanes = path.front().roadSegment->getLanes();
		connectedLanes.insert(lanes.begin(), lanes.end());
	}
}

void VehicleLoadingModel::getLeadAndFollowerVehicles(DriverUpdateParams &params, set<const Lane *> &candidateLanes,
                                                     unordered_map<const Lane *, NearestVehicle> &leadVehicles,
                                                     unordered_map<const Lane *, NearestVehicle> &followerVehicles)
{
	//Initialise the lead and follower vehicles maps with defaults
	for (auto lane : candidateLanes)
	{
		leadVehicles[lane] = NearestVehicle();
		followerVehicles[lane] = NearestVehicle();
	}

	//Select any candidate lane as a reference to search near
	WayPoint wayPt = WayPoint(*(candidateLanes.begin()));
	const RoadSegment *segment = wayPt.lane->getParentSegment();
	const Link *link = segment->getParentLink();

	//Get the nearby agents from the aura manager
	auto nearbyAgents = AuraManager::instance().nearbyAgents(segment->getPolyLine()->getFirstPoint(), wayPt,
	                                                         distanceInFront, distanceBehind, nullptr);

	//Select the agents that are the lead and follower vehicles in each of the candidate lanes
	for (auto agent : nearbyAgents)
	{
		//Perform no action on non-Persons or ourself
		auto *nearbyPerson = dynamic_cast<const Person_ST *> (agent);

		if (!nearbyPerson || nearbyPerson == params.driver->getParent())
		{
			continue;
		}

		auto nearbyAgentRole = nearbyPerson->getRole();

		if (!nearbyAgentRole)
		{
			continue;
		}

		auto nearbyDriver = dynamic_cast<const Driver *>(nearbyAgentRole);

		if (!nearbyDriver || nearbyDriver->IsVehicleInLoadingQueue())
		{
			continue;
		}

		//At this stage we would have only persons which have some type of driver role

		//If the other driver is on a lane, it can be in front of us or behind us (before the intersection)
		//If the other driver is on a turning path, it is behind us
		if (nearbyDriver->IsInIntersection())
		{
			//Vehicle is in intersection. Get the turning path it is on and the get how must distance it must cover
			//to reach the our starting point
			auto turningPath = nearbyDriver->getCurrTurningPath();
			auto destinationLane = turningPath->getToLane();

			//If the destination lane is not one of the candidate lanes, we can ignore this vehicle
			if (candidateLanes.find(destinationLane) == candidateLanes.end())
			{
				continue;
			}

			auto distanceToEndOfTurning = turningPath->getLength() - nearbyDriver->getDistCoveredOnCurrWayPt();
			auto nearestVehicle = followerVehicles[destinationLane];

			if (distanceToEndOfTurning < nearestVehicle.distance)
			{
				nearestVehicle.driver = nearbyDriver;
				nearestVehicle.distance = distanceToEndOfTurning;
			}
		}
		else
		{
			//Vehicle is on a lane.
			//It is ahead of us if it is on a lane that belongs to the same segment or link we are trying to enter

			auto nearbyVehCurrLane = nearbyDriver->getCurrLane();
			auto nearbyVehCurrSeg = nearbyVehCurrLane->getParentSegment();
			auto nearbyVehCurrLink = nearbyVehCurrSeg->getParentLink();

			if (link == nearbyVehCurrLink)
			{
				if (segment == nearbyVehCurrSeg)
				{
					//If the lane is not one of the candidate lanes, we can ignore this vehicle
					if (candidateLanes.find(nearbyVehCurrLane) == candidateLanes.end())
					{
						continue;
					}

					auto distanceAhead = nearbyDriver->getDistCoveredOnCurrWayPt();
					auto nearestVehicle = leadVehicles[nearbyVehCurrLane];

					if (distanceAhead < nearestVehicle.distance)
					{
						nearestVehicle.driver = nearbyDriver;
						nearestVehicle.distance = distanceAhead;
					}
				}
				else if (segment->getSequenceNumber() == nearbyVehCurrSeg->getSequenceNumber() - 1)
				{
					//The vehicle is in the next segment. For each of the candidate lanes, we check if they are
					//connected to the nearby vehicle's lane.
					for (auto lane : candidateLanes)
					{
						for (auto connector : lane->getLaneConnectors())
						{
							if (connector->getToLane() != nearbyVehCurrLane)
							{
								continue;
							}

							auto distanceAhead = lane->getLength() + nearbyDriver->getDistCoveredOnCurrWayPt();
							auto nearestVehicle = leadVehicles[lane];

							if (distanceAhead < nearestVehicle.distance)
							{
								nearestVehicle.driver = nearbyDriver;
								nearestVehicle.distance = distanceAhead;
							}
						}
					}
				}
			}
			else
			{
				//Vehicle is on a lane that is on another link. It is behind us only if this link is connected to
				//the link we are trying to enter (we may wrongly consider vehicles that would take a turn and go on a
				//different link though, but that should be alright)
				auto node = segment->getParentLink()->getFromNode();
				auto tGroup = node->getTurningGroup(nearbyVehCurrLink->getLinkId(), link->getLinkId());

				if (tGroup == nullptr)
				{
					continue;
				}

				//The nearby vehicle is on a link connected to our link. Get the turning path that connects the lane of
				//the nearby vehicle to the candidate lanes
				for (auto lane : candidateLanes)
				{
					auto tPath = tGroup->getTurningPath(nearbyVehCurrLane->getLaneId(), lane->getLaneId());

					if (tPath == nullptr)
					{
						continue;
					}

					//The lanes are connected by a turning path. Distance between the vehicles is length of turning
					//plus the distance to intersection
					auto distanceBehind = nearbyDriver->getDistToIntersection() + tPath->getLength();
					auto nearestVehicle = followerVehicles[lane];

					if (distanceBehind < nearestVehicle.distance)
					{
						nearestVehicle.driver = nearbyDriver;
						nearestVehicle.distance = distanceBehind;
					}
				}
			}
		}
	}
}

const Lane *VehicleLoadingModel::chooseLaneWithMostSpace(const set<const Lane *> &connectedLanes,
                                                         unordered_map<const Lane *, NearestVehicle> &leadVehicles,
                                                         unordered_map<const Lane *, NearestVehicle> &followerVehicles)
{
	double maxSpace = 0;
	const Lane *selectedLane = nullptr;

	for (auto lane : connectedLanes)
	{
		auto leader = leadVehicles[lane];
		auto follower = followerVehicles[lane];
		auto space = (0.7 * leader.distance) + (0.3 * follower.distance);

		if (space > maxSpace)
		{
			maxSpace = space;
			selectedLane = lane;
		}
	}

	return selectedLane;
}