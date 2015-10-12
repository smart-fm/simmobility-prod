#include "PT_PathSetManager.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "path/Path.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/Constructs.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Connection.hpp"
#include "util/threadpool/Threadpool.hpp"

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

boost::shared_ptr<sim_mob::batched::ThreadPool> sim_mob::PT_PathSetManager::threadpool_;

PT_PathSetManager::PT_PathSetManager():labelPoolSize(10){
	ptPathSetWriter.open(ConfigManager::GetInstance().FullConfig().pathSet().publicPathSetOutputFile.c_str());
	//ptPathSetWriter.open("/home/data1/pt_paths.csv");
}
PT_PathSetManager::~PT_PathSetManager() {
	// TODO Auto-generated destructor stub
}
std::string PT_PathSetManager::getVertexIdFromNode(sim_mob::Node* node)
{
	//As in the PT_Network vertices simMobility nodes are represented as the N_ and node id
	// to avoid the conflict with the bus stop id's
	string starter="N_";
	unsigned int node_id = node->getNodeId();
	string stopId = boost::lexical_cast<std::string>("N_"+boost::lexical_cast<std::string>(node_id));
	return stopId;
}
bool sim_mob::compare_OD::operator()(const PT_OD& A,const PT_OD& B) const {
		return ((A.getStartNode() < B.getStartNode()) || (A.getDestNode() < B.getDestNode()));
}

void PT_PathSetManager::PT_BulkPathSetGenerator()
{
	this->ptPathSetWriter.open(ConfigManager::GetInstance().FullConfig().pathSet().publicPathSetOutputFile.c_str());
	std::set<PT_OD,compare_OD> PT_OD_Set;
	//Reading the data from the database
	const std::string& dbId = ConfigManager::GetInstance().FullConfig().system.networkDatabase.database;
	Database database = ConfigManager::GetInstance().FullConfig().constructs.databases.at(dbId);
	std::string cred_id = ConfigManager::GetInstance().FullConfig().system.networkDatabase.credentials;
	Credential credentials = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(cred_id);
	std::string username = credentials.getUsername();
	std::string password = credentials.getPassword(false);
	sim_mob::db::DB_Config dbConfig(database.host, database.port, database.dbName, username, password);

	sim_mob::db::DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	conn.connect();
	soci::session& sql_ = conn.getSession<soci::session>();

	std::stringstream query;
	query << "select * from " << sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().publicPathSetOdSource;
	soci::rowset<soci::row> rs = (sql_.prepare << query.str());
	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
	{
	   soci::row const& row = *it;
	   PT_OD singleOD(row.get<int>(0),row.get<int>(1));
	   PT_OD_Set.insert(singleOD);
	}
	writePathSetFileHeader();
	if(!threadpool_)
	{
		int size = sim_mob::ConfigManager::GetInstance().PathSetConfig().threadPoolSize;
		threadpool_.reset(new sim_mob::batched::ThreadPool(sim_mob::ConfigManager::GetInstance().PathSetConfig().threadPoolSize));
	}
	int total_count = PT_OD_Set.size();
	Print() << "Total OD's in Bulk generation is "<<total_count;
	/*for(std::set<PT_OD>::const_iterator OD_It=PT_OD_Set.begin();OD_It!=PT_OD_Set.end();OD_It++)
	{
		sim_mob::Node* src_node = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(OD_It->getStartNode());
		sim_mob::Node* dest_node = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(OD_It->getDestNode());
		//sim_mob::PT_PathSetManager::Instance().makePathset(src_node,dest_node);
		threadpool_->enqueue(boost::bind(&sim_mob::PT_PathSetManager::makePathset,this,src_node,dest_node));
	}*/
	threadpool_->wait();
	conn.disconnect();
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

	// Checking the feasibility of the paths in the pathset.
	// Infeasible paths are removed.
	ptPathSet.checkPathFeasibilty();
	// Writing the pathSet to the CSV file.

	writePathSetToFile(ptPathSet,from->getNodeId(),to->getNodeId());
	return ptPathSet;
}
void PT_PathSetManager::writePathSetFileHeader()
{
	this->ptPathSetWriter<<"PtPathId,"<<"ptPathSetId,"<<"scenario,"<<"PathTravelTime,"<<"TotalDistanceKms,"<<"PathSize,"
			<<"TotalCost,"<<"Total_In_Vehicle_Travel_Time_Secs,"<<"Total_waiting_time,"<<"Total_walking_time,"
		    <<"Total_Number_of_transfers,"<<"isMinDistance,"<<"isValidPath,"<<"isShortestPath,"<<"isMinInVehicleTravelTimeSecs,"
		    <<"isMinNumberOfTransfers,"<<"isMinWalkingDistance,"<<"isMinTravelOnMrt,"<<"isMinTravelOnBus,"<<"pathset_origin_node,"<<"pathset_dest_node"<<std::endl;
}
void PT_PathSetManager::writePathSetToFile(PT_PathSet &ptPathSet,unsigned int fromNodeId,unsigned int toNodeId)
{
	fileExclusiveWrite.lock();
	for(std::set<PT_Path,cmp_path_vector>::const_iterator itPath=ptPathSet.pathSet.begin();itPath!=ptPathSet.pathSet.end();itPath++)
	{
		this->ptPathSetWriter<<"\""<<itPath->getPtPathId()<<"\""<<","<<"\""<<itPath->getPtPathSetId()<<"\""<<","<<itPath->getScenario()
				<<","<<itPath->getPathTravelTime()<<","<<itPath->getTotalDistanceKms()<<","<<itPath->getPathSize()<<","<<itPath->getTotalCost()<<","
				<<itPath->getTotalInVehicleTravelTimeSecs()<<","<<itPath->getTotalWaitingTimeSecs()<<","<<itPath->getTotalWalkingTimeSecs()<<","
				<<itPath->getTotalNumberOfTransfers()<<","<<itPath->isMinDistance()<<","<<itPath->isValidPath()<<","<<itPath->isShortestPath()<<","
				<<itPath->isMinInVehicleTravelTimeSecs()<<","<<itPath->isMinNumberOfTransfers()
				<<","<<itPath->isMinWalkingDistance()<<","<<itPath->isMinTravelOnMrt()<<","<<itPath->isMinTravelOnBus()<<","<<fromNodeId<<","<<toNodeId<<std::endl;
	}
	fileExclusiveWrite.unlock();
}

