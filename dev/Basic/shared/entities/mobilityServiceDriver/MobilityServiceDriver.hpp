/*
 * MobilityServiceDriver.hpp
 *
 *  Created on: May 2, 2017
 *      Author: zhang huai peng
 */

#ifndef MOBILITYSERVICEDRIVER_HPP_
#define MOBILITYSERVICEDRIVER_HPP_
namespace sim_mob {
class Node;
class MobilityServiceDriver {
public:
	enum ServiceStatus
	{
		SERVICE_UNKNOWN = 0,
		SERVICE_FREE
	};
	MobilityServiceDriver(){};
	virtual ~MobilityServiceDriver(){};
	/**
	 * the interface function to get current driver status
	 * @return current status
	 */
	virtual ServiceStatus getServiceStatus() = 0;
	/**
	 * the interface function to get current node
	 * @return current node.
	 */
	virtual const Node* getCurrentNode() = 0;
};

} /* namespace sim_mob */

#endif /* MOBILITYSERVICEDRIVER_HPP_ */
