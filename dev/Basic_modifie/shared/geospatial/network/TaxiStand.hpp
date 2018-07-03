/*
 * TaxiStand.hpp
 *
 *  Created on: Oct 24, 2016
 *      Author: zhang huai peng
 */

#ifndef TAXISTAND_HPP_
#define TAXISTAND_HPP_

#include <vector>
#include <stdexcept>
#include <map>

#include "RoadItem.hpp"
#include "RoadSegment.hpp"
#include "spatial_trees/GeneralR_TreeManager.hpp"

namespace sim_mob
{

class TaxiStand : public RoadItem {
public:
	TaxiStand();
	virtual ~TaxiStand();
	/**store all taxi-stands into a global r-tree*/
	static GeneralR_TreeManager<TaxiStand> allTaxiStandMap;
private:
	/**Taxi stand location*/
	Point location;

	/**The road segment that contains the taxi stand*/
	RoadSegment *roadSegment;

	/**How many meters of "taxi" can park in the taxi-stand to pick up pedestrians*/
	double length;

	/**Distance from the start of the parent segment*/
	double offset;

	/**the id of taxi-stand*/
	int id;

public:
	/**set location of taxi stand*/
	void setLocation(const Point& pos);
	/**get current location of taxi stand*/
	const Point& getLocation() const;
	/**set the parent road segment*/
	void setRoadSegment(RoadSegment* segment);
	/**get parent road segment*/
	const RoadSegment* getRoadSegment() const;
	/**set the length of taxi-stand*/
	void setLength(const double len);
	/**get the length of taxi-stand*/
	double getLength() const;
	/**set the offset of taxi-stand*/
	void setOffset(const double offset);
	/**get the offset of taxi-stand*/
	double getOffset() const;
	/**set the id of taxi-stand*/
	void setStandId(int id);
	/**get the id of taxi-stand*/
	int getStandId() const;
	/**get x position*/
	double getPosX() const;
	/**get y position*/
	double getPosY() const;
};

};
#endif /* TAXISTAND_HPP_ */
