/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch
#include "simpleconf.hpp"

#include <sstream>

//Make sure our "test" (new) Config variant compiles.
#include "conf/xmlLoader/implementation/conf1-driver.hpp"

//simpleconf.cpp can include GenConfig.h for now (since the entire file will be deleted soon)
#include "GenConfig.h"

#include <tinyxml.h>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <fstream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "geospatial/xmlWriter/boostXmlWriter.hpp"

//Include here (forward-declared earlier) to avoid include-cycles.
#include "entities/PendingEvent.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/BusController.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/signal/Signal.hpp"
#include "entities/FMODController/FMODController.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "util/ReactionTimeDistributions.hpp"
#include "network/CommunicationDataManager.hpp"
#include "password/password.hpp"
#include "logging/Log.hpp"


#include "entities/profile/ProfileBuilder.hpp"
#include "entities/misc/BusSchedule.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/BusStop.hpp"
#include "util/ReactionTimeDistributions.hpp"
#include "util/PassengerDistribution.hpp"

#include "conf/Validate.hpp"
#include "conf/GeneralOutput.hpp"
#include "conf/PrintNetwork.hpp"
#include "conf/LoadAgents.hpp"
#include "conf/LoadNetwork.hpp"

#include "geospatial/xmlLoader/geo10.hpp"

#include "partitions/PartitionManager.hpp"


using std::cout;
using std::endl;
using std::map;
using std::set;
using std::string;
using std::vector;

using namespace sim_mob;


