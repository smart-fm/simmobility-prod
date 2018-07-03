#pragma once

#include "path/Reroute.hpp"
#include <set>

namespace sim_mob
{
//Forward Declaration
class RoadSegment;

namespace medium
{
//Forward Declaration
class DriverMovement;
/**
 * this class considers rerouting at each intersection
 */
	class MesoReroute : public Reroute
	{
		///	the movement object this class is working for
		medium::DriverMovement &dm;
		///	current Segment where the Agent is or has just completed
		const RoadSegment* currSegment;

		///	a container to help avoid circular rerouting
		std::set<const RoadSegment*> traversed;

//		///	 the next segment along the user's path
//		const RoadSegment* nextSegment;

	public:
		/**
		 *override from base class
		 */
		void preReroute();

		/**
		 *override from base class
		 */
		void postReroute();

		MesoReroute(medium::DriverMovement &dm/*, const RoadSegment* currSegment, const RoadSegment* nextSegment*/);
		/**
		 *override from base class
		 */
		bool shouldReroute();

		/**
		 *	override from base class
		 */
		bool reroute();

		/**
		 *	Actual rerouting operation in this class
		 */
		bool doReroute();
	};
}
}
