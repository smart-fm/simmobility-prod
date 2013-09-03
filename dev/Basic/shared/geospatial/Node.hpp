//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

#include "conf/settings/DisableMPI.h"
#include "util/LangHelpers.hpp"
#include "util/OpaqueProperty.hpp"
#include "geospatial/Point2D.hpp"




namespace geo{
//Forward Declaration
class Node_t_pimpl;
class GeoSpatial_t_pimpl;
}

namespace sim_mob
{
//Forward declarations
class Lane;
class LaneConnector;
class RoadSegment;
class Conflux;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * A location on a map where other elements interact. Nodes contain a Point2D representing their
 * location. Additional information (such as lane connectors) are located in other classes (e.g.,
 * Intersections, Roundabouts, and SegmentNodes.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 *
 * Nodes should not be constructed directly. Instead, they exist to provide a uniform interface
 * to define locations in a RoadNetwork. UniNodes and MultiNodes (and their subclasses) provide
 * more comprehensive functionality, and their sub-classes provide even more.
 */
class Node {

//TODO: A lot of this should be private/protected, but the XML loaders need access (at the moment).
//      Should be re-worked later; we really never create "base" Nodes. ~Seth

		//Exactly,But we need this for XML reader when base class information are communicated to children
		//For Now, I will keep working in this way until we find a better solution-vahid
friend class ::geo::Node_t_pimpl;
public:
	unsigned int nodeId;//read from DB
public:
	virtual ~Node() {} //A virtual destructor allows dynamic casting

	///The location of this Node.
	///TODO: Restore const access later.
	Point2D location;


	//Nodes may have hidden properties useful only in for the visualizer.
	OpaqueProperty<int> originalDB_ID;

#ifndef SIMMOB_DISABLE_MPI
	///The identification of Node is packed using PackageUtils;
	static void pack(PackageUtils& package, const Node* one_node);

	///UnPackageUtils use the identification of Node to find the Node Object
	static Node* unpack(UnPackageUtils& unpackage);
#endif


//protected:
    explicit Node(int x=0, int y=0, unsigned int nodeId_=0) : nodeId(nodeId_), location(x, y), parentConflux(nullptr) {}

//private:
//    sim_mob::Link* linkLoc;

public:
    void setID(unsigned int);
    unsigned int getID()const;
  //  void setLinkLoc(sim_mob::Link* link);

  //  sim_mob::Link* getLinkLoc() const;
  const Point2D getLocation() const { return location;}

  sim_mob::Conflux* getParentConflux() const { return parentConflux; }

  void setParentConflux(sim_mob::Conflux* parentConflux) { this->parentConflux = parentConflux; }

  // Conflux to which this node belongs to
  mutable sim_mob::Conflux* parentConflux;
};



}
