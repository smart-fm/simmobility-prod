//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <stdexcept>
#include <map>

#include "RoadItem.hpp"
#include "RoadSegment.hpp"
#include "Point.hpp"

namespace sim_mob
{

/**Defines the various types of bus terminals*/
enum TerminusType
{
	NOT_A_TERMINUS = 0,
	SOURCE_TERMINUS,
	SINK_TERMINUS
};

class BusStop : public RoadItem
{
private:
	/**Bus stop location*/
	Point location;

	/**The road segment that contains the bus stop*/
	RoadSegment *roadSegment;

	/**Control flag for bus terminus stops*/
	TerminusType terminusType;

	/**How many meters of "bus" can park in the bus lane/bay to pick up pedestrians*/
	double length;

	/**store bus line info at each bus stop for passengers*/
	//std::vector<Busline> busLines;

	/**pointer to the twin bus stop. Contains value if this bus stop is a terminus*/
	const BusStop *twinStop;

	/**indicator to determine whether this stop is virtually created for terminus stops*/
	bool virtualStop;

	/**offset to its parent segment's obstacle list*/
	double offset;

	/**bus stop code*/
	std::string stopCode;

	/**bus stop name*/
	std::string stopName;

	/**bus stop Id*/
	unsigned int stopId;

	/**This map stores all the bus stops in the network with bus stop code as the key*/
	static std::map<std::string, BusStop *> mapOfCodevsBusStops;

public:	
	BusStop();
	virtual ~BusStop();

	const Point& getStopLocation() const;
	void setStopLocation(Point& location);

	const RoadSegment* getParentSegment() const;
	void setParentSegment(RoadSegment *roadSegment);

	const TerminusType& getTerminusType() const;
	void setTerminusType(TerminusType type);

	double getCapacityAsLength() const;
	void setCapacityAsLength(double len);

	//void addBusLine(Busline& line);
	//const std::vector<Busline>& getBusLine() const;

	const BusStop* getTwinStop() const;
	void setTwinStop(const BusStop *stop);

	bool isVirtualStop() const;
	void setVirtualStop(bool val);

	double getOffset() const;
	void setOffset(double val);

	const std::string& getStopName() const;
	void setStopName(const std::string name);

	unsigned int getStopId() const;
	void setStopId(unsigned int id);

	const std::string& getStopCode() const;
	void setStopCode(const std::string& code);

	static void registerBusStop(BusStop *stop);
	static BusStop* findBusStop(const std::string &code);
};

}