namespace {

//If any Agents specify manual IDs, we must ensure that:
//   * the ID is < startingAutoAgentID
//   * all manual IDs are unique.
//We do this using the Agent constraints struct
struct AgentConstraints {
	int startingAutoAgentID;
	std::set<unsigned int> manualAgentIDs;
};


//Helper sort
bool agent_sort_by_id (Agent* i, Agent* j) { return (i->getId()<j->getId()); }



int getValueInMS(double value, const std::string& units)
{
	//Detect errors
	if (units.empty() || (units!="minutes" && units!="seconds" && units!="ms")) {
		return -1;
	}

	//Reduce to ms
    if (units == "minutes") {
    	value *= 60*1000;
    }
    if (units == "seconds") {
    	value *= 1000;
    }

    //Check for overflow:
    int res = static_cast<int>(value);
    if (static_cast<double>(res) != value) {
    	std::cout <<"NOTE: Rounding value in ms from " <<value <<" to " <<res <<"\n";
    }

    return res;
}


int ReadGranularity(TiXmlHandle& handle, const std::string& granName)
{
	TiXmlElement* node = handle.FirstChild(granName).ToElement();
	if (!node) {
		return -1;
	}

	double value;
	const char* units = node->Attribute("units");
	if (!node->Attribute("value", &value) || !units) {
		return -1;
	}

	return getValueInMS(value, units);
}



int ReadValue(TiXmlHandle& handle, const std::string& propName, int defaultValue)
{
	TiXmlElement* node = handle.FirstChild(propName).ToElement();
	if (!node) {
		return defaultValue;
	}

	int value;
	if (!node->Attribute("value", &value)) {
		return defaultValue;
	}

	return value;
}


void SplitAndAddString(vector<string>& arr, string str)
{
    std::istringstream iss(str);
	std::copy(std::istream_iterator<string>(iss), std::istream_iterator<string>(),
		std::back_inserter<vector<string> >(arr));
}



vector<string> ReadSpaceSepArray(TiXmlHandle& handle, const std::string& attrName)
{
	//Search for this attribute, parse it.
	TiXmlElement* node = handle.ToElement();
	vector<string> res;
	if (node) {
		const char* strArrP = node->Attribute(attrName.c_str());
		if (strArrP) {
			SplitAndAddString(res, strArrP);
		}
	}

	//Done
	return res;
}

string ReadLowercase(TiXmlHandle& handle, const std::string& attrName)
{
	//Search for this attribute, parse it.
	TiXmlElement* node = handle.ToElement();
	string res;
	if (node) {
		const char* strArrP = node->Attribute(attrName.c_str());
		if (strArrP) {
			res = string(strArrP);
			std::transform(res.begin(), res.end(), res.begin(), ::tolower);
		}
	}

	//Done
	return res;
}


////
//// TODO: Eventually, we need to re-write WorkGroup to encapsulate the functionality of "addOrStash()".
////       For now, just make sure that if you add something to all_agents manually, you call "load()" before.
////
void addOrStashEntity(Agent* p, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents)
{
	//Only agents with a start time of zero should start immediately in the all_agents list.
	if (ConfigParams::GetInstance().DynamicDispatchDisabled() || p->getStartTime()==0) {
		p->load(p->getConfigProperties());
		p->clearConfigProperties();
		active_agents.push_back(p);
	} else {
		//Start later.
		pending_agents.push(p);
	}
}



//NOTE: "constraints" are not used here, but they could be (for manual ID specification).
void generateAgentsFromTripChain(std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, AgentConstraints& constraints)
{
	ConfigParams& config = ConfigParams::GetInstance();
	std::map<std::string, vector<TripChainItem*> >& tcs = ConfigParams::GetInstance().getTripChains();
	Print() << "Size of root tripchain container is " << tcs.size() << std::endl;
	//The current agent we are working on.
	Person* person = nullptr;
	std::string trip_mode;
	typedef vector<TripChainItem*>::const_iterator TCVectIt;
	typedef std::map<std::string, vector<TripChainItem*> >::iterator TCMapIt;
	for (TCMapIt it_map=tcs.begin(); it_map!=tcs.end(); it_map++) {
		Print() << "Size of tripchain item for person " << it_map->first << " is : " << it_map->second.size() << std::endl;
		TripChainItem* tc = it_map->second.front();
		if( tc->itemType != TripChainItem::IT_FMODSIM){
			person = new Person("XML_TripChain", config.mutexStategy, it_map->second);
			person->setPersonCharacteristics();
			addOrStashEntity(person, active_agents, pending_agents);
			//Reset for the next (possible) Agent
			person = nullptr;
		}
		else {
			//insert to FMOD controller so that collection of requests
			if (sim_mob::FMOD::FMODController::InstanceExists()) {
				sim_mob::FMOD::FMODController::Instance()->InsertFMODItems(it_map->first, tc);
			} else {
				Warn() <<"Skipping FMOD agent; FMOD controller is not active.\n";
			}
		}
	}//outer for loop(map)

	if( sim_mob::FMOD::FMODController::InstanceExists() ) {
		sim_mob::FMOD::FMODController::Instance()->Initialize();
	}
}

bool isAndroidClientEnabled(TiXmlHandle& handle)
{
	TiXmlElement* node = handle.FirstChild("config").FirstChild("system").FirstChild("simulation").FirstChild("communication").FirstChild("android_testbed").ToElement();
	ConfigParams::GetInstance().androidClientEnabled = false;
	std::string androidYesNo = std::string(node->Attribute("enabled"));
	if (androidYesNo == "yes") {
		return true;
	}
	return false;
}

bool loadXMLAgents(TiXmlDocument& document, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, const std::string& agentType, AgentConstraints& constraints)
{
	ConfigParams& config = ConfigParams::GetInstance();

	//At the moment, we only load *Roles* from the config file. So, check if this is a valid role.
	// This will only generate an error if someone actually tries to load an agent of this type.
	const RoleFactory& rf = config.getRoleFactory();
	bool knownRole = rf.isKnownRole(agentType);

	//Attempt to load either "agentType"+s or "agentType"+es (drivers, buses).
	// This allows ungrammatical terms like "driveres", but it's probably better than being too restrictive.
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild(agentType+"s").FirstChild(agentType).ToElement();
	if (!node) {
		node = handle.FirstChild("config").FirstChild(agentType+"es").FirstChild(agentType).ToElement();
	}

	//If at least one elemnt of an unknown type exists, it's an error.
	if (node && !knownRole) {
		std::cout <<"Unexpected agent type: " <<agentType <<endl;
		return false;
	}


	//Loop through all agents of this type. If this node doesn't exist in the first place,
	// the loop immediately exits. (An empty "drivers" block is allowed.)
	//  (This is another reason why we have to be permissive for incorrect spellings.)
	for (;node;node=node->NextSiblingElement()) {
		//Keep track of the properties we have found.
		map<string, string> props;

		{
			//Loop through attributes, ensuring that all required attributes are found.
			map<string, bool> propLookup = rf.getRequiredAttributes(agentType);
			size_t propsLeft = propLookup.size();
			for (TiXmlAttribute* attr=node->FirstAttribute(); attr; attr=attr->Next()) {
				//Save
				props[attr->NameTStr()] = attr->ValueStr();

				//If this is a required attribute, set it and decrease our count flag.
				map<string, bool>::iterator nameKey = propLookup.find(attr->NameTStr());
				if (nameKey!=propLookup.end() && (!nameKey->second)) {
					//Count
					propsLeft--;
					nameKey->second = true;
				}
			}

			//Are we missing anything?
			if (propsLeft > 0) {
				std::stringstream msg;
				msg <<"Agent type (" <<agentType <<") missing required properties:";
				string comma = "";
				for (map<string, bool>::iterator it=propLookup.begin(); it!=propLookup.end(); it++) {
					if (!it->second) {
						msg <<comma <<" " <<it->first;
						comma = ",";
					}
				}
				throw std::runtime_error(msg.str().c_str());
			}
		}

		//We should generate the Agent's ID here (since otherwise Agents
		//  will have seemingly random IDs that do not reflect their order in the config file).
		//It is generally preferred to use the automatic IDs, but if a manual ID is specified we
		//  must deal with it here.
		int manualID = -1;
		map<string, string>::iterator propIt = props.find("id");
		if (propIt != props.end()) {
			//Convert the ID to an integer.
			std::istringstream(propIt->second) >> manualID;

			//Simple constraint check.
			if (manualID<0 || manualID>=constraints.startingAutoAgentID) {
				throw std::runtime_error("Manual ID must be within the bounds specified in the config file.");
			}

			//Ensure agents are created with unique IDs
			if (constraints.manualAgentIDs.count(manualID)>0) {
				std::stringstream msg;
				msg <<"Duplicate manual ID: " <<manualID;
				throw std::runtime_error(msg.str().c_str());
			}

			//Mark it, save it, remove it from the list
			constraints.manualAgentIDs.insert(manualID);
			props.erase(propIt);
		}

		//We also need to retrieve and properly set this agent's start time, so that it can be
		//  delayed until this time has arrived.
		int startTime = -1;
		propIt = props.find("time");
		if (propIt != props.end()) {
			//Convert this time to an integer.
			std::istringstream(propIt->second) >> startTime;

			//Remove it from the list.
			props.erase(propIt);
		}

		//Make sure we retrieved a valid start time.
		if (startTime<0) {
			throw std::runtime_error("Start time can't be negative.");
		}

		//Finally, set the "#mode" flag in the configProps array.
		// (XML can't have # inside tag names, so this will never be overwritten)
		//
		//TODO: We should just be able to save "driver" and "pedestrian", but we are
		//      using different vocabulary for modes and roles. We need to change this.
		props["#mode"] = (agentType=="driver"?"Car":(agentType=="pedestrian"?"Walk":"Unknown"));
		if (agentType == "busdriver")
			props["#mode"] = "Bus";
		if (agentType == "passenger")
			props["#mode"] = "BusTravel";

		//Create the Person agent with that given ID (or an auto-generated one)
		Person* agent = new Person("XML_Def", config.mutexStategy, manualID);
		agent->setConfigProperties(props);
		agent->setStartTime(startTime);

		//Add it or stash it
		addOrStashEntity(agent, active_agents, pending_agents);
	}

	return true;
}

bool loadXMLFMODController(TiXmlDocument& document)
{
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild("fmodcontroller").ToElement();
	if(!node)	{
		return true;
	}

	std::string swit = node->Attribute("switch");
	if( swit != "true"){
		return false;
	}

	TiXmlNode* node1 = node->FirstChild("ipaddress");
	if(node1 == nullptr)	{
		return false;
	}

	TiXmlNode* node2 = node1->NextSibling();
	if(node2 == nullptr)	{
		return false;
	}

	TiXmlNode* node3 = node2->NextSibling();
	if(node3 == nullptr)	{
		return false;
	}

	TiXmlNode* node4 = node3->NextSibling();
	if(node4 == nullptr)	{
		return false;
	}

	TiXmlNode* node5 = node4->NextSibling();
	if(node5 == nullptr)   {
		return false;
	}

	std::string ipAddress = node1->FirstChild()->Value();
	int port = atoi( node2->FirstChild()->Value() );
	int updateTiming = atoi( node3->FirstChild()->Value() );
	std::string mapFile = node4->FirstChild()->Value();
	int blockingTime = atoi( node5->FirstChild()->Value() );

	sim_mob::FMOD::FMODController::RegisterController(-1, sim_mob::ConfigParams::GetInstance().mutexStategy);
	sim_mob::FMOD::FMODController::Instance()->Settings(ipAddress, port, updateTiming, mapFile, blockingTime);
	sim_mob::FMOD::FMODController::Instance()->ConnectFMODService();

	return true;
}

bool loadXMLBusControllers(TiXmlDocument& document, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents)
{
	//Retrieve the list of buscontrollers
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild("buscontrollers").FirstChild("buscontroller").ToElement();

	//Loop through all agents of this type
	for (;node;node=node->NextSiblingElement()) {
		//Keep track of the properties we have found.
		map<string, string> props;// temporary use

		char const * timeAttr = node->Attribute("time");

        try {
            int timeValue = boost::lexical_cast<int>(timeAttr);
            if (timeValue < 0) {
                std::cerr << "buscontrollers must have positive time attributes in the config file." << std::endl;
                return false;
            }
            props["time"] = timeAttr;// I dont know how to set props for the buscontroller, it seems no use;
            sim_mob::BusController::RegisterNewBusController(timeValue, sim_mob::ConfigParams::GetInstance().mutexStategy);
        } catch (boost::bad_lexical_cast &) {
        	Warn() << "catch the loop error try!\n"
        		   << "buscontrollers must have 'time' attributes with numerical values in the config file." << std::endl;
            return false;
        }
	}
	return true;
}

bool loadXMLSignals(TiXmlDocument& document, const std::string& signalKeyID)
{
	//Quick check.
	if (signalKeyID!="signal") {
		return false;
	}

	//Build the expression dynamically.
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild(signalKeyID+"s").FirstChild(signalKeyID).ToElement();
	if (!node) {
		//Signals are optional
		return true;
	}

	StreetDirectory& streetDirectory = StreetDirectory::instance();

	//Loop through all agents of this type
	for (;node;node=node->NextSiblingElement()) {
            char const * xPosAttr = node->Attribute("xpos");
            char const * yPosAttr = node->Attribute("ypos");
            if (0 == xPosAttr || 0 == yPosAttr)
            {

                std::cerr << "signals must have 'xpos', and 'ypos' attributes in the config file." << std::endl;
                return false;
            }

            try
            {
                int xpos = boost::lexical_cast<int>(xPosAttr);
                int ypos = boost::lexical_cast<int>(yPosAttr);

                const Point2D pt(xpos, ypos);
                Node* road_node = ConfigParams::GetInstance().getNetwork().locateNode(pt, true);
                if (0 == road_node)
                {
                    std::cerr << "xpos=\"" << xPosAttr << "\" and ypos=\"" << yPosAttr
                              << "\" are not suitable attributes for Signal because there is no node there; correct the config file."
                              << std::endl; 
                    continue;
                }

                // See the comments in createSignals() in geospatial/aimsun/Loader.cpp.
                // At some point in the future, this function loadXMLSignals() will be removed
                // in its entirety, not just the following code fragment.
                std::set<Link const *> links;
                if (MultiNode const * multi_node = dynamic_cast<MultiNode const *>(road_node))
                {
                    std::set<RoadSegment*> const & roads = multi_node->getRoadSegments();
                    std::set<RoadSegment*>::const_iterator iter;
                    for (iter = roads.begin(); iter != roads.end(); ++iter)
                    {
                        RoadSegment const * road = *iter;
                        links.insert(road->getLink());
                    }
                }
                if (links.size() != 4)
                {
                    std::cerr << "the multi-node at " << pt << " does not have 4 links; "
                              << "no signal will be created here." << std::endl;
                    continue;
                }
                Signal const * signal = streetDirectory.signalAt(*road_node);
                if (signal)
                {
                    std::cout << "signal at node(" << xpos << ", " << ypos << ") already exists; "
                              << "skipping this config file entry" << std::endl;
                }
                else
                {
                	std::cout << "signal at node(" << xpos << ", " << ypos << ") was not found; No more action will be taken\n ";
//                    // The following call will create and register the signal with the
//                    // street-directory.
//                	std::cout << "register signal again!" << std::endl;
//                    Signal::signalAt(*road_node, ConfigParams::GetInstance().mutexStategy);
                }
            }
            catch (boost::bad_lexical_cast &)
            {
            	Warn() << "catch the loop error try!\n"
            		   << "signals must have 'id', 'xpos', and 'ypos' attributes with numerical values in the config file." << std::endl;
                return false;
            }
	}
	return true;
}



bool LoadDatabaseDetails(TiXmlElement& parentElem, string& connectionString, map<string, string>& storedProcedures)
{
	TiXmlHandle handle(&parentElem);
	TiXmlElement* elem = handle.FirstChild("connection").FirstChild("param").ToElement();
	if (!elem) {
		return false;
	}

	//Loop through each parameter; add it to the connection string
	for (;elem;elem=elem->NextSiblingElement()) {
		const char* name = elem->Attribute("name");
		const char* value = elem->Attribute("value");
		if (!name || !value) {
			return false;
		}
		if(strcmp(name,"password") == 0) continue;
		string pair = (connectionString.empty()?"":" ") + string(name) + "=" + string(value);
		connectionString += pair;
	}
	connectionString += (connectionString.empty()?"":" ") + string("password = ") + sim_mob::simple_password::load(string());

	//Now, load the stored procedure mappings
	elem = handle.FirstChild("mappings").FirstChild().ToElement();
	if (!elem) {
		return false;
	}

	//Loop through and add them.
	for (;elem;elem=elem->NextSiblingElement()) {
		string name = elem->ValueStr();
		const char* value = elem->Attribute("procedure");
		if (!value) {

			return false;
		}
		if (storedProcedures.count(name)>0) {
			return false;
		}

		storedProcedures[name] = string(value);
	}

	//Done; we'll check the storedProcedures in detail later.
	return true;
}


struct Sorter {
  bool operator() (const sim_mob::RoadSegment* a,const sim_mob::RoadSegment* b)
  {
	  bool result = (a->getSegmentID() < b->getSegmentID());
	  return result;
  }
  bool operator() (const sim_mob::Crossing* a,const sim_mob::Crossing* b)
  {
	  bool result = (a->getRoadItemID() < b->getRoadItemID());
	  return result;
  }
  bool operator() (sim_mob::Lane* a,sim_mob::Lane* b)
  {
	  return ((a->getRoadSegment()->getLink()->getLinkId() < b->getRoadSegment()->getLink()->getLinkId()) && (a->getRoadSegment()->getSegmentID() < b->getRoadSegment()->getSegmentID()) && (a->getLaneID() < b->getLaneID()));
  }
  bool operator() (const sim_mob::BusStop* a,const sim_mob::BusStop* b)
  {
	  return (a->getRoadItemID() < b->getRoadItemID());
  }
  bool operator() ( const sim_mob::LaneConnector * c,  const sim_mob::LaneConnector * d) const
  {
	  if(!(c && d))
	  {
		  std::cout << "A lane connector is null\n";
		  return false;
	  }

	  const sim_mob::Lane* a = (c->getLaneFrom());
	  const unsigned int  aa = a->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  aaa = a->getRoadSegment()->getSegmentID();
	  const unsigned int  aaaa = a->getLaneID() ;

	  const sim_mob::Lane* b = (d->getLaneFrom());
	  const unsigned int  bb = b->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  bbb = b->getRoadSegment()->getSegmentID();
	  const unsigned int  bbbb = b->getLaneID() ;
	  ///////////////////////////////////////////////////////
	  const sim_mob::Lane* a1 = (c->getLaneTo());
	  const unsigned int  aa1 = a1->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  aaa1 = a1->getRoadSegment()->getSegmentID();
	  const unsigned int  aaaa1 = a1->getLaneID() ;

	  const sim_mob::Lane* b1 = (d->getLaneTo());
	  const unsigned int  bb1 = b1->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  bbb1 = b1->getRoadSegment()->getSegmentID();
	  const unsigned int  bbbb1 = b1->getLaneID() ;

	  if(!(a && b))
	  {
		  std::cout << "A lane from is null\n";
		  return false;
	  }
	  bool result = std::make_pair( aa, std::make_pair( aaa, std::make_pair(aaaa, std::make_pair( aa1, std::make_pair( aaa1, aaaa1 ) ))))
	        <
	        std::make_pair( bb, std::make_pair( bbb, std::make_pair(bbbb, std::make_pair( bb1, std::make_pair( bbb1, bbbb1 ) ))));

		  return result;
  }
} sorter_;

void PrintDB_NetworkToFile(const std::string& fileName)
{
	//Short-circuit if output is disabled.
	if (ConfigParams::GetInstance().OutputDisabled()) {
		return;
	}

	//Else, open the file.
	std::ofstream out(fileName.c_str());

	//Save RoadSegments/Connectors to make output simpler
	std::set<const RoadSegment*,Sorter> cachedSegments;
	std::set<LaneConnector*,Sorter> cachedConnectors;

	//Initial message
	const RoadNetwork& rn = ConfigParams::GetInstance().getNetwork();
	out <<"Printing node network" <<endl;
	out <<"NOTE: All IDs in this section are consistent for THIS simulation run, but will change if you run the simulation again." <<endl;

	//Print some properties of the simulation itself
	out <<"(\"simulation\", 0, 0, {";
	out <<"\"frame-time-ms\":\"" <<ConfigParams::GetInstance().baseGranMS <<"\",";
	out <<"})" <<endl;

	sim_mob::Signal::all_signals_const_Iterator it;
	for (it = sim_mob::Signal::all_signals_.begin(); it!= sim_mob::Signal::all_signals_.end(); it++)
	{
		//Print the Signal representation.
		out <<(*it)->toString() <<endl;
	}


	//Print nodes first
	for (set<UniNode*>::const_iterator it=rn.getUniNodes().begin(); it!=rn.getUniNodes().end(); it++) {
		out <<"(\"uni-node\", 0, " <<*it <<", {";
		out <<"\"xPos\":\"" <<(*it)->location.getX() <<"\",";
		out <<"\"yPos\":\"" <<(*it)->location.getY() <<"\",";
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			out <<(*it)->originalDB_ID.getLogItem();
		}
		out <<"})" <<endl;

		//
		if (ConfigParams::GetInstance().InteractiveMode()) {
			std::ostringstream stream;
			stream<<"(\"uni-node\", 0, " <<*it <<", {";
			stream<<"\"xPos\":\"" <<(*it)->location.getX() <<"\",";
			stream<<"\"yPos\":\"" <<(*it)->location.getY() <<"\",";
			if (!(*it)->originalDB_ID.getLogItem().empty()) {
				stream<<(*it)->originalDB_ID.getLogItem();
					}
			stream<<"})";
			std::string s=stream.str();
			ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(s);
		}

		//Cache all segments
		vector<const RoadSegment*> segs = (*it)->getRoadSegments();
		for (vector<const RoadSegment*>::const_iterator i2=segs.begin(); i2!=segs.end(); ++i2) {
			cachedSegments.insert(*i2);
		}
	}

