/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "GenConfig.h"
#include "entities/UpdateParams.hpp"
#include "geospatial/Lane.hpp"
#include "util/DynamicVector.hpp"
#include <boost/random.hpp>
#include "util/LangHelpers.hpp"
#ifdef SIMMOB_NEW_SIGNAL
	#include "entities/signal/Signal.hpp"
#else
	#include "entities/Signal.hpp"
#endif

namespace sim_mob
{

//Forward declarations
class Lane;

namespace medium
{
class Driver;

///Simple struct to hold parameters which only exist for a single update tick.
/// \author Wang Xinyuan
/// \author Li Zhemin
/// \author Seth N. Hetu
///NOTE: Constructor is currently implemented in Driver.cpp. Feel free to shuffle this around if you like.
struct DriverUpdateParams : public UpdateParams {
	explicit DriverUpdateParams(boost::mt19937& gen) : UpdateParams(gen) ,nextLaneIndex(0){}

	virtual void reset(frame_t frameNumber, unsigned int currTimeMS, const Driver& owner);

	const Lane* currLane;  //TODO: This should really be tied to PolyLineMover, but for now it's not important.
	size_t currLaneIndex; //Cache of currLane's index.
	size_t nextLaneIndex; //for lane changing model

	double currLaneOffset;
	double currLaneLength;
	double elapsedSeconds;
	double timeThisTick;

	DriverUpdateParams& operator=(DriverUpdateParams rhs)
	{
		currLane = rhs.currLane;
		currLaneIndex = rhs.currLaneIndex;
		nextLaneIndex = rhs.nextLaneIndex;

		return *this;
	}

	//Handles state information
	bool justChangedToNewSegment;
	DPoint TEMP_lastKnownPolypoint;
	bool justMovedIntoIntersection;
	double overflowIntoIntersection;
};


}}
