//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>

#include "PT_Stop.hpp"
#include "NetworkLoader.hpp"
#include "SurveillanceStation.hpp"
#include "TaxiStand.hpp"

namespace sim_mob
{

class Link;
class NetworkLoader;
class Node;
class ParkingArea;
class ParkingSlot;
class Point;
class PolyPoint;
class PolyLine;
class TurningGroup;
class TurningPath;

/**
 * class for holding the network for simulation
 * \author Neeraj D
 * \author Harish L
 */
class RoadNetwork
{
private:
	friend NetworkLoader;

	/**Points to the singleton instance of the road network*/
	static RoadNetwork *roadNetwork;

	/**This map stores all the links in the network, with the Link id as the key for retrieval*/
	std::map<unsigned int, Link *> mapOfIdVsLinks;

	/**This map stores all the nodes in the network, with the Node id as the key for retrieval*/
	std::map<unsigned int, Node *> mapOfIdvsNodes;

	/**This map stores all the lanes in the network, with the lane id as the key for retrieval*/
	std::map<unsigned int, Lane *> mapOfIdVsLanes;

	/**This map stores all the segments in the network, with the segment id as the key for retrieval*/
	std::map<unsigned int, RoadSegment *> mapOfIdVsRoadSegments;

	/**This map stores all the turning conflicts in the network, with the conflict id as the key for retrieval*/
	std::map<unsigned int, TurningConflict *> mapOfIdVsTurningConflicts;

	/**This map stores all the turning groups in the network, with the group id as the key for retrieval*/
	std::map<unsigned int, TurningGroup *> mapOfIdvsTurningGroups;

	/**This map stores all the turning paths in the network, with the turning id as the key for retrieval*/
	std::map<unsigned int, TurningPath *> mapOfIdvsTurningPaths;

	/**This map stores all the bus stops in the network with bus stop id as the key*/
	std::map<unsigned int, BusStop *> mapOfIdvsBusStops;

	/**This map stores all the taxi stands in the network with taxi-stand id as the key*/
	std::map<unsigned int, TaxiStand *> mapOfIdvsTaxiStands;

	/**This map stores all the parking slots in the network with the id as the key*/
	std::map<unsigned int, ParkingSlot *> mapOfIdVsParkingSlots;

	/**This map stores all the parking areas in the network with the id as the key*/
	std::map<unsigned int, ParkingArea *> mapOfIdVsParkingAreas;

	/**Private constructor as the class is a singleton*/
	RoadNetwork();

	/**
	 * Returns a non-constant pointer to RoadNetwork
	 * This function is private because only the NetworkLoader (a friend of this class) must gain access to this writable instance.
	 *
	 * @return a non-constant pointer to RoadNetwork
	 */
	static RoadNetwork* getWritableInstance();

public:
	virtual ~RoadNetwork();

	/**Returns pointer to the singleton instance of RoadNetwork*/
	static const RoadNetwork* getInstance();

	const std::map<unsigned int, Link *>& getMapOfIdVsLinks() const;

	const std::map<unsigned int, Node *>& getMapOfIdvsNodes() const;

	const std::map<unsigned int, Lane *>& getMapOfIdVsLanes() const;

	const std::map<unsigned int, RoadSegment*>& getMapOfIdVsRoadSegments() const;

	const std::map<unsigned int, TurningGroup *>& getMapOfIdvsTurningGroups() const;

	const std::map<unsigned int, TurningPath *>& getMapOfIdvsTurningPaths() const;

	const std::map<unsigned int, TurningConflict *>& getMapOfIdvsTurningConflicts() const;

	const std::map<unsigned int, BusStop *>& getMapOfIdvsBusStops() const;

	const std::map<unsigned int, TaxiStand *>& getMapOfIdvsTaxiStands() const;

	const std::map<unsigned int, ParkingSlot *>& getMapOfIdVsParkingSlots() const;

	const std::map<unsigned int, ParkingArea *>& getMapOfIdVsParkingAreas() const;
	
	/**
	 * Adds a lane to the road network
	 * @param lane - the lane to be added
	 */
	void addLane(Lane *lane);

	/**
	 * Adds a lane connector to the road network
	 * @param connector - the lane connector to be added
	 */
	void addLaneConnector(LaneConnector *connector);

	/**
	 * Adds a lane poly-line to the road network
	 * @param point - the poly-point to be added to the poly-line
	 */
	void addLanePolyLine(PolyPoint point);

	/**
	 * Adds a link to the road network
	 * @param link - the link to be added
	 */
	void addLink(Link *link);

	/**
	 * Adds a node to the road network
	 * @param node - the node to be added
	 */
	void addNode(Node *node);

	/**
	 * Adds a road segment to the road network
	 * @param segment - the road segment to be added
	 */
	void addRoadSegment(RoadSegment *segment);

	/**
	 * Adds a segment poly-line to the road network
	 * @param point - the poly-point to be added to the poly-line
	 */
	void addSegmentPolyLine(PolyPoint point);

	/**
	 * Adds a turning conflict to the road network
	 * @param turningConflict - the conflict to be added
	 */
	void addTurningConflict(TurningConflict *turningConflict);

	/**
	 * Adds a turning group to the road network
	 * @param turningGroup - the turning group to be added
	 */
	void addTurningGroup(TurningGroup *turningGroup);

	/**
	 * Adds a turning path to the road network
	 * @param turningPath - the turning path to be added
	 */
	void addTurningPath(TurningPath *turningPath);

	/**
	 * Adds a turning poly-line to the road network
	 * @param point - the poly-point to be added to the poly-line
	 */
	void addTurningPolyLine(PolyPoint point);

	/**
	 * Adds a bus stop to the road network
	 * @param stop - the pointer to bus stop
	 */
	void addBusStop(BusStop *stop);

	/**
	 * Adds a taxi stand to the road network
	 * @param stand - the pointer to taxi-stand
	 */
	void addTaxiStand(TaxiStand* stand);
	
	/**
	 * Adds a parking slot to the the respective parking area and the road network
	 * @param parkingSlot - the pointer to the parking slot
	 */
	 void addParking(ParkingSlot *parkingSlot);
	 
	 /**
	 * Adds a surveillance stn to the network
	 * @param station - the pointer to the surveillance station
	 */
	void addSurveillenceStn(SurveillanceStation *station);

	/**
	 * Template class to lookup any map with an unsigned int id
	 *
	 * @param lookup the map to look-up id
	 * @param id the id to look-up in map
	 * @return value mapped to id in map, if found; NULL otherwise
	 */
	template<class T>
	const T* getById(const std::map<unsigned int, T*>& lookup, unsigned int id) const
	{
		typename std::map<unsigned int, T*>::const_iterator lookupIt = lookup.find(id);

		if (lookupIt != lookup.end())
		{
			return (lookupIt->second);
		}
		else
		{
			return nullptr;
		}
	}
	
	/**
	 * Locates the node that is closest to the given position
	 * 
	 * @param position the location closest to which the node is to be found
	 * @return nearest node
	 */
	Node* locateNearestNode(const Point& position) const;
};
}

