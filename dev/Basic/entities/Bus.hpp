#pragma once

#include <vector>
#include "Agent.hpp"
#include "BusStop.hpp"
#include "Route.hpp"
#include "Node.hpp"
#include "Lane.hpp"
#include "Length.hpp"
#include <vector>
#include "metrics/Length.hpp"
#include "util/DynamicVector.hpp"
#include "constants.h"

namespace sim_mob{

class Node;
class Lane;
class BusStop;
class Route;

class Bus : public sim_mob::Vehicle {

public:
	Bus(unsigned int id=-1) : Agent(id);
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

}
}