#pragma once

#include "path/Reroute.hpp"

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
	class MesoReroute : public sim_mob::Reroute
	{
		///	the movement object this class is working for
		sim_mob::medium::DriverMovement &dm;
		///	current Segment where the Agent is or has just completed
		const sim_mob::RoadSegment* currSegment;

		///	 the next segment along the user's path
		const sim_mob::RoadSegment* nextSegment;

	public:
		/**
		 *override from base class
		 */
		void preReroute();

		/**
		 *override from base class
		 */
		void postReroute();

		MesoReroute(sim_mob::medium::DriverMovement &dm/*, const sim_mob::RoadSegment* currSegment, const sim_mob::RoadSegment* nextSegment*/);
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
