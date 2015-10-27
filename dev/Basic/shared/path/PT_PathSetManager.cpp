#include "PT_PathSetManager.hpp"
#include "path/Path.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/Constructs.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Connection.hpp"
#include "util/threadpool/Threadpool.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

using std::vector;
using std::string;
using namespace sim_mob;
namespace {

const std::string LABELLING_APPROACH = "LBL";
const std::string KSHORTEST_PATH = "KSH";
const std::string LINK_ELIMINATION_APPROACH = "LEA";
const std::string SIMULATION_APPROACH = "SNA";

class simpleOD {
private:
	int startNode;
	int destNode;
public:
	simpleOD(int start, int dest) {
		startNode = start;
		destNode = dest;
	}
	int getStartNode() const {
		return startNode;
	}
	void setStartNode(int node) {
		startNode = node;
	}
	int getDestNode() const {
		return destNode;
	}
	void setDestNode(int node) {
		destNode = node;
	}
};

class compareOD: public std::less<simpleOD> {
public:
	bool operator()(const simpleOD& start, const simpleOD& dest) const
	{
		return ((start.getStartNode() < dest.getStartNode())
				|| (start.getDestNode() < dest.getDestNode()));
	}
};

}

PT_PathSetManager sim_mob::PT_PathSetManager::instance;
boost::shared_ptr<sim_mob::batched::ThreadPool> sim_mob::PT_PathSetManager::threadpool;

PT_PathSetManager::PT_PathSetManager():labelPoolSize(10)
{
	ptPathSetWriter.open(ConfigManager::GetInstance().FullConfig().getPathSetConf().publicPathSetOutputFile.c_str());
}
PT_PathSetManager::~PT_PathSetManager()
{

}
PT_PathSetManager&  PT_PathSetManager::Instance()
{
	return instance;
}
std::string PT_PathSetManager::getVertexIdFromNode(const sim_mob::Node* node)
{
	//As in the PT_Network vertices simMobility nodes are represented as the N_ plus node id
	//to avoid the conflict with the bus stop id's
	unsigned int node_id = node->getNodeId();
	string stopId = boost::lexical_cast<std::string>("N_" + boost::lexical_cast<std::string>(node_id));
	return stopId;
}


void PT_PathSetManager::PT_BulkPathSetGenerator()
{
	ptPathSetWriter.open(ConfigManager::GetInstance().FullConfig().getPathSetConf().publicPathSetOutputFile.c_str());
	std::set<simpleOD, compareOD> simpleOD_Set;
	//Reading the data from the database
	const std::string& dbId =ConfigManager::GetInstance().FullConfig().networkDatabase.database;
	Database database =ConfigManager::GetInstance().FullConfig().constructs.databases.at(dbId);
	std::string credId =ConfigManager::GetInstance().FullConfig().networkDatabase.credentials;
	Credential credentials =ConfigManager::GetInstance().FullConfig().constructs.credentials.at(credId);
	std::string username = credentials.getUsername();
	std::string password = credentials.getPassword(false);
	sim_mob::db::DB_Config dbConfig(database.host, database.port,database.dbName, username, password);

	sim_mob::db::DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	conn.connect();
	std::stringstream query;
	soci::session& sql_ = conn.getSession<soci::session>();
	query << "select * from "<< sim_mob::ConfigManager::GetInstance().FullConfig().getPathSetConf().publicPathSetOdSource;
	soci::rowset < soci::row > rs = (sql_.prepare << query.str());
	for (soci::rowset<soci::row>::const_iterator it = rs.begin();it != rs.end(); ++it) {
		simpleOD singleOD((*it).get<int>(0), (*it).get<int>(1));
		simpleOD_Set.insert(singleOD);
	}

	writePathSetFileHeader();
	if (!threadpool) {
		int size =sim_mob::ConfigManager::GetInstance().PathSetConfig().threadPoolSize;
		threadpool.reset(new sim_mob::batched::ThreadPool(sim_mob::ConfigManager::GetInstance().PathSetConfig().threadPoolSize));
	}
	int total_count = simpleOD_Set.size();
	Print() << "Total OD's in Bulk generation is " << total_count;
	const RoadNetwork* rn = RoadNetwork::getInstance();
	const std::map<unsigned int, Node *>& nodeLookup = rn->getMapOfIdvsNodes();
	for(std::set<simpleOD>::const_iterator it=simpleOD_Set.begin();it!=simpleOD_Set.end();it++)
	{
		const sim_mob::Node* srcNode = rn->getById(nodeLookup, it->getStartNode());
		const sim_mob::Node* destNode = rn->getById(nodeLookup, it->getDestNode());
		threadpool->enqueue(boost::bind(&sim_mob::PT_PathSetManager::makePathset,this,srcNode,destNode));
	}
	threadpool->wait();

	conn.disconnect();
}