	for (vector<MultiNode*>::const_iterator it=rn.getNodes().begin(); it!=rn.getNodes().end(); it++) {
		out <<"(\"multi-node\", 0, " <<*it <<", {";
		out <<"\"xPos\":\"" <<(*it)->location.getX() <<"\",";
		out <<"\"yPos\":\"" <<(*it)->location.getY() <<"\",";
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			out <<(*it)->originalDB_ID.getLogItem();
		}
		out <<"})" <<endl;

		if (ConfigParams::GetInstance().InteractiveMode()) {
			std::ostringstream stream;
			stream<<"(\"multi-node\", 0, " <<*it <<", {";
			stream<<"\"xPos\":\"" <<(*it)->location.getX() <<"\",";
			stream<<"\"yPos\":\"" <<(*it)->location.getY() <<"\",";
			if (!(*it)->originalDB_ID.getLogItem().empty()) {
				stream<<(*it)->originalDB_ID.getLogItem();
					}
			stream<<"})";
			std::string s=stream.str();
			ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(s);
		}

		//NOTE: This is temporary; later we'll ensure that the RoadNetwork only stores Intersections,
		//      and RoadSegments will have to be extracted.
		const Intersection* nodeInt = static_cast<const Intersection*>((*it));

		//Cache all segments
		for (set<RoadSegment*>::const_iterator i2=nodeInt->getRoadSegments().begin(); i2!=nodeInt->getRoadSegments().end(); ++i2) {
			cachedSegments.insert(*i2);
		}

