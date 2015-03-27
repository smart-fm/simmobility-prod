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
#include "util/Utils.hpp"

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

double sim_mob::A_StarPublicTransitShortestPathImpl::getLabelingApproachWeights(int Label,PT_NetworkEdge ptEdge)
{
	double cost;
	switch(Label)
	{
		case LabelingApproach1:
		{
			ptEdge.setWalkTimeSecs(0);
			ptEdge.setWaitTimeSecs(0);
			break;
		}
		case LabelingApproach2:
		{
			ptEdge.setWalkTimeSecs(0);
			ptEdge.setWaitTimeSecs(0);
			ptEdge.setDayTransitTimeSecs(0);
			ptEdge.setTransferPenaltySecs(1);
			break;
		}
		case LabelingApproach3:
		{
			if (ptEdge.getType()=="Walk")
			{
				ptEdge.setWalkTimeSecs(1000000);
			}
			break;
		}
		case LabelingApproach4:
		{
			if(ptEdge.getType()=="MRT")
			{
				ptEdge.setTransferPenaltySecs(1000000);
			}
			break;
		}
		case LabelingApproach5:
		{
			if(ptEdge.getType()=="Bus")
			{
				ptEdge.setTransferPenaltySecs(1000000);
			}
			break;
		}
		case LabelingApproach6:
		{
			ptEdge.setDayTransitTimeSecs(0);
			ptEdge.setWalkTimeSecs(0);
			break;
		}
		case LabelingApproach7:
			// DO Nothing
			break;
		case LabelingApproach8:
		{
			double walkTime = ptEdge.getWalkTimeSecs();
			double waitTime = ptEdge.getWaitTimeSecs();
			ptEdge.setWalkTimeSecs(5*walkTime);
			ptEdge.setWaitTimeSecs(10*waitTime);
			break;
		}
		case LabelingApproach9:
		{
			double walkTime = ptEdge.getWalkTimeSecs();
			double waitTime = ptEdge.getWaitTimeSecs();
			ptEdge.setWalkTimeSecs(10*walkTime);
			ptEdge.setWaitTimeSecs(10*waitTime);
			break;
		}
		case LabelingApproach10:
		{
			double walkTime = ptEdge.getWalkTimeSecs();
			double waitTime = ptEdge.getWaitTimeSecs();
			ptEdge.setWalkTimeSecs(20*walkTime);
			ptEdge.setWaitTimeSecs(10*waitTime);
			break;
		}
		default:
			// Do Nothing
			break;
	}
	//Cost function for labeling approach
	//Cost = day_transit_time + walk_time + wait_time + transfer_penalty
	cost= ptEdge.getDayTransitTimeSecs()+ptEdge.getWalkTimeSecs()+ptEdge.getWaitTimeSecs()+ptEdge.getTransferPenaltySecs();
	return cost;
}
double sim_mob::A_StarPublicTransitShortestPathImpl::getSimulationApproachWeights(PT_NetworkEdge ptEdge)
{
	double cost = ptEdge.getDayTransitTimeSecs()+ptEdge.getWalkTimeSecs()+ptEdge.getWaitTimeSecs();
	return Utils::nRandom(cost,5*cost);
}
void sim_mob::A_StarPublicTransitShortestPathImpl::procAddPublicNetworkEdgeweights(const StreetDirectory::PT_Edge& edge,StreetDirectory::PublicTransitGraph& graph,PT_NetworkEdge& ptEdge)
{
	graph[edge].kShortestPathWeight=getKshortestPathcost(ptEdge);

	graph[edge].labelingApproach1Weight=getLabelingApproachWeights(LabelingApproach1,ptEdge);
	graph[edge].labelingApproach2Weight=getLabelingApproachWeights(LabelingApproach2,ptEdge);
	graph[edge].labelingApproach3Weight=getLabelingApproachWeights(LabelingApproach3,ptEdge);
	graph[edge].labelingApproach4Weight=getLabelingApproachWeights(LabelingApproach4,ptEdge);
	graph[edge].labelingApproach5Weight=getLabelingApproachWeights(LabelingApproach5,ptEdge);
	graph[edge].labelingApproach6Weight=getLabelingApproachWeights(LabelingApproach6,ptEdge);
	graph[edge].labelingApproach7Weight=getLabelingApproachWeights(LabelingApproach7,ptEdge);
	graph[edge].labelingApproach8Weight=getLabelingApproachWeights(LabelingApproach8,ptEdge);
	graph[edge].labelingApproach9Weight=getLabelingApproachWeights(LabelingApproach9,ptEdge);
	graph[edge].labelingApproach10Weight=getLabelingApproachWeights(LabelingApproach10,ptEdge);
	graph[edge].simulationApproach1Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach2Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach3Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach4Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach5Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach6Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach7Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach8Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach9Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach10Weight=getSimulationApproachWeights(ptEdge);
	graph[edge].edge_id=ptEdge.getEdgeId();
}

