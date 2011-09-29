/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include "Node.hpp"
#include "Lane.hpp"

namespace sim_mob
{

/**
 * A node where multiple Links join. By themselves, Nodes provide all of the information
 * required to diagram which Lanes lead to other lanes at a given roundabout. The
 * Roundabout class, however, adds a much-needed layer of context by specifying exactly
 * how the roundabout actually looks and behaves.
 *
 * \todo
 * The current image doesn't use the same terminology as the code. Also, I think we can
 * provide a slightly clearer example (e.g., color-coded segments?).
 *
 * \todo
 * See Intersection's note on having multiple parallel data structures.
 *
 * \image html roundabout.jpg
 */
class Roundabout : public sim_mob::MultiNode {
public:
	Roundabout() : MultiNode() {}

protected:
	///List of which RoadSegments approaching a roundabout have a separator in the median.
	std::vector<bool> separator;

	///The length of each chunk. As each RoadSegment approaches a Roundabout, a "chunk" of
	///   this distance is taken from the end of that RoadSegment and is used to model details
	///   specific to this intersection. A chunkLength of zero means that no special processing
	///   is done for that RoadSegment at this Intersection.
	std::vector<float> chunkLengths;

	///Vector of offsets
	///
	/// \note
	/// I'm not clear on how offsets differ from chunks. ~Seth
	std::vector<float> offsets;

	///Angle of this RoadSegment's entrance into the Roundabout.
	///
	/// \note
	/// What is this angle measured in reference to? ~Seth
	std::vector<float> entranceAngle;

	///Number of additional lanes (for each RoadSegment) which are added to the
	///  dominant ("driving") side to allow for greater throughput on the Roundabout.
	///Any additional lanes will end as soon as they reach the Roundabout, at which point
	//   roundaboutNumberOfLanes remain.
	std::vector<int> addDominantLane;

	///Islands....
	///
	/// \note
	/// Note sure how this is specified, esp. as a float. ~Seth
	float roundaboutDominantIslands;

	///Numer of lanes within the roundabout.
	int roundaboutNumberOfLanes;

	///Vector of obstacles.
	///
	/// \note
	/// What's this for? Yield signs? ~Seth
	std::vector<RoadItem*> obstacles;
};





}
