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

#include "geospatial/network/Point.hpp"
#include "util/GeomHelpers.hpp"
#include "util/LangHelpers.hpp"
#include "StreetDirectory.hpp"
#include "util/Utils.hpp"

using std::map;
using std::vector;
using std::list;

namespace {
void printPath(const std::vector<sim_mob::PT_NetworkEdge>& path, double cost, bool addedtoB) {
	if (addedtoB) {
		std::cout << "Added to B: ";
	} else {
		std::cout << "Added to Q: ";
	}
	for (std::vector<sim_mob::PT_NetworkEdge>::const_iterator it = path.begin();
			it != path.end(); it++) {
		std::cout << it->getEdgeId() << "-->";
	}
	std::cout << "pathcost=" << cost;
	std::cout << "\n";
}

struct TempPathSet {
private:
	std::vector<std::vector<sim_mob::PT_NetworkEdge> > paths;
	std::vector<double> pathCosts;

public:
	void addPathIfUnavailable(const std::vector<sim_mob::PT_NetworkEdge>& path,	double cost) {
		if (std::find(paths.begin(), paths.end(), path) == paths.end()) {
			paths.push_back(path);
			pathCosts.push_back(cost);
			//printPath(path,cost,true);
		}
	}

	void deletePath(size_t index)
	{
		paths.erase(paths.begin() + index);
		pathCosts.erase(pathCosts.begin() + index);
	}

	uint32_t getMinElementIdx() const
	{
		return std::distance(pathCosts.begin(),	std::min_element(pathCosts.begin(), pathCosts.end()));
	}

	const std::vector<sim_mob::PT_NetworkEdge>& getPath(int index) const
	{
		if (index >= paths.size()) {
			throw std::runtime_error("invalid index");
		}
		return paths.at(index);
	}

	bool empty() const
	{
		return paths.empty();
	}
};

}

sim_mob::A_StarPublicTransitShortestPathImpl::A_StarPublicTransitShortestPathImpl(
		const std::map<int, PT_NetworkEdge>& ptEdgeMap,
		const std::map<std::string, PT_NetworkVertex>& ptVertexMap)
{
	initPublicNetwork(ptEdgeMap, ptVertexMap);
}

void sim_mob::A_StarPublicTransitShortestPathImpl::initPublicNetwork(
		const std::map<int, PT_NetworkEdge>& ptEdgeMap,	const std::map<std::string, PT_NetworkVertex>& ptVertexMap)
{
	for (std::map<std::string, PT_NetworkVertex>::const_iterator itVertex =	ptVertexMap.begin(); itVertex != ptVertexMap.end(); ++itVertex) {
		procAddPublicNetworkVertices(publicTransitMap, (itVertex->second));
	}

	for (std::map<int, PT_NetworkEdge>::const_iterator itEdge =	ptEdgeMap.begin(); itEdge != ptEdgeMap.end(); ++itEdge) {
		procAddPublicNetworkEdges(publicTransitMap, itEdge->second);
	}
}
void sim_mob::A_StarPublicTransitShortestPathImpl::procAddPublicNetworkVertices(
		StreetDirectory::PublicTransitGraph& graph,	const PT_NetworkVertex& ptVertex)
{
	StreetDirectory::PT_Vertex v = boost::add_vertex(const_cast<StreetDirectory::PublicTransitGraph &>(graph));
	boost::put(boost::vertex_name,	const_cast<StreetDirectory::PublicTransitGraph &>(graph), v, ptVertex.getRoadItemId());
	vertexMap[ptVertex.getRoadItemId()] = v;
}

void sim_mob::A_StarPublicTransitShortestPathImpl::procAddPublicNetworkEdges(
		StreetDirectory::PublicTransitGraph& graph,	const PT_NetworkEdge& ptEdge)
{
	StreetDirectory::PT_Vertex from = vertexMap.find(ptEdge.getStartStop())->second;
	StreetDirectory::PT_Vertex to = vertexMap.find(ptEdge.getEndStop())->second;
	bool ok;
	StreetDirectory::PT_Edge edge;
	boost::tie(edge, ok) = boost::add_edge(from, to, graph);
	edgeMap[ptEdge.getEdgeId()] = edge;
	procAddPublicNetworkEdgeWeights(edge, publicTransitMap, ptEdge);
}
double sim_mob::A_StarPublicTransitShortestPathImpl::getK_ShortestPathCost(const PT_NetworkEdge& ptEdge)
{
	return ptEdge.getDayTransitTimeSecs() + ptEdge.getTransferPenaltySecs()	+ ptEdge.getWalkTimeSecs() + ptEdge.getWaitTimeSecs();
}

