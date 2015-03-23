//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "A_StarPublicTransitShortestPathImpl.hpp"

#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/unordered_map.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/transform_value_property_map.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "geospatial/Point2D.hpp"
#include "util/GeomHelpers.hpp"
#include "util/LangHelpers.hpp"
#include "StreetDirectory.hpp"

using std::map;
using std::vector;
using std::list;

sim_mob::A_StarPublicTransitShortestPathImpl::A_StarPublicTransitShortestPathImpl(std::vector<PT_NetworkEdge> ptEdgesList,std::vector<PT_NetworkVertex> ptVerticesList)
{
	initPublicNetwork(ptEdgesList,ptVerticesList);
}

void sim_mob::A_StarPublicTransitShortestPathImpl::initPublicNetwork(std::vector<PT_NetworkEdge> &ptEdgesList,std::vector<PT_NetworkVertex> &ptVerticesList)
{
	for(std::vector<PT_NetworkVertex>::iterator itVertex=ptVerticesList.begin();itVertex!=ptVerticesList.end();++itVertex)
	{
		procAddPublicNetworkVertices(publictransitMap_,(*itVertex));
	}

	for(std::vector<PT_NetworkEdge>::iterator itEdge=ptEdgesList.begin();itEdge!=ptEdgesList.end();++itEdge)
	{
		procAddPublicNetworkEdges(publictransitMap_,(*itEdge));
	}
}
void sim_mob::A_StarPublicTransitShortestPathImpl::procAddPublicNetworkVertices(StreetDirectory::PublicTransitGraph& graph,PT_NetworkVertex ptVertex)
{
	StreetDirectory::PT_Vertex v = boost::add_vertex(const_cast<StreetDirectory::PublicTransitGraph &>(graph));
	boost::put(boost::vertex_name, const_cast<StreetDirectory::PublicTransitGraph &>(graph),v,ptVertex.getStopId());
	vertexMap[ptVertex.getStopId()]=v;
}

void sim_mob::A_StarPublicTransitShortestPathImpl::procAddPublicNetworkEdges(StreetDirectory::PublicTransitGraph& graph,PT_NetworkEdge ptEdge)
{
	StreetDirectory::PT_Vertex from = vertexMap.find(ptEdge.getStartStop())->second;
	StreetDirectory::PT_Vertex to = vertexMap.find(ptEdge.getEndStop())->second;
	StreetDirectory::PT_Edge edge;
	bool ok;
	boost::tie(edge, ok) = boost::add_edge(from, to, graph);
	edgeMap[ptEdge.getEdgeId()]=edge;
	procAddPublicNetworkEdgeweights(edge,publictransitMap_,ptEdge);
}
double sim_mob::A_StarPublicTransitShortestPathImpl::getKshortestPathcost(const PT_NetworkEdge& ptEdge)
{
	return ptEdge.getDayTransitTimeSecs()+ptEdge.getTransferPenaltySecs()+ptEdge.getWalkTimeSecs();
}
void sim_mob::A_StarPublicTransitShortestPathImpl::procAddPublicNetworkEdgeweights(const StreetDirectory::PT_Edge& edge,StreetDirectory::PublicTransitGraph& graph,PT_NetworkEdge& ptEdge)
{
	graph[edge].kShortestPathWeight=getKshortestPathcost(ptEdge);
	graph[edge].edge_id=ptEdge.getEdgeId();
}

