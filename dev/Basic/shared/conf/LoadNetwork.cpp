/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "LoadNetwork.hpp"

#include "conf/Config.hpp"
#include "geospatial/RoadNetwork.hpp"

using std::map;
using std::set;
using std::list;
using std::vector;
using std::string;

using namespace sim_mob;




sim_mob::LoadNetwork::LoadNetwork(Config& cfg) : cfg(cfg)
{
	///TODO: Load the network, possibly from multiple sources.


	//Workaround required for legacy purposes: generate all Lane and LaneEdge lines.
	RoadNetwork::ForceGenerateAllLaneEdgePolylines(cfg.network());

}
