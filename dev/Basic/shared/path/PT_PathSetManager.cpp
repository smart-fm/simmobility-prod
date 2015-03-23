#include "PT_PathSetManager.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

using std::vector;
using std::string;
using namespace sim_mob;

PT_PathSetManager sim_mob::PT_PathSetManager::_instance;

PT_PathSetManager::PT_PathSetManager() {

}
PT_PathSetManager::~PT_PathSetManager() {
	// TODO Auto-generated destructor stub
}
PT_NetworkVertex PT_PathSetManager::getVertexFromNode(sim_mob::Node* node){
	//As in the PT_Network vertices simMobility nodes are represented as the N_ and node id
	// to avoid the conflict with the bus stop id's
	string starter="N_";
	unsigned int node_id = node->getID();
	string stop_id = boost::lexical_cast<std::string>("N_"+boost::lexical_cast<std::string>(node_id));
	return PT_Network::getInstance().getVertexFromStopId(stop_id);
}

vector<PT_NetworkEdge> PT_PathSetManager::makePathset(sim_mob::Node* from,sim_mob::Node* to)
{
	PT_NetworkVertex fromVertex= getVertexFromNode(from);
	PT_NetworkVertex toVertex= getVertexFromNode(to);
	//StreetDirectory::instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromVertex,toVertex);
	return vector<PT_NetworkEdge>();
}