//  Function searchShortestPath finds the shortest path using astar algorithm
// Input - from node , to node , graph
// output - List of egde id of the edges in the graph traversing the shortest path
//TODO:Make it template so it can be used for any weight parameter given
vector<int> sim_mob::A_StarPublicTransitShortestPathImpl::searchShortestPath(PT_NetworkVertex fromNode,PT_NetworkVertex toNode,PT_WeightLabels cost)
{
	    vector<int> res;
	    StreetDirectory::PT_Vertex from = vertexMap[fromNode.getStopId()];
	    StreetDirectory::PT_Vertex to = vertexMap[toNode.getStopId()];
	    StreetDirectory::PublicTransitGraph& graph = publictransitMap_;
		list<StreetDirectory::PT_Vertex> partialRes;
		vector<StreetDirectory::PT_Vertex> p(boost::num_vertices(graph));  //Output variable
		vector<double> d(boost::num_vertices(graph));  //Output variable
		//boost::property_map<StreetDirectory::PublicTransitGraph,double[]> x;
		//x=get(&StreetDirectory::PT_EdgeProperties::weightMap,graph);
		//boost::property_map<StreetDirectory::PublicTransitGraph,double*>::type kWeightMap;
		//kWeightMap = get(&StreetDirectory::PT_EdgeProperties::label,graph);
		//auto kWeightMap = boost::make_transform_value_property_map(
		  //              [](StreetDirectory::PT_EdgeProperties* ve) { return ve->label[cost]; },
		  //              boost::get(boost::edge_bundle,graph)
		  //         );
		//boost::property_map<StreetDirectory::PublicTransitGraph, boost::edge_weight_t>::type kWeightMap;
		//kWeightMap = get(&StreetDirectory::PT_EdgeProperties::label1,graph);
		try {
			boost::astar_search(
				graph,
				from,
				distance_heuristic_graph_PT(&graph, to),
				boost::weight_map(get(&StreetDirectory::PT_EdgeProperties::kShortestPathWeight,graph)).predecessor_map(&p[0]).
					distance_map(&d[0]).visitor(astar_goal_visitor(to))
			);

		} catch (found_goal& goal) {
			//Build backwards.
			for (StreetDirectory::PT_Vertex v=to;;v=p[v]) {
				partialRes.push_front(v);
			    if(p[v] == v) {
			    	break;
			    }
			}
			//Now build forwards.
			std::list<StreetDirectory::PT_Vertex>::const_iterator prev = partialRes.end();
			for (std::list<StreetDirectory::PT_Vertex>::const_iterator it=partialRes.begin(); it!=partialRes.end(); it++) {
				//Add this edge.
				if (prev!=partialRes.end()) {
					//This shouldn't fail.
					std::pair<StreetDirectory::PT_Edge, bool> edge = boost::edge(*prev, *it, graph);
					if (!edge.second) {
						Warn() <<"ERROR: Boost can't find an edge that it should know about." <<std::endl;
						return vector<int>();
					}

					//Retrieve, add this edge's WayPoint.
					//WayPoint wp = boost::get(boost::edge_name, graph, edge.first);
					int edge_id = get(&StreetDirectory::PT_EdgeProperties::edge_id, graph,edge.first);
					res.push_back(edge_id);
				}
				//Save for later.
				prev = it;
			}
		}
		return res;
}
vector<int> sim_mob::A_StarPublicTransitShortestPathImpl::searchShortestPathWithBlacklist(PT_NetworkVertex fromNode,PT_NetworkVertex toNode,PT_WeightLabels cost,const std::set<StreetDirectory::PT_Edge>& blacklist,double &path_cost)
{
	    StreetDirectory::PublicTransitGraph& graph = publictransitMap_;
	    StreetDirectory::PT_Vertex from = vertexMap[fromNode.getStopId()];
	    StreetDirectory::PT_Vertex to = vertexMap[toNode.getStopId()];
	    blacklist_PT_edge_constraint filter(blacklist);
		boost::filtered_graph<StreetDirectory::PublicTransitGraph, blacklist_PT_edge_constraint> filtered(graph, filter);
		// Same code copied from searchShortestPath function as we use filtered graph in this function rather than
		// adjacency list in the other
		//TODO: can fine way to have generic function
		vector<StreetDirectory::PT_EdgeId> res;
		list<StreetDirectory::PT_Vertex> partialRes;
		vector<StreetDirectory::PT_Vertex> p(boost::num_vertices(graph));  //Output variable
		vector<double> d(boost::num_vertices(graph));  //Output variable
		try
		{
				boost::astar_search(
				filtered,from,
				distance_heuristic_graph_PT(&graph, to),
				boost::weight_map(get(&StreetDirectory::PT_EdgeProperties::kShortestPathWeight,graph)).predecessor_map(&p[0]).
				distance_map(&d[0]).visitor(astar_goal_visitor(to))
				);
		} catch (found_goal& goal) {
			path_cost=d[to];
			//Build backwards.
			for (StreetDirectory::PT_Vertex v=to;;v=p[v]) {
				partialRes.push_front(v);
				if(p[v] == v) {
					break;
				}
			}
			//Now build forwards.
			std::list<StreetDirectory::PT_Vertex>::const_iterator prev = partialRes.end();
			for (std::list<StreetDirectory::PT_Vertex>::const_iterator it=partialRes.begin(); it!=partialRes.end(); it++) {
				//Add this edge.
				if (prev!=partialRes.end()) {
					//This shouldn't fail.
					std::pair<StreetDirectory::PT_Edge, bool> edge = boost::edge(*prev, *it, graph);
					if (!edge.second) {
						Warn() <<"ERROR: Boost can't find an edge that it should know about." <<std::endl;
						return vector<StreetDirectory::PT_EdgeId>();
					}

					//Retrieve, add this edge's WayPoint.
					//WayPoint wp = boost::get(boost::edge_name, graph, edge.first);
					StreetDirectory::PT_EdgeId edge_id = get(&StreetDirectory::PT_EdgeProperties::edge_id, graph,edge.first);
					res.push_back(edge_id);
				}
				//Save for later.
				prev = it;
			}
		}
		return res;
}

