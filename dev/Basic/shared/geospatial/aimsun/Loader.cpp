//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Loader.hpp"

#include <map>
#include <set>
#include <vector>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadNetwork.hpp"

using namespace sim_mob::aimsun;
using std::string;
using std::set;
using std::map;
using std::pair;

namespace
{

class DatabaseLoader : private boost::noncopyable
{
public:
	explicit DatabaseLoader(string const &connectionString);

	static void getCBD_Links(const string &cnn, std::set<const sim_mob::Link *> &zoneLinks);

	static void getCBD_Nodes(const string &cnn, std::map<unsigned int, const sim_mob::Node *> &nodes);

private:
	soci::session sql_;
};

DatabaseLoader::DatabaseLoader(string const &connectionString)
: sql_(soci::postgresql, connectionString)
{
}

void DatabaseLoader::getCBD_Links(const string &cnn, std::set<const sim_mob::Link *> &zoneLinks)
{
	soci::session sql(soci::postgresql, cnn);

	const std::string restrictedRegSegFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
															procedureMappings["restricted_reg_links"];

	soci::rowset<int> rs = sql.prepare << std::string("select * from ") + restrictedRegSegFunc;
	const sim_mob::RoadNetwork *rdNetwork = sim_mob::RoadNetwork::getInstance();
	
	for (soci::rowset<int>::iterator it = rs.begin();	it != rs.end(); it++)
	{
		const sim_mob::Link *link = rdNetwork->getById(rdNetwork->getMapOfIdVsLinks(), *it);
		
		if(link != nullptr)
		{			
			zoneLinks.insert(link);
		}
	}
}

void DatabaseLoader::getCBD_Nodes(const std::string &cnn, std::map<unsigned int, const sim_mob::Node *> &nodes)
{
	soci::session sql(soci::postgresql, cnn);

	const std::string restrictedNodesFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
															procedureMappings["restricted_reg_nodes"];

	soci::rowset<int> rs = sql.prepare << std::string("select * from ") + restrictedNodesFunc;
	const sim_mob::RoadNetwork *rdNetwork = sim_mob::RoadNetwork::getInstance();
	
	for(soci::rowset<int>::iterator it = rs.begin(); it != rs.end(); it++)
	{
		const sim_mob::Node *node = rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), *it);
		
		if(node != nullptr)
		{
			nodes[node->getNodeId()] = node;
		}
	}
}
} //end anon namespace

void sim_mob::aimsun::Loader::getCBD_Links(std::set<const sim_mob::Link *> &zoneLinks)
{
	std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	DatabaseLoader::getCBD_Links(cnn, zoneLinks);
}

void sim_mob::aimsun::Loader::getCBD_Nodes(std::map<unsigned int, const sim_mob::Node *> &nodes)
{
	std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	DatabaseLoader::getCBD_Nodes(cnn, nodes);
}