//  Function searchShortestPath finds the shortest path using astar algorithm
// Input - from node , to node , graph
// output - List of egde id of the edges in the graph traversing the shortest path
//TODO:Make it template so it can be used for any weight parameter given
vector<sim_mob::PT_NetworkEdge> sim_mob::A_StarPublicTransitShortestPathImpl::searchShortestPath(StreetDirectory::PT_VertexId fromNode,StreetDirectory::PT_VertexId toNode,int cost)
{
	    vector<PT_NetworkEdge> res;
	    StreetDirectory::PT_Vertex from;
	    StreetDirectory::PT_Vertex to;
		if(vertexMap.find(fromNode) != vertexMap.end())
		{
			from = vertexMap.find(fromNode)->second;
		}
		else
		{
			std::cout<<"From Node not found in the graph";
			return vector<StreetDirectory::PT_EdgeId>();
		}
		if(vertexMap.find(toNode) != vertexMap.end())
		{
			to = vertexMap.find(toNode)->second;
		}
		else
		{
			std::cout<<"To Node not found in the graph";
			return vector<StreetDirectory::PT_EdgeId>();
		}
	    StreetDirectory::PublicTransitGraph& graph = publictransitMap_;
		list<StreetDirectory::PT_Vertex> partialRes;
		vector<StreetDirectory::PT_Vertex> p(boost::num_vertices(graph));  //Output variable
		vector<double> d(boost::num_vertices(graph));  //Output variable
		boost::property_map<StreetDirectory::PublicTransitGraph, double (StreetDirectory::PT_EdgeProperties::*)[]>::type weightMap;
		switch(cost)
		{
			case KshortestPath:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::kShortestPathWeight, graph);
				break;
			case LabelingApproach1:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach1Weight, graph);
				break;
			case LabelingApproach2:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach2Weight, graph);
				break;
			case LabelingApproach3:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach3Weight, graph);
				break;
			case LabelingApproach4:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach4Weight, graph);
				break;
			case LabelingApproach5:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach5Weight, graph);
				break;
			case LabelingApproach6:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach6Weight, graph);
				break;
			case LabelingApproach7:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach7Weight, graph);
				break;
			case LabelingApproach8:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach8Weight, graph);
				break;
			case LabelingApproach9:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach9Weight, graph);
				break;
			case LabelingApproach10:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach10Weight, graph);
				break;
			case SimulationApproach1:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach1Weight, graph);
				break;
			case SimulationApproach2:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach2Weight, graph);
				break;
			case SimulationApproach3:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach3Weight, graph);
				break;
			case SimulationApproach4:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach4Weight, graph);
				break;
			case SimulationApproach5:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach5Weight, graph);
				break;
			case SimulationApproach6:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach6Weight, graph);
				break;
			case SimulationApproach7:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach7Weight, graph);
				break;
			case SimulationApproach8:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach8Weight, graph);
				break;
			case SimulationApproach9:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach9Weight, graph);
				break;
			case SimulationApproach10:
				weightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach10Weight, graph);
				break;
		}
		try {
			boost::astar_search(
				graph,
				from,
				distance_heuristic_graph_PT(&graph, to),
				boost::weight_map(weightMap).predecessor_map(&p[0]).distance_map(&d[0])
				.visitor(astar_pt_goal_visitor(to)));

		}
		catch (pt_found_goal& goal) {
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
					res.push_back(PT_Network::getInstance().PT_NetworkEdgeMap.find(edge_id));
				}
				//Save for later.
				prev = it;
			}
		}
		return res;
}
vector<sim_mob::PT_NetworkEdge> sim_mob::A_StarPublicTransitShortestPathImpl::searchShortestPathWithBlacklist(StreetDirectory::PT_VertexId fromNode,StreetDirectory::PT_VertexId toNode,int cost,const std::set<StreetDirectory::PT_EdgeId>& blacklist,double &path_cost)
{
		StreetDirectory::PublicTransitGraph& graph = publictransitMap_;
		StreetDirectory::PT_Vertex from;
		StreetDirectory::PT_Vertex to;
		if(vertexMap.find(fromNode) != vertexMap.end())
		{
			 from = vertexMap.find(fromNode)->second;
		}
		else
		{
			std::cout<<"From Node not found in the graph";
			return vector<StreetDirectory::PT_EdgeId>();
		}
		if(vertexMap.find(toNode) != vertexMap.end())
		{
			 to = vertexMap.find(toNode)->second;
		}
		else
		{
			std::cout<<"TO Node not found in the graph";
			return vector<StreetDirectory::PT_EdgeId>();
		}
		std::set<StreetDirectory::PT_Edge> blEdge = std::set<StreetDirectory::PT_Edge>();
		for(std::set<StreetDirectory::PT_EdgeId>::const_iterator blIt=blacklist.begin();blIt!=blacklist.end();blIt++)
		{
			blEdge.insert(edgeMap[*blIt]);
		}
	    blacklist_PT_edge_constraint filter(blEdge);
		boost::filtered_graph<StreetDirectory::PublicTransitGraph, blacklist_PT_edge_constraint> filtered(graph, filter);
		// Same code copied from searchShortestPath function as we use filtered graph in this function rather than
		// adjacency list in the other
		//TODO: can fine way to have generic function
		vector<sim_mob::PT_NetworkEdge> res;
		list<StreetDirectory::PT_Vertex> partialRes;
		vector<StreetDirectory::PT_Vertex> p(boost::num_vertices(graph));  //Output variable
		vector<double> d(boost::num_vertices(graph));  //Output variable
		try
		{
				boost::astar_search(
				filtered,from,
				distance_heuristic_graph_PT(&graph, to),
				boost::weight_map(get(&StreetDirectory::PT_EdgeProperties::kShortestPathWeight,graph)).predecessor_map(&p[0]).
				distance_map(&d[0]).visitor(astar_pt_goal_visitor(to))
				);
		} catch (pt_found_goal& goal) {
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
					res.push_back(PT_Network::getInstance().PT_NetworkEdgeMap.find(edge_id));
				}
				//Save for later.
				prev = it;
			}
		}
		return res;
}

