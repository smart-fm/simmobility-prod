/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include "MultiNode.hpp"
#include "Lane.hpp"

namespace sim_mob
{


namespace aimsun
{
//Forward declarations
class Loader;
}



/**
 * A node where multiple Links join. By themselves, Nodes provide all of the information
 * required to diagram which Lanes lead to other lanes at a given intersection. The
 * Intersection class, however, adds a much-needed layer of context by specifying exactly
 * how the intersection actually looks and behaves.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 *
 * \todo
 * The current image doesn't use the same terminology as the code. Also, I think we can
 * provide a slightly clearer example (e.g., a T-intersection is probably sufficient).
 *
 * \todo
 * Currently, we have several parallel vectors, where each entry is relevant to a RoadSegment
 * at that intersection. It might be safer to group these elements (separator, etc.) into
 * a new class and then have a single vector of items of this class.
 *
 * \image html intersection.jpg
 *
 */
class Intersection : public sim_mob::MultiNode {
public:
	explicit Intersection(int x=0, int y=0) : MultiNode(x, y) {}

protected:
	///The length of each chunk. As each RoadSegment approaches an Intersection, a "chunk" of
	///   this distance is taken from the end of that RoadSegment and is used to model details
	///   specific to this intersection. A chunkLength of zero means that no special processing
	///   is done for that RoadSegment at this Intersection.
	std::vector<int> chunkLengths;

	///List of which RoadSegments approaching an intersection have a separator in the median.
	///
	/// \note
	/// This was originally a float; I changed it to a bool. ~Seth
	std::vector<bool> separator;

	///Vector of offsets
	///
	/// \note
	/// I'm not clear on how offsets differ from chunks. ~Seth
	std::vector<int> offsets;

	///Vector of additional lanes on the dominant ("driving") side.
	/// These lanes begin at a distance of chunkLength from the Intersection.
	std::vector<sim_mob::Lane*> additionalDominantLanes;

	///Vector of additional lanes on the sub-dominant (opposite of "driving") side.
	/// These lanes begin at a distance of chunkLength from the Intersection.
	std::vector<sim_mob::Lane*> additionalSubdominantLanes;

	///Vector of flags; if true, then there is an island on the dominant ("driving") side
	std::vector<bool> dominantIslands;

	///Vector of obstacles.
	///
	/// \note
	/// What's this for? Traffic lights? ~Seth
	std::vector<RoadItem*> obstacles;

friend class aimsun::Loader;

};





}
