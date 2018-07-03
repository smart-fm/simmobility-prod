//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include "conf/settings/DisableMPI.h"
#include "util/LangHelpers.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "logging/Log.hpp"
#include "logging/NullableOutputStream.hpp"
#include "message/Message.hpp"
#include "message/MessageHandler.hpp"
#include "path/Reroute.hpp"

namespace sim_mob
{

class Vehicle;
class Person;
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
class Driver;
class Pedestrian;
class Agent;
struct TravelMetric;
///used to initialize message handler id of all facets
#define FACET_MSG_HDLR_ID 1000

/**
 * A Facet is a subdivision of a Role. The Facet class contains shared functionality for each type of Facet;
 *  at the moment we only have Behavior and Movement facet subclasses. The Role class just serves as a
 *  container for these two classes. Each subclass of role (Driver, Pedestrian, Passenger, ActivityRole etc.)
 *  must contain pointers/references to respective specializations of the BehaviorFacet and MovementFacet classes.
 *
 * \note
 * Make sure that your subclasses call their parent constructors (or the parent Agent won't be set). Also
 * make sure your subclasses have virtual destructors (less essential, but allows destructor chaining).
 *
 * \author Harish Loganathan
 * \author Seth N. Hetu
 */


class Facet
{
public:

	explicit Facet()
	{
	}

	virtual ~Facet()
	{
	}
	///role facets need id if they register for message handlers
	static unsigned int msgHandlerId;

	///Called the first time an Agent's update() method is successfully called.
	/// This will be the tick of its startTime, rounded down(?).
	virtual void frame_init() = 0;

	///Perform each frame's update tick for this Agent.
	virtual void frame_tick() = 0;

	///Generate output for this frame's tick for this Agent.
	virtual std::string frame_tick_output() = 0;

	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void handleMessage(messaging::Message::MessageType type, const messaging::Message& message);
};

/**
 * See: Facet
 *
 * \author Harish Loganathan
 */
class BehaviorFacet : public Facet
{
public:

	explicit BehaviorFacet() : Facet()
	{
	}

	virtual ~BehaviorFacet()
	{
	}

	///NOTE: There is no resource defined in the base class BehaviorFacet. For role facets of drivers, the vehicle of the parent Role could be
	///      shared between behavior and movement facets. This getter must be overridden in the derived classes to return appropriate resource.
	virtual Vehicle* getResource()
	{
		return nullptr;
	}


public:
	friend class sim_mob::PartitionManager;

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil) = 0;
	virtual void unpack(UnPackageUtils& unpackageUtil) = 0;

	virtual void packProxy(PackageUtils& packageUtil) = 0;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) = 0;
#endif
};

/**
 * See: Facet
 *
 * \author Harish Loganathan
 */
class MovementFacet : public Facet
{
public:
	explicit MovementFacet();
	virtual ~MovementFacet();

	virtual void init()
	{
	}

	virtual bool updateNearbyAgent(const sim_mob::Agent* agent, const sim_mob::Driver* other_driver)
	{
		return false;
	};

	virtual void updateNearbyAgent(const sim_mob::Agent* agent, const sim_mob::Pedestrian* pedestrian)
	{
	};

	///	mark startTimeand origin
	virtual TravelMetric& startTravelTimeMetric() = 0;
	///	mark the destination and end time and travel time
	virtual TravelMetric& finalizeTravelTimeMetric() = 0;
	//needed if the role are reused rather than deleted!

	virtual void resetTravelTimeMetric()
	{
		travelMetric.reset();
	}
	/**
	 * checks if lane is connected to the next segment
	 *
	 * @param lane current lane
	 * @param nxtRdSeg next road segment
	 * @return true if lane is connected to nextSegStats; false otherwise
	 */
	static bool isConnectedToNextSeg(const Lane* lane, const sim_mob::RoadSegment *nxtRdSeg);

	/**
	 * checks if 'any' lane is connected to the next segment
	 *
	 * @param srcRdSeg Road Segment
	 * @param nxtRdSeg next road segment
	 * @return true if lane is connected to next Segment; false otherwise
	 */
	static bool isConnectedToNextSeg(const sim_mob::RoadSegment *srcRdSeg, const sim_mob::RoadSegment *nxtRdSeg);

	virtual TravelMetric& getTravelMetric()
	{
		return travelMetric;
	}



public:
	friend class sim_mob::PartitionManager;
protected:


    ///	placeholder for various movement measurements
	TravelMetric travelMetric;
	/// rerouting member in charge


	boost::shared_ptr<sim_mob::Reroute> rerouter;

	///	list of segments this role has traversed
	std::vector<const sim_mob::RoadSegment*> traversed;

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil) = 0;
	virtual void unpack(UnPackageUtils& unpackageUtil) = 0;

	virtual void packProxy(PackageUtils& packageUtil) = 0;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) = 0;
#endif
};

} // namespace sim_mob