		//Now cache all lane connectors at this node
		for (set<RoadSegment*>::iterator rsIt=nodeInt->getRoadSegments().begin(); rsIt!=nodeInt->getRoadSegments().end(); rsIt++) {
			if (nodeInt->hasOutgoingLanes(*rsIt)) {
				for (set<LaneConnector*>::iterator i2=nodeInt->getOutgoingLanes(*rsIt).begin(); i2!=nodeInt->getOutgoingLanes(*rsIt).end(); i2++) {
					//Cache the connector
					cachedConnectors.insert(*i2);
				}
			}
		}
	}

	//Links can go next.
	for (vector<Link*>::const_iterator it=rn.getLinks().begin(); it!=rn.getLinks().end(); it++) {
		out <<"(\"link\", 0, " <<*it <<", {";
		out <<"\"road-name\":\"" <<(*it)->roadName <<"\",";
		out <<"\"start-node\":\"" <<(*it)->getStart() <<"\",";
		out <<"\"end-node\":\"" <<(*it)->getEnd() <<"\",";
		out <<"\"fwd-path\":\"[";
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath().begin(); segIt!=(*it)->getPath().end(); segIt++) {
			out <<*segIt <<",";
		}
		out <<"]\",";
		out <<"})" <<endl;

		if (ConfigParams::GetInstance().InteractiveMode()) {
			std::ostringstream stream;
			stream<<"(\"link\", 0, " <<*it <<", {";
			stream<<"\"road-name\":\"" <<(*it)->roadName <<"\",";
			stream<<"\"start-node\":\"" <<(*it)->getStart() <<"\",";
			stream<<"\"end-node\":\"" <<(*it)->getEnd() <<"\",";
			stream<<"\"fwd-path\":\"[";
			for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath().begin(); segIt!=(*it)->getPath().end(); segIt++) {
				stream<<*segIt <<",";
			}
			stream<<"]\",";
			stream<<"})";
			std::string s=stream.str();
			ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(s);
		}
	}

	//Now print all Segments
	std::set<const Crossing*,Sorter> cachedCrossings;
	std::set<const BusStop*,Sorter> cachedBusStops;
		for (std::set<const RoadSegment*>::const_iterator it=cachedSegments.begin(); it!=cachedSegments.end(); it++) {
		out <<"(\"road-segment\", 0, " <<*it <<", {";
		out <<"\"parent-link\":\"" <<(*it)->getLink() <<"\",";
		out <<"\"max-speed\":\"" <<(*it)->maxSpeed <<"\",";
		out <<"\"width\":\"" <<(*it)->width <<"\",";
		out <<"\"lanes\":\"" <<(*it)->getLanes().size() <<"\",";
		out <<"\"from-node\":\"" <<(*it)->getStart() <<"\",";
		out <<"\"to-node\":\"" <<(*it)->getEnd() <<"\",";
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			out <<(*it)->originalDB_ID.getLogItem();
		}
		out <<"})" <<endl;

		if (!(*it)->polyline.empty()) {
			out <<"(\"polyline\", 0, " <<&((*it)->polyline) <<", {";
			out <<"\"parent-segment\":\"" <<*it <<"\",";
			out <<"\"points\":\"[";
			for (vector<Point2D>::const_iterator ptIt=(*it)->polyline.begin(); ptIt!=(*it)->polyline.end(); ptIt++) {
				out <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
			}
			out <<"]\",";
			out <<"})" <<endl;
		}

		if (ConfigParams::GetInstance().InteractiveMode()) {
			std::ostringstream stream;
			stream<<"(\"road-segment\", 0, " <<*it <<", {";
			stream<<"\"parent-link\":\"" <<(*it)->getLink() <<"\",";
			stream<<"\"max-speed\":\"" <<(*it)->maxSpeed <<"\",";
			stream<<"\"width\":\"" <<(*it)->width <<"\",";
			stream<<"\"lanes\":\"" <<(*it)->getLanes().size() <<"\",";
			stream<<"\"from-node\":\"" <<(*it)->getStart() <<"\",";
			stream<<"\"to-node\":\"" <<(*it)->getEnd() <<"\",";
			if (!(*it)->originalDB_ID.getLogItem().empty()) {
				stream<<(*it)->originalDB_ID.getLogItem();
			}
			stream<<"})";
			std::string s1=stream.str();
			ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(s1);

			std::ostringstream stream2;
			if (!(*it)->polyline.empty()) {
				stream2<<"(\"polyline\", 0, " <<&((*it)->polyline) <<", {";
				stream2<<"\"parent-segment\":\"" <<*it <<"\",";
				stream2<<"\"points\":\"[";
				for (vector<Point2D>::const_iterator ptIt=(*it)->polyline.begin(); ptIt!=(*it)->polyline.end(); ptIt++) {
					stream2<<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
				}
				stream2<<"]\",";
				stream2<<"})";
			}
			std::string ss=stream2.str();
			ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(ss);
		}

		const std::map<centimeter_t, const RoadItem*>& obstacles = (*it)->obstacles;
		for(std::map<centimeter_t, const RoadItem*>::const_iterator obsIt = obstacles.begin(); obsIt != obstacles.end(); ++obsIt) {
			//Save BusStops for later.
			const BusStop* resBS = dynamic_cast<const BusStop*>(obsIt->second);
			if (resBS) {
				cachedBusStops.insert(resBS);
			}

			//Save crossings for later.
			const Crossing* resC = dynamic_cast<const Crossing*>(obsIt->second);
			if (resC) {
				cachedCrossings.insert(resC);
			}
		}

		//Save Lane info for later
		//NOTE: For now this relies on somewhat sketchy behavior, which is why we output a "tmp-*"
		//      flag. Once we add auto-polyline generation, that tmp- output will be meaningless
		//      and we can switch to a full "lane" output type.
		std::stringstream laneBuffer; //Put it in its own buffer since getLanePolyline() can throw.
		laneBuffer <<"(\"lane\", 0, " <<&((*it)->getLanes()) <<", {";
		laneBuffer <<"\"parent-segment\":\"" <<*it <<"\",";
		for (size_t laneID=0; laneID <= (*it)->getLanes().size(); laneID++) {
			const vector<Point2D>& points =(*it)->laneEdgePolylines_cached[laneID];
			laneBuffer <<"\"lane-" <<laneID /*(*it)->getLanes()[laneID]*/<<"\":\"[";
			for (vector<Point2D>::const_iterator ptIt=points.begin(); ptIt!=points.end(); ptIt++) {
				laneBuffer <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
			}
			laneBuffer <<"]\",";

			if (laneID<(*it)->getLanes().size() && (*it)->getLanes()[laneID]->is_pedestrian_lane()) {
				laneBuffer <<"\"line-" <<laneID <<"is-sidewalk\":\"true\",";
				//debug
				if(laneID != 0 && laneID <(*it)->getLanes().size())
				{
					int i = 0;
					i++;
				}
				//debug ends
			}

		}

		if (ConfigParams::GetInstance().InteractiveMode()) {
			std::stringstream stream1;
			stream1 <<"(\"lane\", 0, " <<&((*it)->getLanes()) <<", {";
			stream1 <<"\"parent-segment\":\"" <<*it <<"\",";
			for (size_t laneID=0; laneID <= (*it)->getLanes().size(); laneID++) {
				const vector<Point2D>& points =(*it)->laneEdgePolylines_cached[laneID];
				stream1 <<"\"lane-" <<laneID /*(*it)->getLanes()[laneID]*/<<"\":\"[";
				for (vector<Point2D>::const_iterator ptIt=points.begin(); ptIt!=points.end(); ptIt++) {
					stream1 <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
				}
				stream1 <<"]\",";

				if (laneID<(*it)->getLanes().size() && (*it)->getLanes()[laneID]->is_pedestrian_lane()) {
					stream1 <<"\"line-" <<laneID <<"is-sidewalk\":\"true\",";
				}
			}
			stream1<<"})";
			string s = stream1.str();
			ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(s);
		}

		laneBuffer <<"})" <<endl;
		out <<laneBuffer.str();

	}

	//Crossings are part of Segments
	for (std::set<const Crossing*>::iterator it=cachedCrossings.begin(); it!=cachedCrossings.end(); it++) {
		out <<"(\"crossing\", 0, " <<*it <<", {";
		out <<"\"near-1\":\"" <<(*it)->nearLine.first.getX() <<"," <<(*it)->nearLine.first.getY() <<"\",";
		out <<"\"near-2\":\"" <<(*it)->nearLine.second.getX() <<"," <<(*it)->nearLine.second.getY() <<"\",";
		out <<"\"far-1\":\"" <<(*it)->farLine.first.getX() <<"," <<(*it)->farLine.first.getY() <<"\",";
		out <<"\"far-2\":\"" <<(*it)->farLine.second.getX() <<"," <<(*it)->farLine.second.getY() <<"\",";
		out <<"})" <<endl;

		if (ConfigParams::GetInstance().InteractiveMode()) {
			std::ostringstream stream;
			stream<<"(\"crossing\", 0, " <<*it <<", {";
			stream<<"\"near-1\":\"" <<(*it)->nearLine.first.getX() <<"," <<(*it)->nearLine.first.getY() <<"\",";
			stream<<"\"near-2\":\"" <<(*it)->nearLine.second.getX() <<"," <<(*it)->nearLine.second.getY() <<"\",";
			stream<<"\"far-1\":\"" <<(*it)->farLine.first.getX() <<"," <<(*it)->farLine.first.getY() <<"\",";
			stream<<"\"far-2\":\"" <<(*it)->farLine.second.getX() <<"," <<(*it)->farLine.second.getY() <<"\",";
			stream<<"})";
			string s = stream.str();
			ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(s);
		}
	}

	//Bus Stops are part of Segments
	for (std::set<const BusStop*>::iterator it = cachedBusStops.begin(); it != cachedBusStops.end(); it++) {
		//Assumptions about the size of a Bus Stop
		const centimeter_t length = 400;
		const centimeter_t width  = 250;

		//Use the magnitude of the parent segment to set the Bus Stop's direction and extension.
		const BusStop* bs = *it;

		//std::cout <<"Bust stop: " <<bs <<" id: " <<bs->id <<std::endl;

		Point2D dir;
		{
			const Node* start = 0;
			if(bs && bs->getParentSegment() )
				start = bs->getParentSegment()->getStart();

			const Node* end = 0;
			if(bs && bs->getParentSegment() )
				end = bs->getParentSegment()->getEnd();

			if( start && end )
				dir = Point2D(start->location.getX()-end->location.getX(),start->location.getY()-end->location.getY());
			else
				continue;
		}

		//Get a vector that is at the "lower-left" point of a bus stop, facing "up" and "right"
		DynamicVector vec(bs->xPos, bs->yPos, bs->xPos+dir.getX(), bs->yPos+dir.getY());
		vec.scaleVectTo(length/2).translateVect().flipRight();
		vec.scaleVectTo(width/2).translateVect().flipRight();

		//Now, create a "near" vector and a "far" vector
		DynamicVector near(vec);
		near.scaleVectTo(length);
		DynamicVector far(vec);
		far.flipRight().scaleVectTo(width).translateVect();
		far.flipLeft().scaleVectTo(length);

		//Save it.
		out <<"(\"busstop\", 0, " <<bs <<", {";
		out <<"\"near-1\":\"" <<near.getX()    <<"," <<near.getY()    <<"\",";
		out <<"\"near-2\":\"" <<near.getEndX() <<"," <<near.getEndY() <<"\",";
		out <<"\"far-1\":\""  <<far.getX()     <<"," <<far.getY()     <<"\",";
		out <<"\"far-2\":\""  <<far.getEndX()  <<"," <<far.getEndY()  <<"\",";
		out <<"})" <<std::endl;

		//Old code; this just fakes the bus stop at a 40 degree angle.
		//TODO: Remove this code once we verify that the above code is better.
		/*out <<"(\"busstop\", 0, " <<*it <<", {";
		double x = (*it)->xPos;
		double y = (*it)->yPos;
		int angle = 40;
		double length = 400;
		double width = 250;
		double theta = atan(width / length);
		double phi = M_PI * angle / 180;
		double diagonal_half = (sqrt(length * length + width * width)) / 2;

		//TODO: It looks like this can be easily replaced with DynamicVectors. Should be both faster and simpler.
		double x1d = x + diagonal_half * cos(phi + theta);
		double y1d = y + diagonal_half * sin(phi + theta);
		double x2d = x + diagonal_half * cos(M_PI + phi - theta);
		double y2d = y + diagonal_half * sin(M_PI + phi - theta);
		double x3d = x + diagonal_half * cos(M_PI + phi + theta);
		double y3d = y + diagonal_half * sin(M_PI + phi + theta);
		double x4d = x + diagonal_half * cos(phi - theta);
		double y4d = y + diagonal_half * sin(phi - theta);

		out <<"\"near-1\":\""<<std::setprecision(8)<<x<<","<<y<<"\",";
		out <<"\"near-2\":\""<<x2d<<","<<y2d<<"\",";
		out <<"\"far-1\":\""<<x3d<<","<<y3d<<"\",";
		out <<"\"far-2\":\""<<x4d<<","<<y4d<<"\",";
		out <<"})" <<endl;*/

		//TODO: This code might be better, but it should still be replaced by the above code.
		/*
		if (ConfigParams::GetInstance().InteractiveMode()) {
			std::ostringstream stream;
			stream<<"(\"busstop\", 0, " <<*it <<", {";
			stream<<"\"near-1\":\""<<std::setprecision(8)<<x<<","<<y<<"\",";
			stream<<"\"near-2\":\""<<x2d<<","<<y2d<<"\",";
			stream<<"\"far-1\":\""<<x3d<<","<<y3d<<"\",";
			stream<<"\"far-2\":\""<<x4d<<","<<y4d<<"\",";
			stream<<"})";
			string s = stream.str();
			ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(s);
		}*/
	}


	//Now print all Connectors
	for (std::set<LaneConnector*>::const_iterator it=cachedConnectors.begin(); it!=cachedConnectors.end(); it++) {
		//Retrieve relevant information
		RoadSegment* fromSeg = (*it)->getLaneFrom()->getRoadSegment();
		unsigned int fromLane = std::distance(fromSeg->getLanes().begin(), std::find(fromSeg->getLanes().begin(), fromSeg->getLanes().end(),(*it)->getLaneFrom()));
		RoadSegment* toSeg = (*it)->getLaneTo()->getRoadSegment();
		unsigned int toLane = std::distance(toSeg->getLanes().begin(), std::find(toSeg->getLanes().begin(), toSeg->getLanes().end(),(*it)->getLaneTo()));

		//Output
		out <<"(\"lane-connector\", 0, " <<*it <<", {";
		out <<"\"from-segment\":\"" <<fromSeg <<"\",";
		out <<"\"from-lane\":\"" <<fromLane <<"\",";
		out <<"\"to-segment\":\"" <<toSeg <<"\",";
		out <<"\"to-lane\":\"" <<toLane <<"\",";
		out <<"})" <<endl;

		if (ConfigParams::GetInstance().InteractiveMode()) {
			std::ostringstream stream;
			stream<<"(\"lane-connector\", 0, " <<*it <<", {";
			stream<<"\"from-segment\":\"" <<fromSeg <<"\",";
			stream<<"\"from-lane\":\"" <<fromLane <<"\",";
			stream<<"\"to-segment\":\"" <<toSeg <<"\",";
			stream<<"\"to-lane\":\"" <<toLane <<"\",";
			stream<<"})";
			string s = stream.str();
			ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(s);
		}
	}

	if (ConfigParams::GetInstance().InteractiveMode()) {
		string end = "END";
		ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(end);
	}

	//Print the StreetDirectory graphs.
	StreetDirectory::instance().printDrivingGraph(out);
	StreetDirectory::instance().printWalkingGraph(out);

	//Required for the visualizer
	out <<"ROADNETWORK_DONE" <<endl;
}



