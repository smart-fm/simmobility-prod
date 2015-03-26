#pragma once

namespace sim_mob
{
//Forward Declaration
class Node;
/**
 *  This class provides base methods for rerouting
 */
class Reroute
{
protected:
	sim_mob::Node * reroutingPoint;
public:
	/**
	 * before starting the reroute
	 */
	virtual void preReroute() = 0;
	/**
	 * called after ending the reroute
	 */
	virtual void postReroute() = 0;
	/**
	 * This Method should Assess the situation
	 * and decieds if a rerouting can take place.
	 * @return true or false
	 */
	virtual bool shouldReroute() = 0;
	/**
	 * The Necessary steps for Needed for rerouting
	 */
	virtual bool reroute() = 0;
	virtual ~Reroute(){}
};
}
