//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/Point2D.hpp"
#include "util/GeomHelpers.hpp"
#include "util/LangHelpers.hpp"
#include "entities/params/Pt_network_entities.hpp"

#include <map>
#include <vector>
#include <string>
#include <ostream>

#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include "StreetDirectory.hpp"


namespace sim_mob {


class A_StarPublicTransitShortestPathImpl : public StreetDirectory::ShortestPathImpl {
public:
    explicit A_StarPublicTransitShortestPathImpl(std::vector<PT_NetworkEdges> pt_edgeslist,std::vector<PT_NetworkVertices> pt_verticeslist);
    virtual ~A_StarPublicTransitShortestPathImpl() {}


public:
    StreetDirectory::PublicTransitGraph publictransitMap_;

private:
    //New functions

    void initPublicNetwork(std::vector<PT_NetworkEdges> ptEdgesList,std::vector<PT_NetworkVertices> ptVerticesList);
    void procAddPublicNetworkVertices(StreetDirectory::PublicTransitGraph& graph,PT_NetworkVertices ptVertex,std::map<const std::string,StreetDirectory::PT_Vertex> vertexmap);
    void procAddPublicNetworkEdges(StreetDirectory::PublicTransitGraph& graph,PT_NetworkEdges ptEdge,std::map<const std::string,StreetDirectory::PT_Vertex> vertexmap);

};
}