//Returns the error message, or an empty string if no error.
void loadXMLConf(TiXmlDocument& document, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, ProfileBuilder* prof)
{
	//Save granularities: system
	TiXmlHandle handle(&document);
	handle = handle.FirstChild("config").FirstChild("system").FirstChild("simulation");
	int baseGran = ReadGranularity(handle, "base_granularity");
	int totalRuntime = ReadGranularity(handle, "total_runtime");
	int totalWarmup = ReadGranularity(handle, "total_warmup");
	//Save reaction time parameters
	int distributionType1, distributionType2;
    int mean1, mean2;
    int standardDev1, standardDev2;
	handle.FirstChild("reacTime_distributionType1").ToElement()->Attribute("value",&distributionType1);
	handle.FirstChild("reacTime_distributionType2").ToElement()->Attribute("value",&distributionType2);
	handle.FirstChild("reacTime_mean1").ToElement()->Attribute("value",&mean1);
	handle.FirstChild("reacTime_mean2").ToElement()->Attribute("value",&mean2);
	handle.FirstChild("reacTime_standardDev1").ToElement()->Attribute("value",&standardDev1);
	handle.FirstChild("reacTime_standardDev2").ToElement()->Attribute("value",&standardDev2);


	//Reaction time 1
	//TODO: Refactor to avoid magic numbers
	if (distributionType1==0) {
		ConfigParams::GetInstance().reactDist1 = new NormalReactionTimeDist(mean1, standardDev1);
	} else if (distributionType1==1) {
		ConfigParams::GetInstance().reactDist1 = new LognormalReactionTimeDist(mean1, standardDev1);
	} else {
		throw std::runtime_error("Unknown magic reaction time number.");
	}

	//Reaction time 2
	//TODO: Refactor to avoid magic numbers
	if (distributionType2==0) {
		ConfigParams::GetInstance().reactDist2 = new NormalReactionTimeDist(mean2, standardDev2);
	} else if (distributionType2==1) {
		ConfigParams::GetInstance().reactDist2 = new LognormalReactionTimeDist(mean2, standardDev2);
	} else {
		throw std::runtime_error("Unknown magic reaction time number.");
	}

	//Save passenger distribution parameters
	int passenger_busstop_dist,passenger_crowdness_dist,passengers_min_uniformDist,passengers_max_uniformDist;
	int passenger_busstop_mean,passenger_crowdness_mean,passenger_percent_boarding,passenger_percent_alighting;
	int passenger_busstop_standardDev,passenger_crowdness_standard_dev;
	handle.FirstChild("passenger_distribution_busstop").ToElement()->Attribute("value",&passenger_busstop_dist);
	handle.FirstChild("passenger_mean_busstop").ToElement()->Attribute("value",&passenger_busstop_mean);
	handle.FirstChild("passenger_standardDev_busstop").ToElement()->Attribute("value",&passenger_busstop_standardDev);


	handle.FirstChild("passenger_percent_boarding").ToElement()->Attribute("value",&passenger_percent_boarding);
	handle.FirstChild("passenger_percent_alighting").ToElement()->Attribute("value",&passenger_percent_alighting);

	handle.FirstChild("passenger_min_uniform_distribution").ToElement()->Attribute("value",&passengers_min_uniformDist);
	handle.FirstChild("passenger_max_uniform_distribution").ToElement()->Attribute("value",&passengers_max_uniformDist);

	//for alighting passengers
	//TODO: Refactor to avoid magic numbers
	int no_of_passengers;
	srand(time(NULL));
	ConfigParams::GetInstance().percent_boarding=passenger_percent_boarding;
	ConfigParams::GetInstance().percent_alighting=passenger_percent_alighting;
	if (passenger_busstop_dist ==0) {// normal distribution
		ConfigParams::GetInstance().passengerDist_busstop  = new NormalPassengerDist(passenger_busstop_mean, passenger_busstop_standardDev);
		//	passenger_boardingmean=1+fmod(rand(),ConfigParams::GetInstance().passengerDist1->getnopassengers());
	} else if (passenger_busstop_dist==1) {// log normal distribution
		ConfigParams::GetInstance().passengerDist_busstop = new LognormalPassengerDist(passenger_busstop_mean, passenger_busstop_standardDev);
	} else if(passenger_busstop_dist==2) {// uniform distribution
		ConfigParams::GetInstance().passengerDist_busstop = new UniformPassengerDist(passengers_min_uniformDist,passengers_max_uniformDist);
	} else {
		throw std::runtime_error("Unknown magic passenger distribution number.");
	}

	//Driver::distributionType1 = distributionType1;
	int signalTimingMode;

	//Save simulation start time
	TiXmlElement* node = handle.FirstChild("start_time").ToElement();
	const char* simStartStr = node ? node->Attribute("value") : nullptr;

	node = handle.FirstChild("signalTimingMode").ToElement();
	if(node)
	{
		node->Attribute("value", &signalTimingMode);
	}

	//Save Aura Manager Implementation
	AuraManager::AuraManagerImplementation aura_mgr_impl = AuraManager::IMPL_RSTAR;
	node = handle.FirstChild("aura_manager_impl").ToElement();
	if(node) {
		const char* valC = node->Attribute("value");
		if (!valC) {
			throw std::runtime_error("Aura manager implementation requires a \"value\" tag.");
		} else {
			string val = string(valC);
			if (val=="simtree") {
				aura_mgr_impl = AuraManager::IMPL_SIMTREE;
			} else  if (val=="rdu") {
				aura_mgr_impl = AuraManager::IMPL_RDU;
			} else if (val=="rstar") {
				aura_mgr_impl = AuraManager::IMPL_RSTAR;
			} else {
				throw std::runtime_error("Unknown aura manager implementation type.");
			}
		}
	}

	//Save the WorkGroup assignment strategy
	WorkGroup::ASSIGNMENT_STRATEGY wg_assign_strat = WorkGroup::ASSIGN_ROUNDROBIN;
	node = handle.FirstChild("workgroup_assignment").ToElement();
	if(node) {
		const char* valC = node->Attribute("value");
		if (!valC) {
			throw std::runtime_error("Workgroup assignment strategy requires a \"value\" tag.");
		} else {
			string val = string(valC);
			if (val=="roundrobin") {
				wg_assign_strat = WorkGroup::ASSIGN_ROUNDROBIN;
			} else  if (val=="smallest") {
				wg_assign_strat = WorkGroup::ASSIGN_SMALLEST;
			} else {
				throw std::runtime_error("Unknown workgroup assignment strategy type.");
			}
		}
	}



#ifndef SIMMOB_DISABLE_MPI
	//Save mpi parameters, not used when running on one-pc.
	node = handle.FirstChild("partitioning_solution_id").ToElement();
	int partition_solution_id = boost::lexical_cast<int>(node->Attribute("value")) ;
#endif

	//Save more granularities
	handle = TiXmlHandle(&document);
	handle = handle.FirstChild("config").FirstChild("system").FirstChild("granularities");
	int granAgent = ReadGranularity(handle, "agent");
	int granSignal = ReadGranularity(handle, "signal");
	int granPaths = ReadGranularity(handle, "paths");
	int granDecomp = ReadGranularity(handle, "decomp");

	//Save work group sizes: system
	handle = TiXmlHandle(&document);
	handle = handle.FirstChild("config").FirstChild("system").FirstChild("workgroup_sizes");
	int agentWgSize = ReadValue(handle, "agent", 0);
	int signalWgSize = ReadValue(handle, "signal", 0);
	int commWgSize = ReadValue(handle, "Android_Communication", 0);

	//Save the "single-threaded" flag, if it exists.
	bool singleThreaded = false;
	node = TiXmlHandle(&document).FirstChild("config").FirstChild("system").FirstChild("single_threaded").ToElement();
	if (node) {
		const char* val = node->Attribute("value");
		if (val) {
			std::string valS(val);
			std::transform(valS.begin(), valS.end(), valS.begin(), ::tolower);
			if (valS=="true") {
				singleThreaded = true;
			}
		}
	}
	ConfigParams::GetInstance().singleThreaded = singleThreaded;


	//Save "network_source" if it exists.
	ConfigParams::NetworkSource netSrc = ConfigParams::NETSRC_XML;
	node = TiXmlHandle(&document).FirstChild("config").FirstChild("system").FirstChild("network_source").ToElement();
	if (node) {
		const char* val = node->Attribute("value");
		if (val) {
			std::string valS(val);
			std::transform(valS.begin(), valS.end(), valS.begin(), ::tolower);
			if (valS=="database") {
				netSrc = ConfigParams::NETSRC_DATABASE;
			} else if (valS=="xml") {
				netSrc = ConfigParams::NETSRC_XML;
			} else {
				throw std::runtime_error("Unknown road network source entry.");
			}
		}
	}
	ConfigParams::GetInstance().networkSource = netSrc;


	//Save "network_xml_file", if it exists.
	std::string netXmlFile = "";
	node = TiXmlHandle(&document).FirstChild("config").FirstChild("system").FirstChild("network_xml_file").ToElement();
	if (node) {
		const char* val = node->Attribute("value");
		if (val) {
			netXmlFile = val;
		}
	}
	if (netXmlFile.empty()) {
		netXmlFile = "data/SimMobilityInput.xml"; //Default
	}
	ConfigParams::GetInstance().networkXmlFile = netXmlFile;


	//Save "mergeLogFiles", if it exists.
	bool mergeLogFiles = false;
	node = TiXmlHandle(&document).FirstChild("config").FirstChild("system").FirstChild("merge_log_files").ToElement();
	if (node) {
		const char* val = node->Attribute("value");
		if (val) {
			std::string valS(val);
			std::transform(valS.begin(), valS.end(), valS.begin(), ::tolower);
			if (valS=="true") {
				mergeLogFiles = true;
			}
		}
	}
	ConfigParams::GetInstance().mergeLogFiles = mergeLogFiles;


	//Determine what order we will load Agents in
	handle = TiXmlHandle(&document);
	handle = handle.FirstChild("config").FirstChild("system").FirstChild("simulation").FirstChild("load_agents");
	vector<string> loadAgentOrder = ReadSpaceSepArray(handle, "order");
	cout <<"Agent Load order: ";
	if (loadAgentOrder.empty()) {
		cout <<"<N/A>";
	} else {
		for (vector<string>::iterator it=loadAgentOrder.begin(); it!=loadAgentOrder.end(); it++) {
			cout <<*it <<"  ";
		}
	}
	cout <<endl;

	//Determine the first ID for automatically generated Agents
	int startingAutoAgentID = 0; //(We'll need this later)
	handle = TiXmlHandle(&document);
	node = handle.FirstChild("config").FirstChild("system").FirstChild("simulation").FirstChild("auto_id_start").ToElement();
	if (node) {
		if (node->Attribute("value", &startingAutoAgentID) && startingAutoAgentID>0) {
			Agent::SetIncrementIDStartValue(startingAutoAgentID, true);
			cout <<"Starting ID for automatic agents: " <<startingAutoAgentID <<endl;
		}
	}


	//Buffering strategy (optional)
	handle = TiXmlHandle(&document);
	handle = handle.FirstChild("config").FirstChild("system").FirstChild("simulation").FirstChild("mutex_enforcement");
	string mutexStrat = ReadLowercase(handle, "strategy");
	MutexStrategy mtStrat = MtxStrat_Buffered;
	if (!mutexStrat.empty()) {
		if (mutexStrat == "locked") {
			mtStrat = MtxStrat_Locked;
		} else if (mutexStrat != "buffered") {
			throw std::runtime_error("Unknown mutex strategy");
		}
	}

	//Communication Simulator (optional)
	ConfigParams::GetInstance().commSimEnabled = false;
	std::string commSimYesNo;
	handle = TiXmlHandle(&document);
	node = handle.FirstChild("config").FirstChild("system").FirstChild("simulation").FirstChild("communication").ToElement();
	if (node) {
		commSimYesNo = std::string(node->Attribute("enabled"));
		if (commSimYesNo == "yes") {
			ConfigParams::GetInstance().commSimEnabled = true;
			//createCommunicator();
		}
	}

	if(commSimYesNo == "yes")
	{
		ConfigParams::GetInstance().androidClientEnabled = isAndroidClientEnabled(handle);
	}



	//Busline Control strategy (optional)
	handle = TiXmlHandle(&document);
	node = handle.FirstChild("config").FirstChild("system").FirstChild("simulation").FirstChild("busline_control_type").ToElement();
	if(node) {
		const char* valStr_controlType = node->Attribute("strategy");
		std::string busline_control_type(valStr_controlType);
		if(busline_control_type == "schedule_based" || busline_control_type == "headway_based"
				|| busline_control_type == "evenheadway_based" || busline_control_type == "hybrid_based"
				|| busline_control_type == "no_control") {
			ConfigParams::GetInstance().busline_control_type = busline_control_type;
		} else {
			ConfigParams::GetInstance().busline_control_type = "no_control";// default: no control
		}
	} else {
		ConfigParams::GetInstance().busline_control_type = "no_control";// if no setting for this variable: also no control
	}


	//Miscellaneous settings
	handle = TiXmlHandle(&document);
	node = handle.FirstChild("config").FirstChild("system").FirstChild("misc").FirstChild("manual_fix_demo_intersection").ToElement();
	if (node) {
		ConfigParams::GetInstance().TEMP_ManualFixDemoIntersection = true;
		cout <<"*******************************************" <<endl;
		cout <<"Manual override used for demo intersection." <<endl;
		cout <<"*******************************************" <<endl;
	}

	//Check if we have options for a manual schema specification.
	handle = TiXmlHandle(&document);
	node = handle.FirstChild("config").FirstChild("system").FirstChild("xsd_schema_files").FirstChild("road_network").ToElement();
	if (node) {
		bool triedOnce = false;
		node = node->FirstChild("option")->ToElement();
		while (node) {
			triedOnce = true;
			const char* optVal = node->Attribute("value");
			if (optVal) {
				//See if the file exists.
				if (boost::filesystem::exists(optVal)) {
					//Convert it to an absolute path.
					boost::filesystem::path abs_path = boost::filesystem::absolute(optVal);
					ConfigParams::GetInstance().roadNetworkXsdSchemaFile = abs_path.string();
					break;
				}
			}
			TiXmlNode* sib = node->NextSibling("option");
			node = sib ? sib->ToElement() : nullptr;
		}

		//Did we find nothing?
		if (triedOnce && ConfigParams::GetInstance().roadNetworkXsdSchemaFile.empty()) {
			Warn() <<"Warning: No viable options for road_network schema file." <<std::endl;
		}
	}


	//Output
	const std::string schem = ConfigParams::GetInstance().roadNetworkXsdSchemaFile;
	Print() <<"XML (road network) schema file: "  <<(schem.empty()?"<default>":schem) <<std::endl;



	//Misc.: disable dynamic dispatch?
	handle = TiXmlHandle(&document);
	node = handle.FirstChild("config").FirstChild("system").FirstChild("misc").FirstChild("disable_dynamic_dispatch").ToElement();
	if (node) {
		const char* valStr_c = node->Attribute("value");
		if (valStr_c) {
			std::string valStr(valStr_c);
			if (valStr == "true") {
				ConfigParams::GetInstance().SetDynamicDispatchDisabled(true);
			} else if (valStr == "false") {
				ConfigParams::GetInstance().SetDynamicDispatchDisabled(false);
			} else {
				throw std::runtime_error("Invalid parameter; expecting boolean.");
			}
		}
	}

	std::cout <<"Dynamic dispatch: " <<(ConfigParams::GetInstance().DynamicDispatchDisabled() ? "DISABLED" : "Enabled") <<std::endl;


	//Series of one-line checks.
	if(baseGran == -1) { throw std::runtime_error("Config file fails to specify base granularity."); }
	if(totalRuntime == -1) { throw std::runtime_error("Config file fails to specify total runtime."); }
	if(totalWarmup == -1) { throw std::runtime_error("Config file fails to specify total warmup."); }
	if(granAgent == -1) { throw std::runtime_error("Config file fails to specify agent granularity."); }
	if(granSignal == -1) { throw std::runtime_error("Config file fails to specify signal granularity."); }
	if(agentWgSize == -1) { throw std::runtime_error("Config file fails to specify agent workgroup size."); }
	if(signalWgSize == -1) { throw std::runtime_error("Config file fails to specify signal workgroup size."); }
	if (!simStartStr) { throw std::runtime_error("Config file fails to specify simulation start time."); }

    //Granularity check
    if (granAgent < baseGran) {
    	throw std::runtime_error("Agent granularity cannot be smaller than base granularity.");
    }
    if (granAgent%baseGran != 0) {
    	throw std::runtime_error("Agent granularity not a multiple of base granularity.");
    }
    if (granSignal < baseGran) {
    	throw std::runtime_error("Signal granularity cannot be smaller than base granularity.");
    }
    if (granSignal%baseGran != 0) {
    	throw std::runtime_error("Signal granularity not a multiple of base granularity.");
    }
    if (granPaths < baseGran) {
    	throw std::runtime_error("Path granularity cannot be smaller than base granularity.");
    }
    if (granPaths%baseGran != 0) {
    	throw std::runtime_error("Path granularity not a multiple of base granularity.");
    }
    if (granDecomp < baseGran) {
    	throw std::runtime_error("Decomposition granularity cannot be smaller than base granularity.");
    }
    if (granDecomp%baseGran != 0) {
    	throw std::runtime_error("Decomposition granularity not a multiple of base granularity.");
    }
    if (totalRuntime < baseGran) {
    	throw std::runtime_error("Total Runtime cannot be smaller than base granularity.");
    }
    if (totalRuntime%baseGran != 0) {
    	Warn() <<"Total runtime (" <<totalRuntime <<") will be truncated by the base granularity (" <<baseGran <<")\n";
    }
    if (totalWarmup != 0 && totalWarmup < baseGran) {
    	Warn() << "Warning! Total Warmup is smaller than base granularity.\n";
    }
    if (totalWarmup%baseGran != 0) {
    	Warn() <<"Total warmup (" <<totalWarmup <<") will be truncated by the base granularity (" <<baseGran <<")\n";
    }

    //Save params
    {
    	ConfigParams& config = ConfigParams::GetInstance();
    	config.baseGranMS = baseGran;
    	config.totalRuntimeTicks = totalRuntime/baseGran;
    	config.totalWarmupTicks = totalWarmup/baseGran;
    	config.granAgentsTicks = granAgent/baseGran;
    	config.granSignalsTicks = granSignal/baseGran;
    	config.granPathsTicks = granPaths/baseGran;
    	config.granDecompTicks = granDecomp/baseGran;
    	config.agentWorkGroupSize = agentWgSize;
    	config.signalWorkGroupSize = signalWgSize;
    	config.commWorkGroupSize = commWgSize;
    	config.simStartTime = DailyTime(simStartStr);
    	config.mutexStategy = mtStrat;
    	config.signalTimingMode = signalTimingMode;

    	//Save the Aura Manager implementation type.
    	config.aura_manager_impl = aura_mgr_impl;

    	//Save the WorkGroup strategy.
    	config.defaultWrkGrpAssignment = wg_assign_strat;

    	//add for MPI
#ifndef SIMMOB_DISABLE_MPI
    	if (config.using_MPI) {
			sim_mob::PartitionManager& partitionImpl = sim_mob::PartitionManager::instance();
			std::cout << "partition_solution_id in configuration:" << partition_solution_id << std::endl;

			partitionImpl.partition_config->partition_solution_id = partition_solution_id;
    	}
#endif

    }

	//load scheduledTImes if any
	handle = TiXmlHandle(&document);
	TiXmlElement* busScheduleTimes = handle.FirstChild("config").FirstChild("scheduledTimes").FirstChild().ToElement();
	if(busScheduleTimes){
		int stop = 0;
		for (;busScheduleTimes; busScheduleTimes=busScheduleTimes->NextSiblingElement()) {
			int AT = atoi(busScheduleTimes->Attribute("offsetAT"));
			int DT = atoi(busScheduleTimes->Attribute("offsetDT"));
			vector<int> times;
			times.push_back(AT);
			times.push_back(DT);
			std::pair<int, vector<int> > nextLink(stop, times);
			ConfigParams::GetInstance().scheduledTimes.insert(nextLink);
			++stop;
		}
	}

    //Check the type of geometry
    handle = TiXmlHandle(&document);
    TiXmlElement* geomElem = handle.FirstChild("config").FirstChild("geometry").ToElement();
    if (geomElem) {
    	const char* geomType = geomElem->Attribute("type");
    	if (geomType && string(geomType) == "simple") {
    		throw std::runtime_error("\"simple\" geometry type no longer supported.");
    	} else if (geomType && string(geomType) == "aimsun") {
    		//Ensure we're loading from a database
    		const char* geomSrc = geomElem->Attribute("source");
    		if (!geomSrc || "database" != string(geomSrc)) {
    			throw std::runtime_error("Unknown geometry source");
    		}

    		//Load from the database or from XML, depending.
    		if (ConfigParams::GetInstance().networkSource==ConfigParams::NETSRC_DATABASE) {
    			cout <<"Loading from DATABASE\n";

				//Load the AIMSUM network details
				if (prof) { prof->logGenericStart("Database", "main-prof"); }
				map<string, string> storedProcedures; //Of the form "node" -> "get_node()"
				if (!LoadDatabaseDetails(*geomElem, ConfigParams::GetInstance().connectionString, storedProcedures)) {
						throw std::runtime_error("Unable to load database connection settings...");
				}

				//Actually load it
				sim_mob::aimsun::Loader::LoadNetwork(ConfigParams::GetInstance().connectionString, storedProcedures, ConfigParams::GetInstance().getNetworkRW(), ConfigParams::GetInstance().getTripChains(), prof);
    		} else {
    			cout <<"Loading from XML\n";
				if (!sim_mob::xml::InitAndLoadXML(ConfigParams::GetInstance().networkXmlFile, ConfigParams::GetInstance().getNetworkRW(), ConfigParams::GetInstance().getTripChains())) {
					throw std::runtime_error("Error loading/parsing XML file (see stderr).");
				}
    		}

    		//Finally, mask the password
    		string& s = ConfigParams::GetInstance().connectionString;
    		size_t check = s.find("password=");
    		if (check!=string::npos) {
    			size_t start = s.find("=", check) + 1;
    			size_t end = s.find(" ", start);
    			size_t amt = ((end==string::npos) ? s.size() : end) - start;
    			s = s.replace(start, amt, amt, '*');
    		}
    	} else {
    		throw std::runtime_error("Unknown geometry type");
    	}
    }

    //TEMP: Test network output via boost.
    BoostSaveXML("NetworkCopy.xml", ConfigParams::GetInstance().getNetworkRW());

    //debug: detect sidewalks which are in the middle of road
    {
    	sim_mob::RoadNetwork &rn = ConfigParams::GetInstance().getNetworkRW();
    	for(std::vector<sim_mob::Link *>::iterator it = rn.getLinks().begin(), it_end(rn.getLinks().end()); it != it_end ; it ++)
    	{
    		for(std::set<sim_mob::RoadSegment *>::iterator seg_it = (*it)->getUniqueSegments().begin(), it_end((*it)->getUniqueSegments().end()); seg_it != it_end; seg_it++)
    		{
    			for(std::vector<sim_mob::Lane*>::const_iterator lane_it = (*seg_it)->getLanes().begin(), it_end((*seg_it)->getLanes().end()); lane_it != it_end ; lane_it++)
    			{
    				if(((*lane_it) != (*seg_it)->getLanes().front()) && ((*lane_it) != (*seg_it)->getLanes().back()) && (*lane_it)->is_pedestrian_lane())
    				{
    					std::cout << "we have a prolem with a pedestrian lane in the middle of the segment\n";
    				}
    			}
    		}
    	}
    }
    //debug.. end

 	//Generate lanes, before StreetDirectory::init()
 	RoadNetwork::ForceGenerateAllLaneEdgePolylines(ConfigParams::GetInstance().getNetworkRW());

    //Seal the network; no more changes can be made after this.
    ConfigParams::GetInstance().sealNetwork();
    std::cout << "Network Sealed" << std::endl;
    //Basically, we need to write the XML as soon as the network is sealed.
 //This is the real place to extract xml from road network, tripchian, signal etc but since 'seal network' is not respected (especially in street directory)
 //therefore we write xml after InitUserConf() is completed (in the currently main.cpp)
 #ifdef SIMMOB_XML_WRITER
 	/*
 	 *******************************
 	 * XML Writer
 	 *******************************
 	 */

 	sim_mob::WriteXMLInput(

 			XML_OutPutFileName
 //			"SimOut.xml"

 			);
 	std::cout << "XML input for SimMobility Created....\n";
 	return "XML input for SimMobility Created....\n";//shouldn't be empty :)
 #endif

 	//Initialize the street directory.
    StreetDirectory::instance().init(ConfigParams::GetInstance().getNetwork(), true);
    std::cout << "Street Directory initialized" << std::endl;

    //process confluxes
    std::cout << "confluxes size before: " << ConfigParams::GetInstance().getConfluxes().size() << std::endl;
    sim_mob::aimsun::Loader::ProcessConfluxes(ConfigParams::GetInstance().getNetwork());
    std::cout << "confluxes size after: " << ConfigParams::GetInstance().getConfluxes().size() << std::endl;

    //Maintain unique/non-colliding IDs.
    AgentConstraints constraints;
    constraints.startingAutoAgentID = startingAutoAgentID;

    //Attempt to load all "BusController" elements from the config file.
    if(!loadXMLBusControllers(document, active_agents, pending_agents)) {
    	std::cout << "loadXMLBusControllers Failed!" << std::endl;
    	throw std::runtime_error("Couldn't load buscontrollers");
    }

    if(!loadXMLFMODController(document)) {
    	std::cout << "loadXMLFMODController Failed!" << std::endl;
    	throw std::runtime_error("Couldn't load FMOD controller");
    }

    //Initialize all BusControllers.
	if(BusController::HasBusControllers()) {
		BusController::InitializeAllControllers(active_agents, ConfigParams::GetInstance().getPT_bus_dispatch_freq());
	}

    //Load Agents, Pedestrians, and Trip Chains as specified in loadAgentOrder
	for (vector<string>::iterator it = loadAgentOrder.begin();
			it != loadAgentOrder.end(); it++) {
		if (((*it) == "database") || ((*it) == "xml-tripchains")) {
			//Create an agent for each Trip Chain in the database.
			generateAgentsFromTripChain(active_agents, pending_agents,
					constraints);
			cout << "Loaded Database Agents (from Trip Chains)." << endl;
		} else if ((*it) == "drivers") {
			if (!loadXMLAgents(document, active_agents, pending_agents,
					"driver", constraints)) {
				throw std::runtime_error("Couldn't load drivers");
			}
			if (!loadXMLAgents(document, active_agents, pending_agents,
					"busdriver", constraints)) {
				throw std::runtime_error("Couldn't load bus drivers");
			}
			cout << "Loaded Driver Agents (from config file)." << endl;

		} else if ((*it) == "pedestrians") {
			if (!loadXMLAgents(document, active_agents, pending_agents,
					"pedestrian", constraints)) {
				throw std::runtime_error("Couldn't load pedestrians");
			}
			cout << "Loaded Pedestrian Agents (from config file)." << endl;
		} else if ((*it) == "passengers") {
			if (!loadXMLAgents(document, active_agents, pending_agents,
					"passenger", constraints)) {
				throw std::runtime_error("Couldn't load passengers");
			}
			cout << "Loaded Passenger Agents (from config file)." << endl;
		} else {
			throw std::runtime_error("Unknown item in load_agents");
		}
	}
	std::cout
			<< "Loading Agents, Pedestrians, and Trip Chains as specified in loadAgentOrder Success!"
			<< std::endl;

    //Load signals, which are currently agents
    if (!loadXMLSignals(document,
#if 0
    		Signal::all_signals_,
#endif
    		"signal")) {
    	std::cout << "loadXMLSignals Failed!" << std::endl;
    	throw std::runtime_error("Couldn't load signals");
    }

    //Display
    std::cout <<"Config parameters:\n";
    std::cout <<"------------------\n";
    std::cout <<"Force single-threaded: " <<(ConfigParams::GetInstance().singleThreaded?"yes":"no") <<"\n";
	//Print the WorkGroup strategy.
	std::cout <<"WorkGroup assignment: ";
	if (ConfigParams::GetInstance().defaultWrkGrpAssignment==WorkGroup::ASSIGN_ROUNDROBIN) {
		std::cout <<"roundrobin" <<std::endl;
	} else if (ConfigParams::GetInstance().defaultWrkGrpAssignment==WorkGroup::ASSIGN_SMALLEST) {
		std::cout <<"smallest" <<std::endl;
	} else {
		std::cout <<"<unknown>" <<std::endl;
	}
	std::cout <<"  Base Granularity: " <<ConfigParams::GetInstance().baseGranMS <<" " <<"ms" <<"\n";
    std::cout <<"  Total Runtime: " <<ConfigParams::GetInstance().totalRuntimeTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Total Warmup: " <<ConfigParams::GetInstance().totalWarmupTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Agent Granularity: " <<ConfigParams::GetInstance().granAgentsTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Signal Granularity: " <<ConfigParams::GetInstance().granSignalsTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Paths Granularity: " <<ConfigParams::GetInstance().granPathsTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Decomp Granularity: " <<ConfigParams::GetInstance().granDecompTicks <<" " <<"ticks" <<"\n";
    std::cout <<"  Start time: " <<ConfigParams::GetInstance().simStartTime.toString() <<"\n";
    std::cout <<"  Mutex strategy: " <<(ConfigParams::GetInstance().mutexStategy==MtxStrat_Locked?"Locked":ConfigParams::GetInstance().mutexStategy==MtxStrat_Buffered?"Buffered":"Unknown") <<"\n";
    if (!ConfigParams::GetInstance().boundaries.empty() || !ConfigParams::GetInstance().crossings.empty()) {
    	std::cout <<"  Boundaries Found: " <<ConfigParams::GetInstance().boundaries.size() <<"\n";
		for (map<string, Point2D>::iterator it=ConfigParams::GetInstance().boundaries.begin(); it!=ConfigParams::GetInstance().boundaries.end(); it++) {
			std::cout <<"    Boundary[" <<it->first <<"] = (" <<it->second.getX() <<"," <<it->second.getY() <<")\n";
		}
		std::cout <<"  Crossings Found: " <<ConfigParams::GetInstance().crossings.size() <<"\n";
		for (map<string, Point2D>::iterator it=ConfigParams::GetInstance().crossings.begin(); it!=ConfigParams::GetInstance().crossings.end(); it++) {
			std::cout <<"    Crossing[" <<it->first <<"] = (" <<it->second.getX() <<"," <<it->second.getY() <<")\n";
		}
    }
    //if (!ConfigParams::GetInstance().connectionString.empty()) {
    	//Output AIMSUN data
    	std::cout <<"Network details loaded from connection: " <<ConfigParams::GetInstance().connectionString <<"\n";
    	std::cout <<"------------------\n";
    	PrintDB_NetworkToFile(ConfigParams::GetInstance().outNetworkFileName);
    	std::cout <<"------------------\n";
   // }
    std::cout <<"  Agents Initialized: " <<Agent::all_agents.size() << "|Agents Pending: " << Agent::pending_agents.size() <<"\n";
    /*for (size_t i=0; i<active_agents.size(); i++) {
    	//std::cout <<"    Agent(" <<agents[i]->getId() <<") = " <<agents[i]->xPos.get() <<"," <<agents[i]->yPos.get() <<"\n";

    	Person* p = dynamic_cast<Person*>(active_agents[i]);
    	if (p && p->getTripChain()) {
    		//const TripChain* const tc = p->getTripChain();
    		//std::cout <<"      Trip Chain start time: " <<tc->startTime.toString()  <<" from: " <<tc->from.description <<"(" <<tc->from.location <<") to: " <<tc->to.description <<"(" <<tc->to.location <<") mode: " <<tc->mode <<" primary: " <<tc->primary  <<" flexible: " <<tc->flexible <<"\n";
    	}
    }*/
    std::cout <<"------------------\n";
    // PrintDB_Network() calls getLaneEdgePolyline() which inserts side-walks into the
    // road-segments.  We can only only initialize the StreetDirectory only now, not before.
    //StreetDirectory::instance().init(ConfigParams::GetInstance().getNetwork(), true);

    // Each Signal has its own LoopDetectorEntity which is an Entity that must run at the same
    // rate as the Driver objects.  So we need to put the loop-detectors into the all_agents
    // list.
    //
    // The name "PrintDB_Network" is misleading.  The name gives the impression that the function
    // only dumps the road network data onto std::cout and does not modify the road network.  But
    // it does as the function "insert" side-walks which means road-segments may have more
    // lanes when the function returns.
    //
    // Since the LoopDetectorEntity::init() function needs the lanes information, we can only call
    // it here.
    //todo I think when a loop detector data are dynamically assigned to signal rather that being read from data base,
    //they should be handled with in the signal constructor, not here
    if(BusController::HasBusControllers()) {
    	BusController::DispatchAllControllers(active_agents);
    }

    std::vector<Signal*>& all_signals = Signal::all_signals_;

#if 0
    sim_mob::Signal::All_Signals & all_signals = sim_mob::Signal::all_signals_;
#endif

    for (size_t i = 0; i < all_signals.size(); ++i)
    {
    	Signal  * signal;
//        Signal const * signal = const_cast<Signal *>(Signal::all_signals_[i]);
#if 0
    	signal =  dynamic_cast<Signal  *>(all_signals[i]);
    	LoopDetectorEntity & loopDetector = const_cast<LoopDetectorEntity&>(signal->loopDetector());
#else
    	signal =  dynamic_cast<Signal_SCATS  *>(all_signals[i]);
    	LoopDetectorEntity & loopDetector = const_cast<LoopDetectorEntity&>(dynamic_cast<Signal_SCATS  *>(signal)->loopDetector());
#endif
        loopDetector.init(*signal);
        active_agents.push_back(&loopDetector);
    }
}



} //End anon namespace



