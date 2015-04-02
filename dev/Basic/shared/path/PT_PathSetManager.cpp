#include "PT_PathSetManager.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "path/Path.hpp"

using std::vector;
using std::string;
using namespace sim_mob;
namespace{
	const std::string LABELLING_APPROACH ="LBL";
	const std::string KSHORTEST_PATH = "KSH";
	const std::string LINK_ELIMINATION_APPROACH ="LEA";
	const std::string SIMULATION_APPROACH = "SNA";
}
PT_PathSetManager sim_mob::PT_PathSetManager::_instance;

PT_PathSetManager::PT_PathSetManager():labelPoolSize(10), simulationApproachPoolSize(10){
	ptPathSetWriter.open("PT_Pathset.csv");
}
PT_PathSetManager::~PT_PathSetManager() {
	// TODO Auto-generated destructor stub
}
PT_NetworkVertex PT_PathSetManager::getVertexFromNode(sim_mob::Node* node)
{
	//As in the PT_Network vertices simMobility nodes are represented as the N_ and node id
	// to avoid the conflict with the bus stop id's
	string starter="N_";
	unsigned int node_id = node->getID();
	string stopId = boost::lexical_cast<std::string>("N_"+boost::lexical_cast<std::string>(node_id));
	//return PT_Network::getInstance().getVertexFromStopId(stopId);
	return PT_Network::getInstance().PublicTransitVertexMap.find(stopId)->second;
}

std::string PT_PathSetManager::getVertexIdFromNode(sim_mob::Node* node)
{
	//As in the PT_Network vertices simMobility nodes are represented as the N_ and node id
	// to avoid the conflict with the bus stop id's
	string starter="N_";
	unsigned int node_id = node->getID();
	string stopId = boost::lexical_cast<std::string>("N_"+boost::lexical_cast<std::string>(node_id));
	return stopId;
}
PT_PathSet PT_PathSetManager::makePathset(sim_mob::Node* from,sim_mob::Node* to)
{
	StreetDirectory::PT_VertexId fromId =getVertexIdFromNode(from);
	StreetDirectory::PT_VertexId toId= getVertexIdFromNode(to);
	PT_PathSet ptPathSet;

	// KShortestpath Approach
		getkShortestPaths(fromId,toId,ptPathSet);
	//Labeling Approach
	getLabelingApproachPaths(fromId,toId,ptPathSet);

	// Link Elimination Approach
	getLinkEliminationApproachPaths(fromId,toId,ptPathSet);

	//Simulation approach
	getSimulationApproachPaths(fromId,toId,ptPathSet);
	ptPathSet.computeAndSetPathSize();

	// Writing the pathSet to the CSV file.

	return ptPathSet;
}
void PT_PathSetManager::writePathSetToFile(PT_PathSet &ptPathSet)
{
	for(std::set<PT_Path,cmp_path_vector>::const_iterator itPath=ptPathSet.pathSet.begin();itPath!=ptPathSet.pathSet.end();itPath++)
	{
		this->ptPathSetWriter<<"\""<<itPath->getPtPathId()<<"\""<<itPath->getPtPathSetId()<<itPath->getScenario()<<itPath->getPartialUtility()
				<<itPath->getPathTravelTime()<<itPath->getTotalDistanceKms()<<itPath->getPathSize()<<itPath->getTotalCost()<<itPath->getTotalInVehicleTravelTimeSecs()
				<<itPath->getTotalWaitingTimeSecs()<<itPath->getTotalWalkingTimeSecs()<<itPath->getTotalNumberOfTransfers()<<itPath->isMinDistance()
				<<itPath->isValidPath()<<itPath->isShortestPath()<<itPath->isMinInVehicleTravelTimeSecs()<<itPath->isMinNumberOfTransfers()
				<<itPath->isMinWalkingDistance()<<itPath->isMinTravelOnMrt()<<itPath->isMinTravelOnBus()<<"\n";
	}
}
void PT_PathSetManager::getLabelingApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet)
{
	for(int i=0;i<labelPoolSize;i++)
	{
		vector<PT_NetworkEdge> path;
		path = StreetDirectory::instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromId,toId,i+LabelingApproach1);
		PT_Path ptPath(path);
		if(i==LabelingApproach1){
			ptPath.setMinInVehicleTravelTimeSecs(true);
		}
		if(i==LabelingApproach2){
			ptPath.setMinNumberOfTransfers(true);
		}
		if(i==LabelingApproach3){
			ptPath.setMinWalkingDistance(true);
		}
		if(i==LabelingApproach4){
			ptPath.setMinTravelOnMrt(true);
		}
		if(i==LabelingApproach5){
			ptPath.setMinTravelOnBus(true);
		}
		ptPath.setScenario(LABELLING_APPROACH);
		ptPathSet.pathSet.insert(ptPath);
	}
}

void PT_PathSetManager::getkShortestPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet)
{
	vector<vector<PT_NetworkEdge> > kShortestPaths;
	StreetDirectory::instance().getPublicTransitShortestPathImpl()->getKShortestPaths(fromId,toId,kShortestPaths);
	for(vector<vector<PT_NetworkEdge> >::iterator itPath=kShortestPaths.begin();itPath!=kShortestPaths.end();itPath++)
	{
		PT_Path ptPath(*itPath);
		if(kShortestPaths.begin() == itPath){
			ptPath.setShortestPath(true);
		}
		ptPath.setScenario(KSHORTEST_PATH);
		ptPathSet.pathSet.insert(ptPath);
	}
}

void PT_PathSetManager::getLinkEliminationApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet)
{
	vector<PT_NetworkEdge> shortestPath;
	shortestPath = StreetDirectory::instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromId,toId,KshortestPath);

	for(vector<PT_NetworkEdge>::iterator edgeIt=shortestPath.begin();edgeIt!=shortestPath.end();edgeIt++)
	{
		vector<PT_NetworkEdge> path;
		double cost=0;
		std::set<StreetDirectory::PT_EdgeId> blackList = std::set<StreetDirectory::PT_EdgeId>();
		blackList.insert(edgeIt->getEdgeId());
		path=StreetDirectory::instance().getPublicTransitShortestPathImpl()->searchShortestPathWithBlacklist(fromId,toId,KshortestPath,blackList,cost);
		PT_Path ptPath(path);
		ptPath.setScenario(LINK_ELIMINATION_APPROACH);
		ptPathSet.pathSet.insert(ptPath);
	}
}

void PT_PathSetManager::getSimulationApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet)
{
	for(int i=0;i<simulationApproachPoolSize;i++)
	{
		vector<PT_NetworkEdge> path;
		path = StreetDirectory::instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromId,toId,i+SimulationApproach1);
		PT_Path ptPath(path);
		ptPath.setScenario(SIMULATION_APPROACH);
		ptPathSet.pathSet.insert(ptPath);
	}
}


