//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/Point2D.hpp"
#include "util/GeomHelpers.hpp"
#include "util/LangHelpers.hpp"

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
#include "A_StarPublicTransitShortestPathImpl.hpp"

using std::map;

sim_mob::A_StarPublicTransitShortestPathImpl::A_StarPublicTransitShortestPathImpl(std::vector<PT_NetworkEdges> ptEdgesList,std::vector<PT_NetworkVertices> ptVerticesList)
{
	initPublicNetwork(ptEdgesList,ptVerticesList);
}

void sim_mob::A_StarPublicTransitShortestPathImpl::initPublicNetwork(std::vector<PT_NetworkEdges> ptEdgesList,std::vector<PT_NetworkVertices> ptVerticesList)
{
	map<const std::string,StreetDirectory::PT_Vertex> vertexmap;
	for(std::vector<PT_NetworkVertices>::iterator itVertex=ptVerticesList.begin();itVertex!=ptVerticesList.end();++itVertex)
	{
		procAddPublicNetworkVertices(publictransitMap_,(*itVertex),vertexmap);
	}

	for(std::vector<PT_NetworkEdges>::iterator itEdge=ptEdgesList.begin();itEdge!=ptEdgesList.end();++itEdge)
	{
		procAddPublicNetworkEdges(publictransitMap_,(*itEdge),vertexmap);
	}
}
void sim_mob::A_StarPublicTransitShortestPathImpl::procAddPublicNetworkVertices(StreetDirectory::PublicTransitGraph& graph,PT_NetworkVertices ptVertex,map<const std::string,StreetDirectory::PT_Vertex> vertexmap)
{
	StreetDirectory::PT_Vertex v = boost::add_vertex(const_cast<StreetDirectory::PublicTransitGraph &>(graph));
	boost::put(boost::vertex_name, const_cast<StreetDirectory::PublicTransitGraph &>(graph),v,ptVertex.getStopId());
	vertexmap[ptVertex.getStopId()]=v;
}

void sim_mob::A_StarPublicTransitShortestPathImpl::procAddPublicNetworkEdges(StreetDirectory::PublicTransitGraph& graph,PT_NetworkEdges ptEdge,map<const std::string,StreetDirectory::PT_Vertex> vertexmap)
{
	StreetDirectory::PT_Vertex from = vertexmap.find(ptEdge.getStartStop())->second;
	StreetDirectory::PT_Vertex to = vertexmap.find(ptEdge.getEndStop())->second;
	StreetDirectory::PT_Edge edge;
	bool ok;
	boost::tie(edge, ok) = boost::add_edge(from, to, graph);
	boost::put(boost::edge_name, graph, edge,ptEdge.getEdgeId());
}
