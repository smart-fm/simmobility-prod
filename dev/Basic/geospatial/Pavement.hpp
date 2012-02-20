/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <vector>

#include "Traversable.hpp"
#include "Point2D.hpp"
#include "metrics/Length.hpp"


namespace sim_mob
{

//Forward declarations
class RoadItem;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif


/**
 * Helper class that represents a RoadItem at a given offset.
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 */
struct RoadItemAndOffsetPair
{
	RoadItemAndOffsetPair(const sim_mob::RoadItem* item, centimeter_t offset) : item(item), offset(offset) {}

	///The next RoadItem
    const sim_mob::RoadItem* item;

    /// The offset from the Pavement::start where \c item is located.
    centimeter_t offset;
};



/**
 * Represents a traversable area that may contain obstacles and have a complex geometry.
 * Intended for things like Roadways (car, bike, pedestrian) and (eventually) the interiors
 * of parking structures.
 */
class Pavement : public sim_mob::Traversable {
public:
	Pavement() : Traversable() {}

	///The length of this Pavement.
	///
	/// \note
	/// The length of a Pavement object is NOT the Euclidean distance between its start
	/// and end nodes, but rather the total length of the polyline. An Agent's position
	/// along a Pavement object is alyways given with respect to that Pavement's length,
	/// not to the Euclidean distance between start and end.
	centimeter_t length;


	///The total width of this Pavement. If the width is zero, then it is assumed that
	///  one can determine the width through some other means (e.g., this is a RoadSegment
	///  where the Lanes have their own individual widths) or that the width is irrelevant.
	mutable centimeter_t width; //NOTE: This shouldn't be mutable, but for now I'm trying to get it to work...


	///The polyline of this Pavement. In the case of RoadSegments, the polyline is assumed to
	///   go down the middle of the median, from start to end, including the start and end points.
	///The polyline must trace the median so that bidirectional roads' polylines line up with
	///   single-directional ones even when zoomed out.
	///The start and end points must be included so that lane polylines are consistent with the
	///   median polyline.
	std::vector<sim_mob::Point2D> polyline;

	///The obstacles on this Pavement. Obstacles generally include things like RoadBumps and possibly
	///   bus stops (still under discussion), and are stored in a map with their position down the
	///   road segment as the key.
	///
	/// \note
	/// Currently, there can be only one obstacle at any given point on the Pavement. We may have to
	/// revisit this problem if length is represented as an integer, but if length remains represented
	/// as a double then we can simply inch the obstacle slightly further down the road.
	std::map<centimeter_t, const RoadItem*> obstacles;

	///Return the next obstacle from a given point on this Pavement.
	sim_mob::RoadItemAndOffsetPair nextObstacle(const sim_mob::Point2D& pos, bool isForward) const;

	///Return the next obstacle from a given offset along the current Pavement.
	sim_mob::RoadItemAndOffsetPair nextObstacle(centimeter_t offset, bool isForward) const;

	///Helper method: build a polyline given a bulge and a center. Segments are generated
	///   of length segmentLength.
	///
	/// \todo segmentLength might be better represented as something like "maxAngle", which
	///       would allow for long non-curving segments and short curving ones.
	static void GeneratePolyline(Pavement* p, Point2D center, double bulge, int segmentLength);

#ifndef SIMMOB_DISABLE_MPI
	///The identification of Crossing is packed using PackageUtils;
	static void pack(PackageUtils& package, Pavement* one_pavement);

	///UnPackageUtils use the identification of Crossing to find the Crossing Object
	static const Pavement* unpack(UnPackageUtils& unpackage);
#endif
};





}
