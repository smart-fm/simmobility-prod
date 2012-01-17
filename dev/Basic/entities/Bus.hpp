#pragma once

#include <vector>
#include "Agent.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Route.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/Lane.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "metrics/Length.hpp"
#include "util/DynamicVector.hpp"
#include "GenConfig.h"

namespace sim_mob{

class Node;
class Lane;
class BusStop;
class Route;

class Bus : public sim_mob::Vehicle {

public:
	///NOTE: Currently vehicles need to be constructed with a route. Feel free to modify as needed; it's a relic
	///      from our cleanup of Driver. ~Seth
	explicit Bus(unsigned int id=-1) : Vehicle(std::vector<sim_mob::WayPoint>(), 0), busroute(nullptr), passenger_count_standing(0), passenger_count_sitting(0), passenger_capacity(0) {}

	virtual void update(frame_t frameNumber);
	virtual void buildSubscriptionList();	
protected:
	bool setRoute(const BusRoute* bus_route);
	void updateAtNewStop(const BusStop* prevStop, const BusStop* nextStop);  //sets current bus stop to previous busstop and nextbus stop to current bus stop
	void updateAtNewNode(const Node* prevNode, const Node* newNode);  //same but for nodes
	void updateAtNewLane();
	bool canMove(double proposedX, double proposedY); //checks if it can move to a given location
    void flagBus();
    void flagBus(const BusStop* bus_stop);
	const centimeter_t getLength();

	const BusRoute* busroute;
	const int passenger_count_standing;
	const int passenger_count_sitting;
	const int passenger_capacity;
public:
	double length;				//length of the vehicle
	double width;				//width of the vehicle
	double timeStep;			//time step size of simulation
	int xPos;
	int yPos;
	double xVel;
	double yVel;
	double xAcc;
	double yAcc;
	
	int xPos_;
	int yPos_;
	double xVel_;
	double yVel_;
	double xAcc_;
	double yAcc_;

	DynamicVector heading;
	centimeter_t distAlongHeading;

};

}