//////////////////////////////////////////
// Simple singleton implementation
//////////////////////////////////////////
ConfigParams sim_mob::ConfigParams::instance;


void sim_mob::ConfigParams::reset()
{
	//TODO: This *should* work fine, but check the comment below.
	instance = sim_mob::ConfigParams();

	//TODO: This is the old code for reset(); I prefer the above code, as it is more generic, but I
	//      have not tested that it works.  ~Seth
	//sealedNetwork=false;
	//roleFact.clear();
}

sim_mob::CommunicationDataManager&  sim_mob::ConfigParams::getCommDataMgr() const
{
#ifdef SIMMOB_INTERACTIVE_MODE
	if (!commDataMgr) {
		commDataMgr = new CommunicationDataManager();
	}
	return *commDataMgr;
#else
	throw std::runtime_error("ConfigParams::getCommDataMgr() not supported; SIMMOB_INTERACTIVE_MODE is off.");
#endif
}

sim_mob::ControlManager* sim_mob::ConfigParams::getControlMgr() const
{
#ifdef SIMMOB_INTERACTIVE_MODE
	//In this case, ControlManager's constructor performs some logic, so it's best to use a pointer.
	if (!controlMgr) {
		controlMgr = new ControlManager();
	}
	return controlMgr;
#else
	throw std::runtime_error("ConfigParams::getControlMgr() not supported; SIMMOB_INTERACTIVE_MODE is off.");
#endif
}