int sim_mob::A_StarPublicTransitShortestPathImpl::getKShortestPaths(PT_NetworkVertex& from,PT_NetworkVertex& to, vector<vector<StreetDirectory::PT_EdgeId> > &res)
{
	// Define Q to hold the k shortest paths
	int kShortestPaths=10;
	StreetDirectory::PublicTransitGraph& graph = publictransitMap_;
	PT_WeightLabels KshortestPath;
	double path_cost;
	std::vector<vector<StreetDirectory::PT_EdgeId> > Q;
	struct Btype{
		vector<vector<StreetDirectory::PT_EdgeId> > paths;
		vector<double> pathCosts;
	};

	// Define B to hold the intermediate paths found (Only subset from this set goes to Q)
	Btype B;

	//step 1: Search shortest path p_1
	std::set<StreetDirectory::PT_Edge> bl = std::set<StreetDirectory::PT_Edge>();
	std::vector<StreetDirectory::PT_EdgeId> p_1 = searchShortestPathWithBlacklist(from,to,KshortestPath,bl,path_cost);
	Q.push_back(p_1);

	// For k in 1:K-1 (K - number of paths required )
	for(int k=1;k<kShortestPaths;k++)
	{

		//step 2: Each iteration of the below code gives the kth shortest path
		// Eliminate first edge from each paths found already(p_1,p_2,...,p_(k-1))
		for(int p=0;p<k;p++)
		{
			StreetDirectory::PT_EdgeId first_edge_id = Q[p].front();
			StreetDirectory::PT_Edge first_edge= edgeMap[first_edge_id];
			bl.insert(first_edge);
		}

		//Search Shortest path , denote p_temp , if p_temp doesnt exist in Q or B , B = union(B,p_temp)
		std::vector<StreetDirectory::PT_EdgeId> p_temp = searchShortestPathWithBlacklist(from,to,KshortestPath,bl,path_cost);


		//Check if p_temp present in Q or B already
		if(std::find(Q.begin(), Q.end(), p_temp)==Q.end())
		{
			if(std::find(B.paths.begin(),B.paths.end(),p_temp)==B.paths.end())
			{
				B.paths.push_back(p_temp);
				B.pathCosts.push_back(path_cost);

			}
		}

		// q(k-1) - Number of edges in the path p_(k-1)
		// If q(k-1) > 1 continue ;
		if(Q[k-1].size()<=1)
		{
			continue;
		}


		// For i in 1:q(k-1)
		for(int i=0;i<Q[k-1].size();i++)
		{
			vector<StreetDirectory::PT_EdgeId> rootpath(Q[k-1].begin(),Q[k-1].begin()+i);
			//std::copy(Q[k-1].begin(),Q.begin()+i,rootpath.begin());

			// For j in 1:k-1
			for(int j=0;j<k-1;j++)
			{
				// Check if (e_1,e_2,...e_(i))in p_(k-1) is same as (e_1,e_2,...e(i)) in p_j
				if(std::equal(Q[j].begin(),Q[j].begin()+i,rootpath.begin()))
				{

					// Block e(i+1) from path p_j in graph
					StreetDirectory::PT_EdgeId remove_edge_id = Q[j].at(i+1);
					StreetDirectory::PT_Edge remove_edge= edgeMap[remove_edge_id];
					bl.insert(remove_edge);
				}
			}

			// Denote (e_1,e_2,...e_(i-1)) as S_i. Find the shortest route R_i from ending vertex of e_i and same destination
			vector<StreetDirectory::PT_EdgeId> S_i(Q[k-1].begin(),Q[k-1].begin()+(i-1));
			vector<StreetDirectory::PT_EdgeId> R_i;
			//std::copy(Q[k-1].begin(),Q.begin()+(i-1),S_i.begin());

			//Finding the ending vertex of the edge e_i in Q[k-1]
			StreetDirectory::PT_Vertex start_vertex = boost::target(edgeMap[rootpath.back()],graph);
			PT_NetworkVertex start = PT_Network::getInstance().getVertexFromStopId(boost::get(boost::vertex_name,graph,start_vertex));
			R_i = searchShortestPathWithBlacklist(start,to,KshortestPath,bl,path_cost);

			// Concatenate S_i and R_i to obtain A_i_k
			vector<StreetDirectory::PT_EdgeId> A_i_k ;
			A_i_k.insert( A_i_k.end(),S_i.begin(),S_i.end());
			A_i_k.insert( A_i_k.end(),R_i.begin(),R_i.end());


			// If A_i_k not exist in B not in Q
			// B = Union(B,A_i_k)
			if(std::find(Q.begin(), Q.end(), A_i_k)==Q.end())
			{
				if(std::find(B.paths.begin(),B.paths.end(),A_i_k)==B.paths.end())
				{
					B.paths.push_back(A_i_k);
					B.pathCosts.push_back(path_cost);
				}
			}
		}

		// If B is empty Break the whole loop
		if(B.paths.size() == 0)
		{
			break;
		}

		// Remove the least cost path A_j in B and put it in Q as p_k
		int index=std::distance(B.pathCosts.begin(),std::min_element(B.pathCosts.begin(),B.pathCosts.end()));
		Q.push_back(B.paths.at(index));
		B.paths.erase(B.paths.begin()+index);
		B.pathCosts.erase(B.pathCosts.begin()+index);
	}
	res=Q; // Assigning the result to the reference variable
}