double sim_mob::A_StarPublicTransitShortestPathImpl::getLabelingApproachWeights(int Label, PT_NetworkEdge ptEdge)
{
	double cost=0.0;
	switch (Label) {
	case LabelingApproach1: {
		ptEdge.setWalkTimeSecs(0);
		ptEdge.setWaitTimeSecs(0);
		break;
	}
	case LabelingApproach2: {
		ptEdge.setWalkTimeSecs(0);
		ptEdge.setWaitTimeSecs(0);
		ptEdge.setDayTransitTimeSecs(0);
		ptEdge.setTransferPenaltySecs(1);
		break;
	}
	case LabelingApproach3: {
		if (ptEdge.getType() == sim_mob::WALK_EDGE) {
			ptEdge.setWalkTimeSecs(100000000);
		}
		break;
	}
	case LabelingApproach4: {
		if (ptEdge.getType() != sim_mob::TRAIN_EDGE) {
			ptEdge.setTransferPenaltySecs(100000000);
		}
		break;
	}
	case LabelingApproach5: {
		if (ptEdge.getType() != sim_mob::BUS_EDGE) {
			ptEdge.setTransferPenaltySecs(100000000);
		}
		break;
	}
	case LabelingApproach6: {
		ptEdge.setDayTransitTimeSecs(0);
		ptEdge.setWalkTimeSecs(0);
		break;
	}
	case LabelingApproach7:
		// DO Nothing
		break;
	case LabelingApproach8: {
		double walkTime = ptEdge.getWalkTimeSecs();
		double waitTime = ptEdge.getWaitTimeSecs();
		ptEdge.setWalkTimeSecs(5 * walkTime);
		ptEdge.setWaitTimeSecs(10 * waitTime);
		break;
	}
	case LabelingApproach9: {
		double walkTime = ptEdge.getWalkTimeSecs();
		double waitTime = ptEdge.getWaitTimeSecs();
		ptEdge.setWalkTimeSecs(10 * walkTime);
		ptEdge.setWaitTimeSecs(10 * waitTime);
		break;
	}
	case LabelingApproach10: {
		double walkTime = ptEdge.getWalkTimeSecs();
		double waitTime = ptEdge.getWaitTimeSecs();
		ptEdge.setWalkTimeSecs(20 * walkTime);
		ptEdge.setWaitTimeSecs(10 * waitTime);
		break;
	}
	default:
		// Do Nothing
		break;
	}
	//Cost function for labeling approach. Cost = day_transit_time + walk_time + wait_time + transfer_penalty
	cost = ptEdge.getDayTransitTimeSecs() + ptEdge.getWalkTimeSecs() + ptEdge.getWaitTimeSecs() + ptEdge.getTransferPenaltySecs();
	return cost;
}

double sim_mob::A_StarPublicTransitShortestPathImpl::getSimulationApproachWeights(PT_NetworkEdge ptEdge)
{
	double cost = ptEdge.getDayTransitTimeSecs() + ptEdge.getWalkTimeSecs()	+ ptEdge.getWaitTimeSecs() + ptEdge.getTransferPenaltySecs();
	return abs(Utils::nRandom(cost, 5 * cost));
}
void sim_mob::A_StarPublicTransitShortestPathImpl::procAddPublicNetworkEdgeWeights(	const StreetDirectory::PT_Edge& edge,
		StreetDirectory::PublicTransitGraph& graph,	const PT_NetworkEdge& ptEdge)
{
	graph[edge].kShortestPathWeight = getK_ShortestPathCost(ptEdge);
	graph[edge].labelingApproach1Weight = getLabelingApproachWeights(LabelingApproach1, ptEdge);
	graph[edge].labelingApproach2Weight = getLabelingApproachWeights(LabelingApproach2, ptEdge);
	graph[edge].labelingApproach3Weight = getLabelingApproachWeights(LabelingApproach3, ptEdge);
	graph[edge].labelingApproach4Weight = getLabelingApproachWeights(LabelingApproach4, ptEdge);
	graph[edge].labelingApproach5Weight = getLabelingApproachWeights(LabelingApproach5, ptEdge);
	graph[edge].labelingApproach6Weight = getLabelingApproachWeights(LabelingApproach6, ptEdge);
	graph[edge].labelingApproach7Weight = getLabelingApproachWeights(LabelingApproach7, ptEdge);
	graph[edge].labelingApproach8Weight = getLabelingApproachWeights(LabelingApproach8, ptEdge);
	graph[edge].labelingApproach9Weight = getLabelingApproachWeights(LabelingApproach9, ptEdge);
	graph[edge].labelingApproach10Weight = getLabelingApproachWeights(LabelingApproach10, ptEdge);
	graph[edge].simulationApproach1Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach2Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach3Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach4Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach5Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach6Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach7Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach8Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach9Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].simulationApproach10Weight = getSimulationApproachWeights(ptEdge);
	graph[edge].edge_id = ptEdge.getEdgeId();
}

