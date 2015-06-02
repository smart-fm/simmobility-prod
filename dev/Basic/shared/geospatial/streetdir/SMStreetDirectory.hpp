//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

//#include "geospatial/Point2D.hpp"
//#include "geospatial/WayPoint.hpp"
//#include "metrics/Length.hpp"
//#include "util/LangHelpers.hpp"
//#include "entities/params/PT_NetworkEntities.hpp"

#include "geospatial/simmobility_network/WayPoint.hpp"
#include "geospatial/simmobility_network/Node.hpp"
#include "geospatial/simmobility_network/Link.hpp"
#include "geospatial/simmobility_network/Point.hpp"


namespace simmobility_network
{

class SMStreetDirectory : private boost::noncopyable{
public:
	SMStreetDirectory();
	virtual ~SMStreetDirectory();

public:
	 static SMStreetDirectory& instance() {
	        return instance_;
	    }

	/**
	 * Internal typedef to StreetDirectory representing:
	 *   key:    The "vertex_name" property.
	 *   value:  The Point representing that vertex's location. This point is not 100% accurate,
	 *           and should be considered a rough guideline as to the vertex's location.
	 * This is just a handy way of retrieving data "stored" at a given vertex.
	 */
	typedef boost::property<boost::vertex_name_t, simmobility_network::Point> SMVertexProperties;

	/**
	 * Internal typedef to StreetDirectory representing:
	 *   keys:   The "edge_weight" and "edge_name" properties.
	 *   values: The Euclidean distance of this edge, and the WayPoint which represents this edge's traversal.
	 * The distance is needed for our A* search, and the WayPoint is used when returning the actual results.
	 */

	typedef boost::property<boost::edge_weight_t, double,
				boost::property<boost::edge_name_t, simmobility_network::WayPoint > > SMEdgeProperties;


	/**
	 * Internal typedef to StreetDirectory representing:
	 *   The actual graph, bound to its VertexProperties and EdgeProperties (see those comments for details on what they store).
	 * You can use StreetDirectory::Graph to mean "a graph" in all contexts.
	 */
	typedef boost::adjacency_list<boost::vecS,
									  boost::vecS,
									  boost::directedS,
									  SMVertexProperties,
									  SMEdgeProperties> SMGraph;

	/**
	 * Internal typedef to StreetDirectory representing:
	 *   A Vertex within our graph. Internally, these are defined as some kind of integer, but you should
	 *   simply treat this as an identifying handle.
	 * You can use StreetDirectory::Vertex to mean "a vertex" in all contexts.
	 */
	typedef SMGraph::vertex_descriptor SMVertex;

	/**
	 * Internal typedef to StreetDirectory representing:
	 *   An Edge within our graph. Internally, these are defined as pairs of integers (fromVertex, toVertex),
	 *   but you should simply treat this as an identifying handle.
	 * You can use StreetDirectory::Edge to mean "an edge" in all contexts.
	 */
	typedef SMGraph::edge_descriptor SMEdge;


	//A return value for "Driving/WalkingVertex"
	struct SMVertexDesc {
		bool valid;    //Is this a valid struct? If false, treat as "null"
		SMVertex source; //The outgoing Vertex (used for "source" master nodes).
		SMVertex sink;   //The incoming Vertex (used for "sink" master nodes).

		SMVertexDesc(bool valid=false) : valid(valid), source(SMVertex()), sink(SMVertex()) {}
	};

    /**
     * Provides an implementation of the StreetDirectory's shortest-path lookup functionality. See Impl's description for
     *  the general idea with these classes.
     */
    class SMShortestPathImpl {
    protected:
    	//ShortestPathImpl();   //Abstract?

    	///Retrieve a Vertex based on a Node, BusStop, etc.. Flag in the return value is false to indicate failure.
    	virtual SMVertexDesc DrivingVertex(const simmobility_network::Node& n) const = 0;

    	//Meant to be used with the "DrivingVertex/WalkingVertex" functions.
        virtual std::vector<simmobility_network::WayPoint> GetShortestDrivingPath(simmobility_network::Node* from,
        													 simmobility_network::Node* to,
															 std::vector<const simmobility_network::Link*> blacklist=std::vector<const simmobility_network::Link*>()) const = 0;

        //TODO: Does this work the way I want it to?
        friend class SMStreetDirectory;
    };

private:
	 static SMStreetDirectory instance_;
};


}