PT_PathSet PT_PathSetManager::makePathset(const sim_mob::Node* from,const sim_mob::Node* to)
{
	PT_PathSet ptPathSet;
	StreetDirectory::PT_VertexId fromId = getVertexIdFromNode(from);
	StreetDirectory::PT_VertexId toId = getVertexIdFromNode(to);

	//K-Shortest path Approach
	getK_ShortestPaths(fromId, toId, ptPathSet);
	//Labeling Approach
	getLabelingApproachPaths(fromId, toId, ptPathSet);
	// Link Elimination Approach
	getLinkEliminationApproachPaths(fromId, toId, ptPathSet);
	//Simulation approach
	getSimulationApproachPaths(fromId, toId, ptPathSet);
	//computing path size
	ptPathSet.computeAndSetPathSize();
	// Checking the feasibility of the paths in the pathset. Infeasible paths are removed.
	ptPathSet.checkPathFeasibilty();
	// Writing the pathSet to the CSV file.
	writePathSetToFile(ptPathSet, from->getNodeId(), to->getNodeId());

	return ptPathSet;
}

void PT_PathSetManager::writePathSetFileHeader()
{
	this->ptPathSetWriter << "PtPathId," << "ptPathSetId," << "scenario,"
			<< "PathTravelTime," << "TotalDistanceKms," << "PathSize,"
			<< "TotalCost," << "Total_In_Vehicle_Travel_Time_Secs,"
			<< "Total_waiting_time," << "Total_walking_time,"
			<< "Total_Number_of_transfers," << "isMinDistance,"
			<< "isValidPath," << "isShortestPath,"
			<< "isMinInVehicleTravelTimeSecs," << "isMinNumberOfTransfers,"
			<< "isMinWalkingDistance," << "isMinTravelOnMrt,"
			<< "isMinTravelOnBus," << "pathset_origin_node,"
			<< "pathset_dest_node" << std::endl;
}

void PT_PathSetManager::writePathSetToFile(const PT_PathSet &ptPathSet,	unsigned int fromNodeId, unsigned int toNodeId)
{
	fileExclusiveWrite.lock();
	for (std::set<PT_Path, cmp_path_vector>::const_iterator itPath = ptPathSet.pathSet.begin(); itPath != ptPathSet.pathSet.end();itPath++)
	{
		this->ptPathSetWriter << "\"" << itPath->getPtPathId() << "\"" << ","<< "\"" << itPath->getPtPathSetId() << "\"" << ","
				<< itPath->getScenario() << "," << itPath->getPathTravelTime()<< ","
				<< itPath->getTotalDistanceKms() << ","
				<< itPath->getPathSize() << "," << itPath->getTotalCost() << ","
				<< itPath->getTotalInVehicleTravelTimeSecs() << ","
				<< itPath->getTotalWaitingTimeSecs() << ","
				<< itPath->getTotalWalkingTimeSecs() << ","
				<< itPath->getTotalNumberOfTransfers() << ","
				<< itPath->isMinDistance() << "," << itPath->isValidPath()<< ","
				<< itPath->isShortestPath() << ","
				<< itPath->isMinInVehicleTravelTimeSecs() << ","
				<< itPath->isMinNumberOfTransfers() << ","
				<< itPath->isMinWalkingDistance() << ","
				<< itPath->isMinTravelOnMrt() << ","
				<< itPath->isMinTravelOnBus() << "," << fromNodeId << ","
				<< toNodeId << std::endl;
	}
	fileExclusiveWrite.unlock();
}

