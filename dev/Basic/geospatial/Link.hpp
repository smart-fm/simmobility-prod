/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <iostream>

#include "GenConfig.h"

#include "Traversable.hpp"

//TODO: Once the new signal class is stabilized, replace this include with a forward declaration:
#include "entities/signal_transitional.hpp"

namespace sim_mob
{

//Forward declarations
class RoadSegment;
class MultiNode;
class RoadNetworkPackageManager;
class Signal;
class Worker;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

namespace aimsun
{
//Forward declaration
class Loader;
} //End aimsun namespace



/**
 * A road or sidewalk. Generalized movement rules apply for agents inside a link,
 * which is itself composed of segments.
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Xu Yan
 */
class Link : public sim_mob::Traversable {
public:
	Link() : Traversable() {}

	//Initialize a link with the given set of segments
	void initializeLinkSegments(const std::set<sim_mob::RoadSegment*>& segments);

	///Return the length of this Link, which is the sum of all RoadSegments
	/// in the forward (if isForward is true) direction.
	int getLength(bool isForward) const;

	///Return the RoadSegments which make up this Link, in either the forward
	/// (if isForward is true) or reverse direction.
	///
	/// \note
	/// If bidirectional segments are present, this path may include
	/// RoadSegments that should actually be read as end->start, not start->end.
	const std::vector<sim_mob::RoadSegment*>& getPath(bool isForward) const;

	///The name of the particular segment. E.g., "Main Street 01".
	///Useful for debugging by location. May be auto-numbered.
	std::string getSegmentName(const sim_mob::RoadSegment* segment);
	const std::set<sim_mob::RoadSegment*> & getUniqueSegments();

	void extendPolylinesBetweenRoadSegments();
	void extendPolylinesBetweenRoadSegments(std::vector<RoadSegment*>& segments);

	//added methods to access the worker who is managing this link
	Worker* getCurrWorker();
	void setCurrWorker(Worker* w);

#ifndef SIMMOB_DISABLE_MPI
	///The identification of Link is packed using PackageUtils;
	static void pack(sim_mob::PackageUtils& package, const Link* one_link);

	///UnPackageUtils use the identification of Link to find the Link Object
	static const Link* unpack(sim_mob::UnPackageUtils& unpackage);

#endif

public:
	///The road link's name. E.g., "Main Street"
	std::string roadName;
	std::string linkID;

protected:
	//List of pointers to RoadSegments in each direction
	std::vector<sim_mob::RoadSegment*> fwdSegments;
	std::vector<sim_mob::RoadSegment*> revSegments;
	std::set<sim_mob::RoadSegment*> uniqueSegments;

	//who is currently managing this link
	//added by Jenny
	sim_mob::Worker* currWorker;


friend class sim_mob::aimsun::Loader;
friend class sim_mob::RoadNetworkPackageManager;
friend class sim_mob::Signal;
//friend class sim_mob::Worker;

};





}