//////////////////////////////////////////
// Macro definitions
//////////////////////////////////////////


bool sim_mob::ConfigParams::UsingConfluxes() const
{
#ifdef SIMMOB_USE_CONFLUXES
	return true;
#else
	return false;
#endif
}


bool sim_mob::ConfigParams::MPI_Disabled() const
{
#ifdef SIMMOB_DISABLE_MPI
	return true;
#else
	return false;
#endif
}


bool sim_mob::ConfigParams::OutputDisabled() const
{
#ifdef SIMMOB_DISABLE_OUTPUT
	return true;
#else
	return false;
#endif
}

bool sim_mob::ConfigParams::InteractiveMode() const
{
#ifdef SIMMOB_INTERACTIVE_MODE
	return true;
#else
	return false;
#endif
}

bool sim_mob::ConfigParams::StrictAgentErrors() const
{
#ifdef SIMMOB_STRICT_AGENT_ERRORS
	return true;
#else
	return false;
#endif
}

bool sim_mob::ConfigParams::ProfileOn() const
{
#ifdef SIMMOB_PROFILE_ON
	return true;
#else
	return false;
#endif
}

bool sim_mob::ConfigParams::ProfileAgentUpdates(bool accountForOnFlag) const
{

#ifdef SIMMOB_PROFILE_AGENT_UPDATES
	if (accountForOnFlag) {
		return ProfileOn();
	}
	return true;
#else
	return false;
#endif
}

