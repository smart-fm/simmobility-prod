/*
 * FrazzoliController.h
 *
 *  Created on: 21 Jun 2017
 *      Author: araldo
 */

#ifndef SHARED_ENTITIES_CONTROLLERS_FRAZZOLICONTROLLER_HPP_
#define SHARED_ENTITIES_CONTROLLERS_FRAZZOLICONTROLLER_HPP_

#include "OnCallController.hpp"

namespace sim_mob {

/**
 * This edge exists in the RV graph if the two requests can share the same ride
 */
//typedef std::pair<TripRequestMessage, Person*> RR_Edge; Useless

/**
 * This edge exists in the RV graph if the driver can serve the request
 * (D stands for Driver)
 */
typedef std::pair<TripRequestMessage, const Person*> RD_Edge;

/**
 * This edge exists in the RTV graph if the request belongs to the request group
 * (G stands for request Group )
 */
typedef std::pair<TripRequestMessage, Group<TripRequestMessage>> RG_Edge;

/**
 * This edge exists in the RTV graph if the request request group can be served by the driver
 * (G stands for request Group, D stands for Driver )
 */
class GD_Edge
{
public:
	GD_Edge(const Group<TripRequestMessage>& requestGroup_, const Person* driver_, double cost_,
			const Schedule& schedule):
		requestGroup(requestGroup_), driver(driver_),cost(cost_){};

	GD_Edge(const GD_Edge& other):requestGroup(other.getRequestGroup()),driver(other.getDriver()),
			cost(other.getCost()), schedule(other.getSchedule() ){};

	bool operator<(const GD_Edge& other) const;

	Group<TripRequestMessage> getRequestGroup() const;
	const Person* getDriver() const;
	double getCost() const;
	Schedule getSchedule() const;

protected:
	Group<TripRequestMessage> requestGroup;
	const Person* driver;
	double cost;
	Schedule schedule;
};

/**
 * In [Frazzoli2017] this graph is called RV graph
 */
class RD_Graph
{
public:
	virtual ~RD_Graph(){};
	/**
	 * Returns the Request-to-Driver edges having that specific driver
	 */
	const std::vector< RD_Edge> getRD_Edges(const Person* driver) const;

	virtual void addEdge(const TripRequestMessage& r1, const TripRequestMessage& r2) ;
	virtual void addEdge(TripRequestMessage request, const Person* mobilityServiceDriver);
	virtual bool doesEdgeExist(const TripRequestMessage& r1, const TripRequestMessage& r2) const;
	virtual const std::string getProperties() const;

protected:

	/**
	 * Request-to-request association. If a request is associated to another, it is like we
	 * are drawing an edge between them
	 */
	std::map< TripRequestMessage, std::set<TripRequestMessage > > rrEdges;

	/**
	 * Request-to-vehicle association
	 */
	std::map<const Person*, std::vector<RD_Edge>> rdEdgeMap;

};

/**
 * In [Frazzoli2017] this graph is callerd RTV graph
 */
class RGD_Graph
{
public:
#ifndef NDEBUG
	RGD_Graph():isGdEdgesSorted(false){};
#endif

	virtual void addEdge(const TripRequestMessage& request, const Group<TripRequestMessage>& requestGroup);
	virtual void addEdge(const Group<TripRequestMessage>& requestGroup, const Person* mobilityServiceDriver,
			double cost, const Schedule& schedule);

	/**
	 * Sort the edges from the most costly to the least
	 */
	virtual void sortGD_Edges();

	virtual GD_Edge popGD_Edge();
	virtual bool hasGD_Edges() const;
	void consistencyChecks() const;


protected:

	/**
	 * RequestGroup-to-Driver association
	 */
	std::vector<GD_Edge> gdEdges;

#ifndef NDEBUG
	/**
	 * Request-to-RequestGroup association
	 */
	sim_mob::Group<RG_Edge> rgEdges;

	bool isGdEdgesSorted;
#endif
};

class FrazzoliController: public OnCallController {
public:
	FrazzoliController
		(const MutexStrategy& mtxStrat, unsigned int computationPeriod, unsigned id) :
		OnCallController(mtxStrat, computationPeriod, MobilityServiceControllerType::SERVICE_CONTROLLER_FRAZZOLI, id),
		requestGroupsPerOccupancy(std::vector< Group< Group<TripRequestMessage> > >(maxVehicleOccupancy) )
	{
	}




protected:
	/**
	 * This mimicks Appendix II.C of [Frazzoli2017]
	 */
	virtual RD_Graph generateRD_Graph();

	/**
	 * Performs the controller algorithm to assign vehicles to requests. This mimicks Alg.1 of Appendix [Frazzoli2017]. In the paper, the RGD graph is called RTV.
	 * [Frazzoli2017] Alonso-mora, J., Samaranayake, S., Wallar, A., Frazzoli, E., & Rus, D. (2017). On-demand high-capacity ride-sharing via dynamic trip-vehicle assignment - Supplemental Material. Proceedings of the National Academy of Sciences of the United States of America, 114(3).
	 */
	virtual RGD_Graph generateRGD_Graph(const RD_Graph& rdGraph);

	/**
	 * This mimicks Algorithm 2 of Suppl.Material of [Frazzoli2017]
	 */
	virtual void greedyAssignment(RD_Graph& rdGraph, RGD_Graph& rgdGraph);

	/**
	 * Overrides the parent function
	 */
	virtual void computeSchedules();

	// requestGroupsPerOccupancy[i] will contain all the request groups of i+1 requests
	std::vector< Group< Group<TripRequestMessage> > > requestGroupsPerOccupancy;




};

} /* namespace sim_mob */

std::ostream& operator<<(std::ostream& strm, const sim_mob::RG_Edge& rgEdge);

#endif /* SHARED_ENTITIES_CONTROLLERS_FRAZZOLICONTROLLER_HPP_ */


