//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RailTransit.hpp"

#include <cstdlib>
#include <list>
#include <stdexcept>

using namespace std;
using namespace sim_mob;

sim_mob::RTS_NetworkEdge::RTS_NetworkEdge() : edgeTravelTime(0.0), transferEdge(false)
{
}

sim_mob::RTS_NetworkEdge::~RTS_NetworkEdge()
{
}

sim_mob::RailTransit::RailTransit()
{
}

sim_mob::RailTransit::~RailTransit()
{
}

RailTransit RailTransit::railTransit;

RailTransit& sim_mob::RailTransit::getInstance()
{
	return railTransit;
}

void sim_mob::RailTransit::initGraph(const std::set<string>& vertices, const std::vector<RTS_NetworkEdge>& edges)
{
	if(vertices.empty() || edges.empty())
	{
		throw std::runtime_error("insufficient data provided for rail transit graph construction");
	}

	for(const string& stn : vertices)
	{
		RT_Vertex stnVertex = boost::add_vertex(railTransitGraph);
		boost::put(boost::vertex_name, railTransitGraph, stnVertex, stn);
		rtVertexLookup[stn] = stnVertex;
	}

	for(const RTS_NetworkEdge& rtNwEdge : edges)
	{
		RT_Edge rtEdge;
		bool edgeAdded = false;
		RT_Vertex fromStnVertex = findVertex(rtNwEdge.getFromStationId());
		RT_Vertex toStnVertex = findVertex(rtNwEdge.getToStationId());
		boost::tie(rtEdge, edgeAdded) = boost::add_edge(fromStnVertex, toStnVertex, railTransitGraph);
		boost::put(boost::edge_weight, railTransitGraph, rtEdge, rtNwEdge.getEdgeTravelTime());
		boost::put(boost::edge_name, railTransitGraph, rtEdge, rtNwEdge.isTransferEdge()); //edge type is set as name
	}
}

vector<string> sim_mob::RailTransit::fetchBoardAlightStopSeq(string origin, string dest) const
{
	if(origin == dest) //trivial
	{
		return vector<string>();
	}

	RT_Vertex fromVertex = findVertex(origin);
	RT_Vertex toVertex = findVertex(dest);

	vector<RT_Vertex> p(boost::num_vertices(railTransitGraph));
	vector<double> d(boost::num_vertices(railTransitGraph));
	list<RT_Vertex> partialRes;
	vector<string> res;
	try
	{
		boost::dijkstra_shortest_paths(railTransitGraph, fromVertex, boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(RT_GoalVisitor(toVertex)));
	}
	catch (RT_FoundGoal& goal)
	{
		//Build backwards.
		for (RT_Vertex v = toVertex;; v = p[v])
		{
			partialRes.push_front(v);
			if (p[v] == v)
			{
				break;
			}
		}

		if(partialRes.empty()) //path should at least be the origin station and the destination station
		{
			char errBuf[50];
			sprintf(errBuf, "no path found between %s and %s", origin.c_str(), dest.c_str());
			throw std::runtime_error(errBuf);
		}

		//Now build forwards.
		string stnId;
		list<RT_Vertex>::const_iterator prevIt = partialRes.begin();
		list<RT_Vertex>::const_iterator currIt = prevIt; currIt++; //second element in list
		for (; currIt!=partialRes.end(); currIt++)
		{
			std::pair<RT_Edge, bool> edge = boost::edge(*prevIt, *currIt, railTransitGraph);
			if (!edge.second)
			{
				throw std::runtime_error("ERROR: Boost can't find an edge that it should know about.");
			}
			bool transferEdge = boost::get(boost::edge_name, railTransitGraph, edge.first);
			if(transferEdge)
			{
				stnId = boost::get(boost::vertex_name, railTransitGraph, *prevIt);
				res.push_back(stnId); // add alighting point for transfer
				stnId = boost::get(boost::vertex_name, railTransitGraph, *currIt);
				res.push_back(stnId); // add next boarding point for transfer
			}
			prevIt = currIt;
		}

		if(res.empty() || res.front() != origin)
		{
			res.insert(res.begin(), origin); // add first boarding stn
		}
		if(res.back() != dest)
		{
			res.push_back(dest); // add last alighting stn
		}
	}
	return res;
}

RailTransit::RT_Vertex sim_mob::RailTransit::findVertex(const std::string& vertexName) const
{
	std::map<std::string, RT_Vertex>::const_iterator vertexIt = rtVertexLookup.find(vertexName);
	if(vertexIt == rtVertexLookup.end())
	{
		char buf[100];
		sprintf(buf, "cannot find stn:%s in rail transit graph", vertexName.c_str());
		throw std::runtime_error(buf);
	}
	return vertexIt->second;
}