/*
void sim_mob::A_StarPublicTransitShortestPathImpl::shortest_path(map<const std::string,StreetDirectory::PT_Vertex> vertexmap)
{
	std::ofstream filewriter;
	filewriter.open("graph.txt");
	std::vector<int> d(boost::num_vertices(publictransitMap_));
	std::vector<StreetDirectory::PT_Vertex> p(boost::num_vertices(publictransitMap_));
	std::string start_node= "10009";
	std::string end_node = "10001";
	StreetDirectory::PT_Vertex start_vertex = vertexmap.find(start_node)->second;
	StreetDirectory::PT_Vertex toVertex = vertexmap.find(end_node)->second;

	boost::dijkstra_shortest_paths(publictransitMap_,start_vertex,
		boost::weight_map(get(&StreetDirectory::PT_EdgeProperties::label_2_weight, publictransitMap_)).
		predecessor_map(&p[0]).distance_map(&d[0]));
	//std::cout<< "Distance from source vertex 10009 to end vertex 10001 is ";
	//IndexMap index = get(boost::vertex_index,publictransitMap_);
	//std::cout<<d[toVertex];
	//typedef boost::graph_traits<StreetDirectory::PublicTransitGraph>::vertex_iterator vertex_iter;
	 //   std::pair<vertex_iter, vertex_iter> vp;
	  //  for (vp = vertices(publictransitMap_); vp.first != vp.second; ++vp.first)
	   //   std::cout << *vp.first<<" " << index[*vp.first] <<  " "<<std::endl;
	  boost::get
	  std::cout << "distances from start vertex:" << std::endl;
	  boost::graph_traits<StreetDirectory::PublicTransitGraph>::vertex_iterator vi;
	  for(vi = vertices(publictransitMap_).first; vi != vertices(publictransitMap_).second; ++vi)
	  filewriter << "distance() = "
	              << d[*vi] << std::endl;
	  filewriter << std::endl;
}
*/