void PT_PathSetManager::getLabelingApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet)
{
	for(int i=0;i<labelPoolSize;i++)
	{
		vector<PT_NetworkEdge> path;
		path = StreetDirectory::instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromId,toId,i+LabelingApproach1);
		if(path.size()!=0)
		{
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
			std::stringstream scenario;
			scenario<<LABELLING_APPROACH<<i;
			ptPath.setScenario(scenario.str());
			ptPathSet.pathSet.insert(ptPath);
		}
	}
}

void PT_PathSetManager::getkShortestPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet)
{
	int i=0;
	vector<vector<PT_NetworkEdge> > kShortestPaths;
	int kShortestLevel = ConfigManager::GetInstance().FullConfig().pathSet().publickShortestPathLevel;
	StreetDirectory::instance().getPublicTransitShortestPathImpl()->getKShortestPaths(kShortestLevel,fromId,toId,kShortestPaths);
	for(vector<vector<PT_NetworkEdge> >::iterator itPath=kShortestPaths.begin();itPath!=kShortestPaths.end();itPath++)
	{
		i++;
		PT_Path ptPath(*itPath);
		if(kShortestPaths.begin() == itPath){
			ptPath.setShortestPath(true);
		}
		std::stringstream scenario;
		scenario<<KSHORTEST_PATH<<i;
		ptPath.setScenario(scenario.str());
		ptPathSet.pathSet.insert(ptPath);
	}
}

void PT_PathSetManager::getLinkEliminationApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet)
{
	int i=0;
	vector<PT_NetworkEdge> shortestPath;
	shortestPath = StreetDirectory::instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromId,toId,KshortestPath);

	for(vector<PT_NetworkEdge>::iterator edgeIt=shortestPath.begin();edgeIt!=shortestPath.end();edgeIt++)
	{
		vector<PT_NetworkEdge> path;
		double cost=0;
		std::set<StreetDirectory::PT_EdgeId> blackList = std::set<StreetDirectory::PT_EdgeId>();
		blackList.insert(edgeIt->getEdgeId());
		path=StreetDirectory::instance().getPublicTransitShortestPathImpl()->searchShortestPathWithBlacklist(fromId,toId,blackList,cost);
		if(path.size()!=0)
		{
			i++;
			PT_Path ptPath(path);
			std::stringstream scenario;
			scenario<<LINK_ELIMINATION_APPROACH<<i;
			ptPath.setScenario(scenario.str());
			ptPathSet.pathSet.insert(ptPath);
		}
	}
}

void PT_PathSetManager::getSimulationApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet)
{
	int simulationApproachPoolSize = ConfigManager::GetInstance().FullConfig().pathSet().simulationApproachIterations;
	for(int i=0;i<simulationApproachPoolSize;i++)
	{
		vector<PT_NetworkEdge> path;
		path = StreetDirectory::instance().getPublicTransitShortestPathImpl()->searchShortestPath(fromId,toId,i+SimulationApproach1);
		if(path.size()!=0)
		{
			PT_Path ptPath(path);
			std::stringstream scenario;
			scenario<<SIMULATION_APPROACH<<(i+1);
			ptPath.setScenario(scenario.str());
			ptPathSet.pathSet.insert(ptPath);
		}
	}
}