bool sim_mob::ConfigParams::ProfileWorkerUpdates(bool accountForOnFlag) const
{
#ifdef SIMMOB_PROFILE_WORKER_UPDATES
	if (accountForOnFlag) {
		return ProfileOn();
	}
	return true;
#else
	return false;
#endif
}


//////////////////////////////////////////
// Main external method
//////////////////////////////////////////

void sim_mob::ConfigParams::InitUserConf(const string& configPath, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, ProfileBuilder* prof, const Config::BuiltInModels& builtInModels)
{
	//We'll be switching over pretty cleanly, so just use a local variable here.
	//  * simpleconf.cpp will be removed (and InitUserConf will go somewhere else).
	//  * Various new "loaders" or "initializers" will take Config objects and perform their tasks.
	const bool LOAD_NEW_CONFIG_FILE = false;


	if (LOAD_NEW_CONFIG_FILE) {
		//Load using our new config syntax.

		//Load and parse the file, create xml-based objects.
		Config cfg;
		cfg.InitBuiltInModels(builtInModels);
		if (!sim_mob::xml::InitAndLoadConfigXML("data/simrun_seth.xml", cfg)) {
			throw std::runtime_error("New config XML loader failed.");
		}

		//Validate simple data elements
		//This needs to be first, since it sets our mutex enforcement strategy.
		Validate val(cfg);

		//Load the Road Network
		LoadNetwork loadN(cfg);

		//Load the Agents
		LoadAgents loadA(cfg, active_agents, pending_agents);

		//Print the network.
		PrintNetwork print(cfg);

		//Finally, print general logging information to stdout
		GeneralOutput out(cfg);
	} else {
		//Load using our old config syntax.

		//Load our config file into an XML document object.
		//NOTE: Do *not* use by-value syntax for doc. For some reason, this crashes OSX.
		TiXmlDocument* doc = new TiXmlDocument(configPath);

		if (prof) { prof->logGenericStart("XML", "main-prof-xml"); }
		if (!doc->LoadFile()) {
			std::stringstream msg;
			msg <<"Error loading config file: " <<doc->ErrorDesc();
			throw std::runtime_error(msg.str().c_str());
		}
		if (prof) { prof->logGenericEnd("XML", "main-prof-xml"); }

		//Parse it
		loadXMLConf(*doc, active_agents, pending_agents, prof);
		delete doc;
	}
}


