/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Loader.hpp"

#include "soci/soci.h"
#include "soci/postgresql/soci-postgresql.h"

#include "Node.hpp"
#include "Section.hpp"
#include "Turning.hpp"
#include "SOCI_Converters.hpp"


using namespace sim_mob::aimsun;
using std::vector;
using std::map;


namespace {

void LoadNodes(soci::session& sql, map<int, Node>& nodelist)
{
	//Our SQL statement
	soci::rowset<Node> rs = (sql.prepare <<"select * from get_node()");

	//Exectue as a rowset to avoid repeatedly building the query.
	nodelist.clear();
	for (soci::rowset<Node>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		if (nodelist.count(it->id)>0) {
			throw std::runtime_error("Duplicate AIMSUN node.");
		}
		nodelist[it->id] = *it;
	}
}


void LoadSections(soci::session& sql, map<int, Section>& sectionlist, map<int, Node>& nodelist)
{
	//Our SQL statement
	soci::rowset<Section> rs = (sql.prepare <<"select * from get_section()");

	//Exectue as a rowset to avoid repeatedly building the query.
	sectionlist.clear();
	for (soci::rowset<Section>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		if(nodelist.count(it->TMP_FromNodeID)==0 || nodelist.count(it->TMP_ToNodeID)==0) {
			throw std::runtime_error("Invalid From or To node.");
		}

		//Note: Make sure not to resize the Node map after referencing its elements.
		it->fromNode = &nodelist[it->TMP_FromNodeID];
		it->toNode = &nodelist[it->TMP_ToNodeID];
		sectionlist[it->id] = *it;
	}
}


void LoadTurnings(soci::session& sql, map<int, Turning>& turninglist, map<int, Section>& sectionlist)
{
	//Our SQL statement
	soci::rowset<Turning> rs = (sql.prepare <<"select * from get_lane_connector()");

	//Exectue as a rowset to avoid repeatedly building the query.
	turninglist.clear();
	for (soci::rowset<Turning>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		if(sectionlist.count(it->TMP_FromSection)==0 || sectionlist.count(it->TMP_ToSection)==0) {
			throw std::runtime_error("Invalid From or To section.");
		}

		//Note: Make sure not to resize the Section map after referencing its elements.
		it->fromSection = &sectionlist[it->TMP_FromSection];
		it->toSection = &sectionlist[it->TMP_ToSection];
		turninglist[it->id] = *it;
	}
}



void LoadBasicAimsunObjects(sim_mob::RoadNetwork& rn)
{
	//Connect
	//NOTE: Our git repository is private (SSH-only) for now, so just storing the password to the DB here.
	soci::session sql(soci::postgresql, "host=localhost port=5432 dbname=SimMobility_DB user=postgres password=S!Mm0bility");

	//Load all nodes
	map<int, Node> nodelist;
	LoadNodes(sql, nodelist);

	//Load all sections
	map<int, Section> sectionlist;
	LoadSections(sql, sectionlist, nodelist);

	//Load all turnings
	map<int, Turning> turninglist;
	LoadTurnings(sql, turninglist, sectionlist);
}



} //End anon namespace


bool sim_mob::aimsun::Loader::LoadNetwork(sim_mob::RoadNetwork& rn)
{
	//try {
		LoadBasicAimsunObjects(rn);



	/*} catch (std::exception& ex) {
		//TODO: We can provide ex.what() (the message) to the developer once
		//      we decide if our library will throw exceptions, use error codes,
		//      return pair<bool, string>, etc.
		std::cout <<"Error: " <<ex.what() <<std::endl;

		return false;
	}*/
	return true;
}