vector<sim_mob::PT_NetworkEdge> sim_mob::A_StarPublicTransitShortestPathImpl::searchShortestPath(
		const StreetDirectory::PT_VertexId& fromNode,const StreetDirectory::PT_VertexId& toNode, const PT_CostLabel cost)
{
	vector<PT_NetworkEdge> res = vector<sim_mob::PT_NetworkEdge>();
	StreetDirectory::PT_Vertex from;
	StreetDirectory::PT_Vertex to;
	if (vertexMap.find(fromNode) != vertexMap.end()) {
		from = vertexMap.find(fromNode)->second;
	} else {
		std::cout << "From Node " << fromNode << " not found in the graph\n";
		return vector<sim_mob::PT_NetworkEdge>();
	}
	if (vertexMap.find(toNode) != vertexMap.end()) {
		to = vertexMap.find(toNode)->second;
	} else {
		std::cout << "To Node " << toNode << " not found in the graph\n";
		return vector<sim_mob::PT_NetworkEdge>();
	}
	StreetDirectory::PublicTransitGraph& graph = publicTransitMap;
	list<StreetDirectory::PT_Vertex> partialRes;
	vector<StreetDirectory::PT_Vertex> p(boost::num_vertices(graph));
	vector<double> d(boost::num_vertices(graph));
	boost::property_map<StreetDirectory::PublicTransitGraph,double (StreetDirectory::PT_EdgeProperties::*)>::type edgeWeightMap;

	switch (cost) {
	case KshortestPath:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::kShortestPathWeight,graph);
		break;
	case LabelingApproach1:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach1Weight,graph);
		break;
	case LabelingApproach2:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach2Weight,graph);
		break;
	case LabelingApproach3:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach3Weight,graph);
		break;
	case LabelingApproach4:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach4Weight,graph);
		break;
	case LabelingApproach5:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach5Weight,graph);
		break;
	case LabelingApproach6:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach6Weight,graph);
		break;
	case LabelingApproach7:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach7Weight,graph);
		break;
	case LabelingApproach8:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach8Weight,graph);
		break;
	case LabelingApproach9:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach9Weight,graph);
		break;
	case LabelingApproach10:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::labelingApproach10Weight,graph);
		break;
	case SimulationApproach1:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach1Weight,graph);
		break;
	case SimulationApproach2:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach2Weight,graph);
		break;
	case SimulationApproach3:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach3Weight,graph);
		break;
	case SimulationApproach4:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach4Weight,graph);
		break;
	case SimulationApproach5:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach5Weight,graph);
		break;
	case SimulationApproach6:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach6Weight,graph);
		break;
	case SimulationApproach7:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach7Weight,graph);
		break;
	case SimulationApproach8:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach8Weight,graph);
		break;
	case SimulationApproach9:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach9Weight,graph);
		break;
	case SimulationApproach10:
		edgeWeightMap = boost::get(&StreetDirectory::PT_EdgeProperties::simulationApproach10Weight,	graph);
		break;
	}
	try {
		boost::astar_search(graph, from, PT_HeuristicGraph(&graph, to),
				boost::weight_map(edgeWeightMap).predecessor_map(&p[0]).distance_map(&d[0]).visitor(PT_GoalVisitor(to)));

	} catch (PT_FoundGoal& goal) {
		//Build backwards.
		for (StreetDirectory::PT_Vertex v = to;; v = p[v]) {
			partialRes.push_front(v);
			if (p[v] == v) {
				break;
			}
		}
		//Now build forwards.
		std::list<StreetDirectory::PT_Vertex>::const_iterator prev = partialRes.end();
		for (std::list<StreetDirectory::PT_Vertex>::const_iterator it =	partialRes.begin(); it != partialRes.end(); it++) {
			//Add this edge.
			if (prev != partialRes.end()) {
				//This shouldn't fail.
				std::pair<StreetDirectory::PT_Edge, bool> edge = boost::edge(*prev, *it, graph);
				if (!edge.second) {
					Warn() << "ERROR: Boost can't find an edge that it should know about."
							<< std::endl;
					return vector<sim_mob::PT_NetworkEdge>();
				}
				StreetDirectory::PT_EdgeId edge_id = get(&StreetDirectory::PT_EdgeProperties::edge_id, graph,edge.first);
				res.push_back(PT_Network::getInstance().PT_NetworkEdgeMap.find(edge_id)->second);
			}
			//Save for later.
			prev = it;
		}
	}
	return res;
}
vector<sim_mob::PT_NetworkEdge> sim_mob::A_StarPublicTransitShortestPathImpl::searchShortestPathWithBlacklist(
		const StreetDirectory::PT_VertexId& fromNode,const StreetDirectory::PT_VertexId& toNode,const std::set<StreetDirectory::PT_EdgeId>& blacklist,	double &pathCost)
{
	StreetDirectory::PublicTransitGraph& graph = publicTransitMap;
	StreetDirectory::PT_Vertex from;
	StreetDirectory::PT_Vertex to;
	if (vertexMap.find(fromNode) != vertexMap.end()) {
		from = vertexMap.find(fromNode)->second;
	} else {
		std::cout << "From Node not found in the graph";
		return vector<sim_mob::PT_NetworkEdge>();
	}
	if (vertexMap.find(toNode) != vertexMap.end()) {
		to = vertexMap.find(toNode)->second;
	} else {
		std::cout << "TO Node not found in the graph";
		return vector<sim_mob::PT_NetworkEdge>();
	}
	std::set<StreetDirectory::PT_Edge> blEdge = std::set<StreetDirectory::PT_Edge>();
	for (std::set<StreetDirectory::PT_EdgeId>::const_iterator blIt =blacklist.begin(); blIt != blacklist.end(); blIt++) {
		blEdge.insert(edgeMap[*blIt]);
	}

	PT_EdgeConstraint filter(blEdge);
	boost::filtered_graph<StreetDirectory::PublicTransitGraph, PT_EdgeConstraint> filtered(graph, filter);
	vector<sim_mob::PT_NetworkEdge> res;
	list<StreetDirectory::PT_Vertex> partialRes;
	vector<StreetDirectory::PT_Vertex> p(boost::num_vertices(graph));
	vector<double> d(boost::num_vertices(graph));
	try {
		boost::astar_search(filtered, from, PT_HeuristicGraph(&graph, to),
				boost::weight_map(get(&StreetDirectory::PT_EdgeProperties::kShortestPathWeight,	graph)).predecessor_map(&p[0]).
				distance_map(&d[0]).visitor(PT_GoalVisitor(to)));
	} catch (PT_FoundGoal& goal) {
		pathCost = d[to];
		//Build backwards.
		for (StreetDirectory::PT_Vertex v = to;; v = p[v]) {
			partialRes.push_front(v);
			if (p[v] == v) {
				break;
			}
		}
		//Now build forwards.
		std::list<StreetDirectory::PT_Vertex>::const_iterator prev =partialRes.end();
		for (std::list<StreetDirectory::PT_Vertex>::const_iterator it =	partialRes.begin(); it != partialRes.end(); it++) {
			//Add this edge.
			if (prev != partialRes.end()) {
				//This shouldn't fail.
				std::pair<StreetDirectory::PT_Edge, bool> edge = boost::edge(*prev, *it, graph);
				if (!edge.second) {
					Warn()	<< "ERROR: Boost can't find an edge that it should know about."
							<< std::endl;
					return vector<sim_mob::PT_NetworkEdge>();
				}

				StreetDirectory::PT_EdgeId edge_id = get(&StreetDirectory::PT_EdgeProperties::edge_id, graph,edge.first);
				res.push_back(PT_Network::getInstance().PT_NetworkEdgeMap.find(edge_id)->second);
			}
			//Save for later.
			prev = it;
		}
	}
	return res;
}