void PT_PathSetManager::getLabelingApproachPaths(const StreetDirectory::PT_VertexId& fromId,const StreetDirectory::PT_VertexId& toId, PT_PathSet& ptPathSet)
{
	for (int i = 0; i < labelPoolSize; i++) {
		vector<PT_NetworkEdge> path;
		path = StreetDirectory::Instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromId, toId, (PT_CostLabel) (i + LabelingApproach1));
		if (path.size() != 0) {
			PT_Path ptPath(path);
			if (i == LabelingApproach1) {
				ptPath.setMinInVehicleTravelTimeSecs(true);
			}
			if (i == LabelingApproach2) {
				ptPath.setMinNumberOfTransfers(true);
			}
			if (i == LabelingApproach3) {
				ptPath.setMinWalkingDistance(true);
			}
			if (i == LabelingApproach4) {
				ptPath.setMinTravelOnMrt(true);
			}
			if (i == LabelingApproach5) {
				ptPath.setMinTravelOnBus(true);
			}
			std::stringstream scenario;
			scenario << LABELLING_APPROACH << i;
			ptPath.setScenario(scenario.str());
			ptPathSet.pathSet.insert(ptPath);
		}
	}
}

void PT_PathSetManager::getK_ShortestPaths(const StreetDirectory::PT_VertexId& fromId,const StreetDirectory::PT_VertexId& toId, PT_PathSet& ptPathSet)
{
	int i = 0;
	vector<vector<PT_NetworkEdge> > kShortestPaths;
	int kShortestLevel = ConfigManager::GetInstance().FullConfig().getPathSetConf().publickShortestPathLevel;
	StreetDirectory::Instance().getPublicTransitShortestPathImpl()->searchK_ShortestPaths(kShortestLevel, fromId, toId, kShortestPaths);
	for (vector<vector<PT_NetworkEdge> >::iterator itPath =	kShortestPaths.begin(); itPath != kShortestPaths.end(); itPath++) {
		i++;
		PT_Path ptPath(*itPath);
		if (kShortestPaths.begin() == itPath) {
			ptPath.setShortestPath(true);
		}
		std::stringstream scenario;
		scenario << KSHORTEST_PATH << i;
		ptPath.setScenario(scenario.str());
		ptPathSet.pathSet.insert(ptPath);
	}
}

void PT_PathSetManager::getLinkEliminationApproachPaths(const StreetDirectory::PT_VertexId& fromId,	const StreetDirectory::PT_VertexId& toId, PT_PathSet& ptPathSet)
{
	int i = 0;
	vector<PT_NetworkEdge> shortestPath;
	shortestPath =StreetDirectory::Instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromId, toId, KshortestPath);
	for (vector<PT_NetworkEdge>::iterator edgeIt = shortestPath.begin();edgeIt != shortestPath.end(); edgeIt++)
	{
		vector<PT_NetworkEdge> path;
		double cost = 0;
		std::set<StreetDirectory::PT_EdgeId> blackList = std::set<StreetDirectory::PT_EdgeId>();
		blackList.insert(edgeIt->getEdgeId());
		path =StreetDirectory::Instance().getPublicTransitShortestPathImpl()->searchShortestPathWithBlacklist(fromId, toId, blackList, cost);
		if (path.size() != 0) {
			i++;
			PT_Path ptPath(path);
			std::stringstream scenario;
			scenario << LINK_ELIMINATION_APPROACH << i;
			ptPath.setScenario(scenario.str());
			ptPathSet.pathSet.insert(ptPath);
		}
	}
}

void PT_PathSetManager::getSimulationApproachPaths(const StreetDirectory::PT_VertexId& fromId,const StreetDirectory::PT_VertexId& toId, PT_PathSet& ptPathSet)
{
	int simulationApproachPoolSize = ConfigManager::GetInstance().FullConfig().getPathSetConf().simulationApproachIterations;
	for (int i = 0; i < simulationApproachPoolSize; i++) {
		vector<PT_NetworkEdge> path;
		path =StreetDirectory::Instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromId, toId, (PT_CostLabel) (i + SimulationApproach1));
		if (path.size() != 0) {
			PT_Path ptPath(path);
			std::stringstream scenario;
			scenario << SIMULATION_APPROACH << (i + 1);
			ptPath.setScenario(scenario.str());
			ptPathSet.pathSet.insert(ptPath);
		}
	}
}
