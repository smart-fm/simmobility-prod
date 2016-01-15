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
#include "geospatial/network/RoadSegment.hpp"

using namespace sim_mob::aimsun;
using std::string;
using std::set;
using std::map;
using std::pair;

namespace {
class DatabaseLoader : private boost::noncopyable
{
public:
	explicit DatabaseLoader(string const & connectionString);

	static void getCBD_Border(const string & cnn,
			std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > &in,
			std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > & out);
	static void getCBD_Segments(const string & cnn,std::set<const sim_mob::RoadSegment*> & zoneSegments);

	static void getCBD_Nodes(const string& cnn, std::map<unsigned int, const sim_mob::Node*>& nodes);

private:
	soci::session sql_;
};

DatabaseLoader::DatabaseLoader(string const & connectionString)
: sql_(soci::postgresql, connectionString)
{
}

void DatabaseLoader::getCBD_Border(const string & cnn,
		std::set<std::pair<const sim_mob::RoadSegment*,const sim_mob::RoadSegment*> > &in,
		std::set<std::pair<const sim_mob::RoadSegment*,const sim_mob::RoadSegment*> > &out)
{
	/*
	soci::session sql(soci::postgresql, cnn);

	const std::string& inTurningFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
														procedureMappings["restricted_reg_in_turning"];

	soci::rowset<sim_mob::CBD_Pair> rsIn = sql.prepare << std::string("select * from ") + inTurningFunc;
	for (soci::rowset<sim_mob::CBD_Pair>::iterator it = rsIn.begin();it != rsIn.end(); it++)
	{
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itFromSeg(sim_mob::RoadSegment::allSegments.find(it->from_section));
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itToSeg(sim_mob::RoadSegment::allSegments.find(it->to_section));
		if (itFromSeg != sim_mob::RoadSegment::allSegments.end() && itToSeg != sim_mob::RoadSegment::allSegments.end())
		{
			in.insert(std::make_pair(itFromSeg->second, itToSeg->second));
		}
		else
		{
			std::stringstream str("");
			str << "Section ids " << it->from_section << "," << it->to_section
					<< " has no candidate Road Segment among "
					<< sim_mob::RoadSegment::allSegments.size()
					<< " segments\n";
			throw std::runtime_error(str.str());
		}
	}

	const std::string& outTurningFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
															procedureMappings["restricted_reg_out_turning"];

	soci::rowset<sim_mob::CBD_Pair> rsOut = sql.prepare << std::string("select * from ") + outTurningFunc;
	for (soci::rowset<sim_mob::CBD_Pair>::iterator it = rsOut.begin();	it != rsOut.end(); it++)
	{
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itFromSeg(sim_mob::RoadSegment::allSegments.find(it->from_section));
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itToSeg(sim_mob::RoadSegment::allSegments.find(it->to_section));

		if (itFromSeg != sim_mob::RoadSegment::allSegments.end() && itToSeg != sim_mob::RoadSegment::allSegments.end())
		{
			out.insert(std::make_pair(itFromSeg->second, itToSeg->second));
		}
		else
		{
			std::stringstream str("");
			str << "Section ids " << it->from_section << "," << it->to_section
					<< " has no candidate Road Segment among "
					<< sim_mob::RoadSegment::allSegments.size()
					<< " segments\n";
			throw std::runtime_error(str.str());
		}
	}
	*/
}

void DatabaseLoader::getCBD_Segments(const string & cnn, std::set<const sim_mob::RoadSegment*> & zoneSegments)
{
	/*
	soci::session sql(soci::postgresql, cnn);

	const std::string& restrictedRegSegFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
															procedureMappings["restricted_reg_segments"];

	soci::rowset<int> rs = sql.prepare << std::string("select * from ") + restrictedRegSegFunc;
	for (soci::rowset<int>::iterator it = rs.begin();	it != rs.end(); it++)
	{
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itSeg(sim_mob::RoadSegment::allSegments.find(*it));
		if(itSeg != sim_mob::RoadSegment::allSegments.end())
		{
			itSeg->second->CBD = true;
			zoneSegments.insert(itSeg->second);
		}
	}
	*/
}

void DatabaseLoader::getCBD_Nodes(const std::string& cnn, std::map<unsigned int, const sim_mob::Node*>& nodes)
{
	/*
	soci::session sql(soci::postgresql, cnn);

	const std::string& restrictedNodesFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
															procedureMappings["restricted_reg_nodes"];

	soci::rowset<int> rs = sql.prepare << std::string("select * from ") + restrictedNodesFunc;
	for(soci::rowset<int>::iterator it = rs.begin(); it != rs.end(); it++)
	{
		std::map<unsigned int, const sim_mob::Node*>::iterator itNode = sim_mob::Node::allNodes.find((*it));
		if(itNode != sim_mob::Node::allNodes.end())
		{
			itNode->second->CBD = true;
			nodes[itNode->second->getID()] = itNode->second;
		}
	}
	*/
}
} //end anon namespace

void sim_mob::aimsun::Loader::getCBD_Border(
		std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > &in,
		std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > &out)
{
	std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	DatabaseLoader::getCBD_Border(cnn, in, out);
}


void sim_mob::aimsun::Loader::getCBD_Segments(std::set<const sim_mob::RoadSegment*> & zoneSegments)
{
	std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	DatabaseLoader::getCBD_Segments(cnn, zoneSegments);
}

void sim_mob::aimsun::Loader::getCBD_Nodes(std::map<unsigned int, const sim_mob::Node*>& nodes)
{
	std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	DatabaseLoader::getCBD_Nodes(cnn, nodes);
}