void sim_mob::A_StarPublicTransitShortestPathImpl::searchK_ShortestPaths(uint32_t kPaths, const StreetDirectory::PT_VertexId& from,
		const StreetDirectory::PT_VertexId& to,	vector<vector<sim_mob::PT_NetworkEdge> >& res)
{
	StreetDirectory::PublicTransitGraph& graph = publicTransitMap;

	double pathCost = 0.0;
	// Define Q to hold the k shortest paths
	vector<vector<sim_mob::PT_NetworkEdge> >& Q = res; //another reference to be consistent with Tan Rui's algorithm specification
	// Define B to hold the intermediate paths found (Only subset from this set goes to Q)
	TempPathSet B;

	//step 1: Search shortest path p_0
	std::set<StreetDirectory::PT_EdgeId> blackList = std::set<StreetDirectory::PT_EdgeId>();
	std::vector<PT_NetworkEdge> p0 = searchShortestPathWithBlacklist(from, to,blackList, pathCost);

	if (p0.empty()) {
		return;
	}
	Q.push_back(p0);
	//printPath(p0,pathCost,false);

	//step 2: Each iteration of the below loop gives the kth shortest path
	// For k in 1:K-1 (K - number of paths required )
	for (int k = 1; k < kPaths; k++) {
		//black list first edge from each path found already (p_0,p_1,p_2,...,p_(k-1))
		blackList.clear();
		for (int p = 0; p < k; p++) {
			StreetDirectory::PT_EdgeId firstEdgeId = Q[p].front().getEdgeId();
			blackList.insert(firstEdgeId);
		}
		//Search shortest path, denote p_temp, if p_temp doesn't exist in Q or B, B = (B union {p_temp})
		std::vector<PT_NetworkEdge> pTemp = searchShortestPathWithBlacklist(from, to, blackList, pathCost);
		blackList.clear();
		//Check if p_temp present in Q or B already
		if (!pTemp.empty() && std::find(Q.begin(), Q.end(), pTemp) == Q.end()) {
			B.addPathIfUnavailable(pTemp, pathCost);
		}

		size_t prevPathLength = Q[k - 1].size();
		if (prevPathLength - 1 >= 1) // Enter loop only if q(k-1) > 1 ;
		{
			const vector<PT_NetworkEdge>& prevPath = Q[k - 1];
			// For i in 1:q(k-1)
			for (int i = 0; i < prevPathLength - 1; i++) {
				vector<PT_NetworkEdge> rootpath(prevPath.begin(),prevPath.begin() + (i + 1));
				// For j in 1:k-1
				for (int j = 0; j <= k - 1; j++) {
					const std::vector<PT_NetworkEdge>& Q_j = Q[j];
					// Check if (e_1,e_2,...e_(i))in p_(k-1) is same as (e_1,e_2,...e(i)) in p_j
					if (std::equal(Q_j.begin(), Q_j.begin() + (i + 1),	rootpath.begin())) {
						// Block e(i+1) from path p_j in graph
						if (Q_j.size() > i + 1) {
							PT_NetworkEdge removeEdge = Q_j.at(i + 1);
							blackList.insert(removeEdge.getEdgeId());
						}
					}
				}

				// Denote (e_1,e_2,...e_(i-1)) as S_i. Find the shortest route R_i from ending vertex of e_i and same destination
				vector<PT_NetworkEdge> S_i(Q[k - 1].begin(),Q[k - 1].begin() + (i));
				StreetDirectory::PT_VertexId start;
				//Finding the ending vertex of the edge e_i in Q[k-1]
				if (S_i.empty()) {
					start = from;
				} else {
					StreetDirectory::PT_Vertex startVertex = boost::target(edgeMap[S_i.back().getEdgeId()], graph);
					start = boost::get(boost::vertex_name, graph, startVertex);
				}
				double s_i_cost = 0.0;
				for (std::vector<sim_mob::PT_NetworkEdge>::const_iterator itEdge = S_i.begin(); itEdge != S_i.end(); itEdge++) {
					s_i_cost +=	graph[sim_mob::A_StarPublicTransitShortestPathImpl::edgeMap[itEdge->getEdgeId()]].kShortestPathWeight;
				}
				vector<PT_NetworkEdge> R_i = searchShortestPathWithBlacklist(start, to, blackList, pathCost);
				blackList.clear();
				if (!R_i.empty()) {
					// Concatenate S_i and R_i to obtain A_i_k
					vector<PT_NetworkEdge> A_i_k;
					A_i_k.insert(A_i_k.end(), S_i.begin(), S_i.end());
					A_i_k.insert(A_i_k.end(), R_i.begin(), R_i.end());
					// If A_i_k not exist in B not in Q
					// B = Union(B,A_i_k)
					if (std::find(Q.begin(), Q.end(), A_i_k) == Q.end()) {
						B.addPathIfUnavailable(A_i_k, (pathCost + s_i_cost));
					}
				}
			}
		}
		// If B is empty Break the whole loop
		if (B.empty()) {
			break;
		}

		// Remove the least cost path A_j in B and put it in Q as p_k
		uint32_t index = B.getMinElementIdx();
		Q.push_back(B.getPath(index));
		//printPath(B.getPath(index),0.0,false);
		B.deletePath(index);
	}
	return;
}