int sim_mob::A_StarPublicTransitShortestPathImpl::getKShortestPaths(StreetDirectory::PT_VertexId from,StreetDirectory::PT_VertexId to, vector<vector<sim_mob::PT_NetworkEdge> > &res)
{

	int kShortestPaths=10;
	StreetDirectory::PublicTransitGraph& graph = publictransitMap_;
	PT_WeightLabels KshortestPath;
	double path_cost;
	struct tempPathSet {
		vector<vector<StreetDirectory::PT_EdgeId> > paths;
		vector<double> pathCosts;
	};


	// Define Q to hold the k shortest paths
	std::vector<vector<StreetDirectory::PT_EdgeId> > Q;

	// Define B to hold the intermediate paths found (Only subset from this set goes to Q)
	tempPathSet B;

	//step 1: Search shortest path p_1
	std::set<StreetDirectory::PT_EdgeId> bl = std::set<StreetDirectory::PT_EdgeId>();
	std::vector<StreetDirectory::PT_EdgeId> p_1 = searchShortestPathWithBlacklist(from,to,KshortestPath,bl,path_cost);
	Q.push_back(p_1);

	// For k in 1:K-1 (K - number of paths required )
	for(int k=1;k<kShortestPaths;k++)
	{
		bl.clear();
		//step 2: Each iteration of the below code gives the kth shortest path
		// Eliminate first edge from each paths found already(p_1,p_2,...,p_(k-1))
		for(int p=0;p<k;p++)
		{
			StreetDirectory::PT_EdgeId firstEdgeId = Q[p].front();
			bl.insert(firstEdgeId);
		}

		//Search Shortest path , denote p_temp , if p_temp doesnt exist in Q or B , B = union(B,p_temp)
		std::vector<StreetDirectory::PT_EdgeId> p_temp = searchShortestPathWithBlacklist(from,to,KshortestPath,bl,path_cost);

		bl.clear();
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
					StreetDirectory::PT_EdgeId removeEdgeId = Q[j].at(i+1);
					bl.insert(removeEdgeId);
				}
			}

			// Denote (e_1,e_2,...e_(i-1)) as S_i. Find the shortest route R_i from ending vertex of e_i and same destination
			vector<StreetDirectory::PT_EdgeId> S_i(Q[k-1].begin(),Q[k-1].begin()+(i-1));
			vector<StreetDirectory::PT_EdgeId> R_i;

			//Finding the ending vertex of the edge e_i in Q[k-1]
			StreetDirectory::PT_Vertex startVertex = boost::target(edgeMap[rootpath.back()],graph);
			StreetDirectory::PT_VertexId start = boost::get(boost::vertex_name,graph,startVertex);
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
	 // Assigning the result to the reference variable
	for(vector<vector<StreetDirectory::PT_EdgeId> >::const_pointer itVecEdge=Q.begin();itVecEdge!=Q.end();itVecEdge++)
	{
		vector<StreetDirectory::PT_EdgeId> temp;
		for(vector<StreetDirectory::PT_EdgeId>::const_pointer itEdge=(itVecEdge)->begin();itEdge!=itVecEdge->end();itEdge++)
		{
			temp.push_back(PT_Network::getInstance().PT_NetworkEdgeMap.find(*itEdge));
		}
		res.push_back(temp);
	}
	return 1;
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
