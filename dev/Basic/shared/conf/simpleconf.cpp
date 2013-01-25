/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch
#include "simpleconf.hpp"

//Make sure our "test" (new) Config variant compiles.
#include "conf/xmlLoader/implementation/conf1-driver.hpp"

#include <tinyxml.h>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <algorithm>
#include <boost/lexical_cast.hpp>

#include "geospatial/xmlWriter/boostXmlWriter.hpp"

//Include here (forward-declared earlier) to avoid include-cycles.
#include "entities/PendingEvent.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/BusController.hpp"
#include "entities/signal/Signal.hpp"

#include "entities/profile/ProfileBuilder.hpp"
#include "entities/misc/BusSchedule.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/BusStop.hpp"
#include "util/ReactionTimeDistributions.hpp"
#include "util/OutputUtil.hpp"

#include "geospatial/xmlLoader/geo8-driver.hpp"
#include "geospatial/xmlLoader/geo10.hpp"

//add by xuyan
#include "partitions/PartitionManager.hpp"
//#include "../short/xmlWriter/xmlWriter.hpp"

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



int ReadValue(TiXmlHandle& handle, const std::string& propName)
{
	TiXmlElement* node = handle.FirstChild(propName).ToElement();
	if (!node) {
		return -1;
	}

	int value;
	if (!node->Attribute("value", &value)) {
		return -1;
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
	int i = 0;
	ConfigParams& config = ConfigParams::GetInstance();
	std::map<unsigned int, vector<TripChainItem*> >& tcs = ConfigParams::GetInstance().getTripChains();

	std::cout << "tcs.count = " << tcs[1].size() << std::endl;
	//The current agent we are working on.
	Person* currAg = nullptr;
	std::string trip_mode;
	std::vector<const TripChainItem*> currAgTripChain;

	typedef vector<TripChainItem*>::const_iterator TCVectIt;
	typedef std::map<unsigned int, vector<TripChainItem*> >::iterator TCMapIt;
	for (TCMapIt it_map=tcs.begin(); it_map!=tcs.end(); it_map++) {
		TripChainItem* tc = it_map->second.front();
		currAg = new Person("XML_TripChain", config.mutexStategy, it_map->second); i++;
//		std::cout << "Person::preson " << currAg->getId() << "[" << currAg << "] : currTripChainItem[" << currAg->currTripChainItem << "] : currSubTrip[" << currAg->currSubTrip << "]" << std::endl;
//		//getchar();
//		const Trip* trip = dynamic_cast<const Trip*>(tc);
//		const Activity* act = dynamic_cast<const Activity*>(tc);
//
//		if (trip && tc->itemType==TripChainItem::IT_TRIP) {
//			const SubTrip firstSubTrip = trip->getSubTrips()[0];
//			//Origin and destination must be those of the first subtrip if current item is a trip
//			currAg->originNode = firstSubTrip.fromLocation;
//			currAg->destNode = firstSubTrip.toLocation;
//			trip_mode = firstSubTrip.mode;// currently choose the first subtrip mode as the mode of the trip
//		} else if (act && tc->itemType==TripChainItem::IT_ACTIVITY) {
//			currAg->originNode = currAg->destNode = act->location;
//		} else { //Offer some protection
//			throw std::runtime_error("Trip/Activity mismatch, or unknown TripChainItem subclass.");
//		}
//
//		currAg->setTripChain(it_map->second);
//		if (currAg->currSubTrip) {
//			if (currAg->currSubTrip->mode == "Bus") {
//				// currently only one
//				if (!BusController::all_busctrllers_.empty()) {
//					BusController::all_busctrllers_[0]->addOrStashBuses(currAg,
//							active_agents);
//				}
//			}
//		}
//		else
		{
//			std::cout << i << " Person Agent addorstashing..\n"; /*getchar();*/
			addOrStashEntity(currAg, active_agents, pending_agents);
		}

		//Reset for the next (possible) Agent
		currAg = nullptr;

		
	} //outer for loop(map)
}




bool loadXMLAgents(TiXmlDocument& document, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, const std::string& agentType, AgentConstraints& constraints)
{
	ConfigParams& config = ConfigParams::GetInstance();

	//At the moment, we only load *Roles* from the config file. So, check if this is a valid role.
	// This will only generate an error if someone actually tries to load an agent of this type.
	const RoleFactory& rf = config.getRoleFactory();
	bool knownFole = rf.isKnownRole(agentType);

	//Attempt to load either "agentType"+s or "agentType"+es (drivers, buses).
	// This allows ungrammatical terms like "driveres", but it's probably better than being too restrictive.
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild(agentType+"s").FirstChild(agentType).ToElement();
	if (!node) {
		node = handle.FirstChild("config").FirstChild(agentType+"es").FirstChild(agentType).ToElement();
	}

	//If at least one elemnt of an unknown type exists, it's an error.
	if (node && !knownFole) {
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

		//Create the Person agent with that given ID (or an auto-generated one)
		Person* agent = new Person("XML_Def", config.mutexStategy, manualID);
		agent->setConfigProperties(props);
		agent->setStartTime(startTime);

		//Add it or stash it
		addOrStashEntity(agent, active_agents, pending_agents);
	}

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
            std::cerr << "buscontrollers must have 'time' attributes with numerical values in the config file." << std::endl;
            return false;
        }
	}
	return true;
}

bool loadXMLSignals(TiXmlDocument& document, const std::string& signalKeyID)

#if 0
bool loadXMLSignals(TiXmlDocument& document, std::vector<Signal*> all_signals, const std::string& signalKeyID)
#endif

{
	std::cout << "inside loadXMLSignals !" << std::endl;
	//Quick check.
	if (signalKeyID!="signal") {
		std::cout << "oops! returning false!" << std::endl;
		return false;
	}

	//Build the expression dynamically.
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild(signalKeyID+"s").FirstChild(signalKeyID).ToElement();
	if (!node) {
		//Signals are optional
		std::cout << "ooops! returning true!" << std::endl;
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
            	std::cout << "catch the loop error try!" << std::endl;
                std::cerr << "signals must have 'id', 'xpos', and 'ypos' attributes with numerical values in the config file." << std::endl;
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
		string pair = (connectionString.empty()?"":" ") + string(name) + "=" + string(value);
		connectionString += pair;
	}

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



bool LoadXMLBoundariesCrossings(TiXmlDocument& document, const string& parentStr, const string& childStr, map<string, Point2D>& result)
{
	//Quick check.
	if (parentStr!="boundaries" && parentStr!="crossings") {
		return false;
	}
	if (childStr!="boundary" && childStr!="crossing") {
		return false;
	}

	//Build the expression dynamically.
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild(parentStr).FirstChild(childStr).ToElement();
	if (!node) {
		//Boundaries/crossings are optional
		return true;
	}

	//Move through results
	for (;node; node=node->NextSiblingElement()) {
		//xmlNode* curr = xpObject->nodesetval->nodeTab[i];
		string key;
		double xPos;
		double yPos;
		unsigned int flagCheck = 0;
		for (TiXmlAttribute* attr=node->FirstAttribute(); attr; attr=attr->Next()) {
			//Read each attribute.
			std::string name = attr->NameTStr();
			std::string value = attr->ValueStr();
			if (name.empty() || value.empty()) {
				return false;
			}

			//Assign it.
			if (name=="position") {
				key = value;
				flagCheck |= 1;
			} else {
				int valueI;
				std::istringstream(value) >> valueI;
				if (name=="xPos") {
					xPos = valueI;
					flagCheck |= 2;
				} else if (name=="yPos") {
					yPos = valueI;
					flagCheck |= 4;
				} else {
					return false;
				}
			}
		}

		if (flagCheck!=7) {
			return false;
		}

		//Save it.
		result[key] = Point2D(xPos, yPos);
	}

	return true;
}



//NOTE: We guarantee that the log file contains data in the order it will be needed. So, Nodes are listed
//      first because Links need Nodes. Otherwise, the output will be in no guaranteed order.
void PrintDB_Network()
{
	if (ConfigParams::GetInstance().OutputDisabled()) {
		return;
	}

	//Save RoadSegments/Connectors to make output simpler
	std::set<const RoadSegment*> cachedSegments;
	std::set<LaneConnector*> cachedConnectors;

	//Initial message
	const RoadNetwork& rn = ConfigParams::GetInstance().getNetwork();
	LogOutNotSync("Printing node network" <<endl);
	LogOutNotSync("NOTE: All IDs in this section are consistent for THIS simulation run, but will change if you run the simulation again." <<endl);

	//Print some properties of the simulation itself
	LogOutNotSync("(\"simulation\", 0, 0, {");
	LogOutNotSync("\"frame-time-ms\":\"" <<ConfigParams::GetInstance().baseGranMS <<"\",");
	LogOutNotSync("})" <<endl);


	sim_mob::Signal::all_signals_const_Iterator it;
	for (it = sim_mob::Signal::all_signals_.begin(); it!= sim_mob::Signal::all_signals_.end(); it++)

#if 0
	for (std::vector<Signal*>::const_iterator it=Signal::all_signals_.begin(); it!=Signal::all_signals_.end(); it++)
#endif
	//Print the Signal representation.
	{
		LogOutNotSync((*it)->toString() <<endl);
	}


	//Print nodes first
	for (set<UniNode*>::const_iterator it=rn.getUniNodes().begin(); it!=rn.getUniNodes().end(); it++) {
		LogOutNotSync("(\"uni-node\", 0, " <<*it <<", {");
		LogOutNotSync("\"xPos\":\"" <<(*it)->location.getX() <<"\",");
		LogOutNotSync("\"yPos\":\"" <<(*it)->location.getY() <<"\",");
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			LogOutNotSync((*it)->originalDB_ID.getLogItem());
		}
		LogOutNotSync("})" <<endl);

		//Cache all segments
		vector<const RoadSegment*> segs = (*it)->getRoadSegments();
		for (vector<const RoadSegment*>::const_iterator i2=segs.begin(); i2!=segs.end(); ++i2) {
			cachedSegments.insert(*i2);
		}
	}
	for (vector<MultiNode*>::const_iterator it=rn.getNodes().begin(); it!=rn.getNodes().end(); it++) {
		LogOutNotSync("(\"multi-node\", 0, " <<*it <<", {");
		LogOutNotSync("\"xPos\":\"" <<(*it)->location.getX() <<"\",");
		LogOutNotSync("\"yPos\":\"" <<(*it)->location.getY() <<"\",");
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			LogOutNotSync((*it)->originalDB_ID.getLogItem());
		}
		LogOutNotSync("})" <<endl);

		//NOTE: This is temporary; later we'll ensure that the RoadNetwork only stores Intersections,
		//      and RoadSegments will have to be extracted.
		const Intersection* nodeInt = static_cast<const Intersection*>((*it));

		//Cache all segments
		for (set<RoadSegment*>::const_iterator i2=nodeInt->getRoadSegments().begin(); i2!=nodeInt->getRoadSegments().end(); ++i2) {
			cachedSegments.insert(*i2);
			//std::cout <<"   Has segement: " <<*i2 <<"\n";
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
		LogOutNotSync("(\"link\", 0, " <<*it <<", {");
		LogOutNotSync("\"road-name\":\"" <<(*it)->roadName <<"\",");
		LogOutNotSync("\"start-node\":\"" <<(*it)->getStart() <<"\",");
		LogOutNotSync("\"end-node\":\"" <<(*it)->getEnd() <<"\",");
		LogOutNotSync("\"fwd-path\":\"[");
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath().begin(); segIt!=(*it)->getPath().end(); segIt++) {
			LogOutNotSync(*segIt <<",");
		}
		LogOutNotSync("]\",");
/*		LogOutNotSync("\"rev-path\":\"[");
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath(false).begin(); segIt!=(*it)->getPath(false).end(); segIt++) {
			LogOutNotSync(*segIt <<",");
		}
		LogOutNotSync("]\",");*/
		LogOutNotSync("})" <<endl);
	}




	//Now print all Segments
	std::set<const Crossing*> cachedCrossings;
	std::set<const BusStop*> cachedBusStops;
	for (std::set<const RoadSegment*>::const_iterator it=cachedSegments.begin(); it!=cachedSegments.end(); it++) {
		LogOutNotSync("(\"road-segment\", 0, " <<*it <<", {");
		LogOutNotSync("\"parent-link\":\"" <<(*it)->getLink() <<"\",");
		LogOutNotSync("\"max-speed\":\"" <<(*it)->maxSpeed <<"\",");
		LogOutNotSync("\"lanes\":\"" <<(*it)->getLanes().size() <<"\",");
		LogOutNotSync("\"from-node\":\"" <<(*it)->getStart() <<"\",");
		LogOutNotSync("\"to-node\":\"" <<(*it)->getEnd() <<"\",");
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			LogOutNotSync((*it)->originalDB_ID.getLogItem());
		}
		LogOutNotSync("})" <<endl);

		if (!(*it)->polyline.empty()) {
			LogOutNotSync("(\"polyline\", 0, " <<&((*it)->polyline) <<", {");
			LogOutNotSync("\"parent-segment\":\"" <<*it <<"\",");
			LogOutNotSync("\"points\":\"[");
			for (vector<Point2D>::const_iterator ptIt=(*it)->polyline.begin(); ptIt!=(*it)->polyline.end(); ptIt++) {
				LogOutNotSync("(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),");
			}
			LogOutNotSync("]\",");
			LogOutNotSync("})" <<endl);
		}

		const std::map<centimeter_t, const RoadItem*>& mapBusStops = (*it)->obstacles;
		for (std::map<centimeter_t, const RoadItem*>::const_iterator itBusStops = mapBusStops.begin(); itBusStops != mapBusStops.end(); ++itBusStops) {
			const RoadItem* ri = itBusStops->second;
			const BusStop* resBS = dynamic_cast<const BusStop*>(ri);
			if (resBS) {
				cachedBusStops.insert(resBS);
			}
		}

		//Save crossing info for later
		const std::map<centimeter_t, const RoadItem*>& mapCrossings = (*it)->obstacles;
		for(std::map<centimeter_t, const RoadItem*>::const_iterator itCrossings = mapCrossings.begin();	itCrossings != mapCrossings.end(); ++itCrossings)
		{
			const RoadItem* ri = itCrossings->second;
			const Crossing* resC = dynamic_cast<const Crossing*>(ri);
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
//		std::cout << "Segment " << (*it)->getSegmentID() << "    getLanes().size() = " << (*it)->getLanes().size() << " Before...";
//		getchar();
		for (size_t laneID=0; laneID<=(*it)->getLanes().size(); laneID++) {
			const vector<Point2D>& points = const_cast<RoadSegment*>(*it)->getLaneEdgePolyline(laneID);
			laneBuffer <<"\"line-" <<laneID <<"\":\"[";
			for (vector<Point2D>::const_iterator ptIt=points.begin(); ptIt!=points.end(); ptIt++) {
				laneBuffer <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
			}
			laneBuffer <<"]\",";

			if (laneID<(*it)->getLanes().size() && (*it)->getLanes()[laneID]->is_pedestrian_lane()) {
				laneBuffer <<"\"line-" <<laneID <<"is-sidewalk\":\"true\",";
			}

		}
//		std::cout << "Segment " << (*it)->getSegmentID() << "    getLanes().size() = " << (*it)->getLanes().size() << " After...";
//		getchar();

		laneBuffer <<"})" <<endl;
		LogOutNotSync(laneBuffer.str());
	}

	//Crossings are part of Segments
	for (std::set<const Crossing*>::iterator it=cachedCrossings.begin(); it!=cachedCrossings.end(); it++) {
		LogOutNotSync("(\"crossing\", 0, " <<*it <<", {");
		LogOutNotSync("\"near-1\":\"" <<(*it)->nearLine.first.getX() <<"," <<(*it)->nearLine.first.getY() <<"\",");
		LogOutNotSync("\"near-2\":\"" <<(*it)->nearLine.second.getX() <<"," <<(*it)->nearLine.second.getY() <<"\",");
		LogOutNotSync("\"far-1\":\"" <<(*it)->farLine.first.getX() <<"," <<(*it)->farLine.first.getY() <<"\",");
		LogOutNotSync("\"far-2\":\"" <<(*it)->farLine.second.getX() <<"," <<(*it)->farLine.second.getY() <<"\",");
		LogOutNotSync("})" <<endl);
	}

	//Bus Stops are part of Segments
	for (std::set<const BusStop*>::iterator it = cachedBusStops.begin(); it != cachedBusStops.end(); it++) {
		LogOutNotSync("(\"busstop\", 0, " <<*it <<", {");
		double x = (*it)->xPos;
		double y = (*it)->yPos;
		int angle = 40;
		double length = 400;
		double width = 250;
		double theta = atan(width / length);
		double phi = M_PI * angle / 180;
		double diagonal_half = (sqrt(length * length + width * width)) / 2;

		double x1d = x + diagonal_half * cos(phi + theta);
		double y1d = y + diagonal_half * sin(phi + theta);
		double x2d = x + diagonal_half * cos(M_PI + phi - theta);
		double y2d = y + diagonal_half * sin(M_PI + phi - theta);
		double x3d = x + diagonal_half * cos(M_PI + phi + theta);
		double y3d = y + diagonal_half * sin(M_PI + phi + theta);
		double x4d = x + diagonal_half * cos(phi - theta);
		double y4d = y + diagonal_half * sin(phi - theta);

		LogOutNotSync("\"near-1\":\""<<std::setprecision(8)<<x<<","<<y<<"\",");
		LogOutNotSync("\"near-2\":\""<<x2d<<","<<y2d<<"\",");
		LogOutNotSync("\"far-1\":\""<<x3d<<","<<y3d<<"\",");
		LogOutNotSync("\"far-2\":\""<<x4d<<","<<y4d<<"\",");
		LogOutNotSync("})" <<endl);
	}


	//Now print all Connectors
	for (std::set<LaneConnector*>::const_iterator it=cachedConnectors.begin(); it!=cachedConnectors.end(); it++) {
		//Retrieve relevant information
		RoadSegment* fromSeg = (*it)->getLaneFrom()->getRoadSegment();
		unsigned int fromLane = (*it)->getLaneFrom()->getLaneID();
		RoadSegment* toSeg = (*it)->getLaneTo()->getRoadSegment();
		unsigned int toLane = (*it)->getLaneTo()->getLaneID();

		//Output
		LogOutNotSync("(\"lane-connector\", 0, " <<*it <<", {");
		LogOutNotSync("\"from-segment\":\"" <<fromSeg <<"\",");
		LogOutNotSync("\"from-lane\":\"" <<fromLane <<"\",");
		LogOutNotSync("\"to-segment\":\"" <<toSeg <<"\",");
		LogOutNotSync("\"to-lane\":\"" <<toLane <<"\",");
		LogOutNotSync("})" <<endl);
	}

	//Print the StreetDirectory graphs.
	StreetDirectory::instance().printDrivingGraph();
	StreetDirectory::instance().printWalkingGraph();
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
		  getchar();
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

void PrintDB_Network_ptrBased()
{
	if (ConfigParams::GetInstance().OutputDisabled()) {
		return;
	}

	//Save RoadSegments/Connectors to make output simpler
	std::set<const RoadSegment*,Sorter> cachedSegments;
	std::set<LaneConnector*,Sorter> cachedConnectors;

	//Initial message
	const RoadNetwork& rn = ConfigParams::GetInstance().getNetwork();
	LogOutNotSync("Printing node network" <<endl);
	LogOutNotSync("NOTE: All IDs in this section are consistent for THIS simulation run, but will change if you run the simulation again." <<endl);

	//Print some properties of the simulation itself
	LogOutNotSync("(\"simulation\", 0, 0, {");
	LogOutNotSync("\"frame-time-ms\":\"" <<ConfigParams::GetInstance().baseGranMS <<"\",");
	LogOutNotSync("})" <<endl);


	sim_mob::Signal::all_signals_const_Iterator it;
	for (it = sim_mob::Signal::all_signals_.begin(); it!= sim_mob::Signal::all_signals_.end(); it++)

#if 0
	for (std::vector<Signal*>::const_iterator it=Signal::all_signals_.begin(); it!=Signal::all_signals_.end(); it++)
#endif
	//Print the Signal representation.
	{
		LogOutNotSync((*it)->toString() <<endl);
	}


	//Print nodes first
	for (set<UniNode*>::const_iterator it=rn.getUniNodes().begin(); it!=rn.getUniNodes().end(); it++) {
		LogOutNotSync("(\"uni-node\", 0, " <<*it <<", {");
		LogOutNotSync("\"xPos\":\"" <<(*it)->location.getX() <<"\",");
		LogOutNotSync("\"yPos\":\"" <<(*it)->location.getY() <<"\",");
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			LogOutNotSync((*it)->originalDB_ID.getLogItem());
		}
		LogOutNotSync("})" <<endl);

		//Cache all segments
		vector<const RoadSegment*> segs = (*it)->getRoadSegments();
		for (vector<const RoadSegment*>::const_iterator i2=segs.begin(); i2!=segs.end(); ++i2) {
			cachedSegments.insert(*i2);
		}
	}

	for (vector<MultiNode*>::const_iterator it=rn.getNodes().begin(); it!=rn.getNodes().end(); it++) {
		LogOutNotSync("(\"multi-node\", 0, " <<*it <<", {");
		LogOutNotSync("\"xPos\":\"" <<(*it)->location.getX() <<"\",");
		LogOutNotSync("\"yPos\":\"" <<(*it)->location.getY() <<"\",");
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			LogOutNotSync((*it)->originalDB_ID.getLogItem());
		}
		LogOutNotSync("})" <<endl);

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
		LogOutNotSync("(\"link\", 0, " <<*it <<", {");
		LogOutNotSync("\"road-name\":\"" <<(*it)->roadName <<"\",");
		LogOutNotSync("\"start-node\":\"" <<(*it)->getStart() <<"\",");
		LogOutNotSync("\"end-node\":\"" <<(*it)->getEnd() <<"\",");
		LogOutNotSync("\"fwd-path\":\"[");
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath().begin(); segIt!=(*it)->getPath().end(); segIt++) {
			LogOutNotSync(*segIt <<",");
		}
		LogOutNotSync("]\",");
/*		LogOutNotSync("\"rev-path\":\"[");
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath(false).begin(); segIt!=(*it)->getPath(false).end(); segIt++) {
			LogOutNotSync(*segIt <<",");
		}
		LogOutNotSync("]\",");*/
		LogOutNotSync("})" <<endl);
	}


	//Now print all Segments
	std::set<const Crossing*,Sorter> cachedCrossings;
	std::set<const BusStop*,Sorter> cachedBusStops;
		for (std::set<const RoadSegment*>::const_iterator it=cachedSegments.begin(); it!=cachedSegments.end(); it++) {
		LogOutNotSync("(\"road-segment\", 0, " <<*it <<", {");
		LogOutNotSync("\"parent-link\":\"" <<(*it)->getLink() <<"\",");
		LogOutNotSync("\"max-speed\":\"" <<(*it)->maxSpeed <<"\",");
		LogOutNotSync("\"width\":\"" <<(*it)->width <<"\",");
		LogOutNotSync("\"lanes\":\"" <<(*it)->getLanes().size() <<"\",");
		LogOutNotSync("\"from-node\":\"" <<(*it)->getStart() <<"\",");
		LogOutNotSync("\"to-node\":\"" <<(*it)->getEnd() <<"\",");
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			LogOutNotSync((*it)->originalDB_ID.getLogItem());
		}
		LogOutNotSync("})" <<endl);

		if (!(*it)->polyline.empty()) {
			LogOutNotSync("(\"polyline\", 0, " <<&((*it)->polyline) <<", {");
			LogOutNotSync("\"parent-segment\":\"" <<*it <<"\",");
			LogOutNotSync("\"points\":\"[");
			for (vector<Point2D>::const_iterator ptIt=(*it)->polyline.begin(); ptIt!=(*it)->polyline.end(); ptIt++) {
				LogOutNotSync("(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),");
			}
			LogOutNotSync("]\",");
			LogOutNotSync("})" <<endl);
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
			}

		}

		laneBuffer <<"})" <<endl;
		LogOutNotSync(laneBuffer.str());
	}

	//Crossings are part of Segments
	for (std::set<const Crossing*>::iterator it=cachedCrossings.begin(); it!=cachedCrossings.end(); it++) {
		LogOutNotSync("(\"crossing\", 0, " <<*it <<", {");
		LogOutNotSync("\"near-1\":\"" <<(*it)->nearLine.first.getX() <<"," <<(*it)->nearLine.first.getY() <<"\",");
		LogOutNotSync("\"near-2\":\"" <<(*it)->nearLine.second.getX() <<"," <<(*it)->nearLine.second.getY() <<"\",");
		LogOutNotSync("\"far-1\":\"" <<(*it)->farLine.first.getX() <<"," <<(*it)->farLine.first.getY() <<"\",");
		LogOutNotSync("\"far-2\":\"" <<(*it)->farLine.second.getX() <<"," <<(*it)->farLine.second.getY() <<"\",");
		LogOutNotSync("})" <<endl);
	}

	//Bus Stops are part of Segments
	for (std::set<const BusStop*>::iterator it = cachedBusStops.begin(); it != cachedBusStops.end(); it++) {
		LogOutNotSync("(\"busstop\", 0, " <<*it <<", {");
		double x = (*it)->xPos;
		double y = (*it)->yPos;
		int angle = 40;
		double length = 400;
		double width = 250;
		double theta = atan(width / length);
		double phi = M_PI * angle / 180;
		double diagonal_half = (sqrt(length * length + width * width)) / 2;

		double x1d = x + diagonal_half * cos(phi + theta);
		double y1d = y + diagonal_half * sin(phi + theta);
		double x2d = x + diagonal_half * cos(M_PI + phi - theta);
		double y2d = y + diagonal_half * sin(M_PI + phi - theta);
		double x3d = x + diagonal_half * cos(M_PI + phi + theta);
		double y3d = y + diagonal_half * sin(M_PI + phi + theta);
		double x4d = x + diagonal_half * cos(phi - theta);
		double y4d = y + diagonal_half * sin(phi - theta);

		LogOutNotSync("\"near-1\":\""<<std::setprecision(8)<<x<<","<<y<<"\",");
		LogOutNotSync("\"near-2\":\""<<x2d<<","<<y2d<<"\",");
		LogOutNotSync("\"far-1\":\""<<x3d<<","<<y3d<<"\",");
		LogOutNotSync("\"far-2\":\""<<x4d<<","<<y4d<<"\",");
		LogOutNotSync("})" <<endl);
	}


	//Now print all Connectors
	for (std::set<LaneConnector*>::const_iterator it=cachedConnectors.begin(); it!=cachedConnectors.end(); it++) {
		//Retrieve relevant information
		RoadSegment* fromSeg = (*it)->getLaneFrom()->getRoadSegment();
		unsigned int fromLane = std::distance(fromSeg->getLanes().begin(), std::find(fromSeg->getLanes().begin(), fromSeg->getLanes().end(),(*it)->getLaneFrom()));
		RoadSegment* toSeg = (*it)->getLaneTo()->getRoadSegment();
		unsigned int toLane = std::distance(toSeg->getLanes().begin(), std::find(toSeg->getLanes().begin(), toSeg->getLanes().end(),(*it)->getLaneTo()));

		//Output
		LogOutNotSync("(\"lane-connector\", 0, " <<*it <<", {");
		LogOutNotSync("\"from-segment\":\"" <<fromSeg <<"\",");
		LogOutNotSync("\"from-lane\":\"" <<fromLane <<"\",");
		LogOutNotSync("\"to-segment\":\"" <<toSeg <<"\",");
		LogOutNotSync("\"to-lane\":\"" <<toLane <<"\",");
		LogOutNotSync("})" <<endl);
	}

	//Print the StreetDirectory graphs.
	StreetDirectory::instance().printDrivingGraph();
	StreetDirectory::instance().printWalkingGraph();

}


//NOTE: We guarantee that the log file contains data in the order it will be needed. So, Nodes are listed
//      first because Links need Nodes. Otherwise, the output will be in no guaranteed order.
void PrintDB_Network_idBased()
{
	if (ConfigParams::GetInstance().OutputDisabled()) {
		return;
	}

	//Save RoadSegments/Connectors to make output simpler
	std::set<const RoadSegment*,Sorter> cachedSegments;
	std::set<LaneConnector*,Sorter> cachedConnectors;

	//Initial message
	const RoadNetwork& rn = ConfigParams::GetInstance().getNetwork();
	LogOutNotSync("Printing node network" <<endl);
	LogOutNotSync("NOTE: All IDs in this section are consistent for THIS simulation run, but will change if you run the simulation again." <<endl);

	//Print some properties of the simulation itself
	LogOutNotSync("(\"simulation\", 0, 0, {");
	LogOutNotSync("\"frame-time-ms\":\"" <<ConfigParams::GetInstance().baseGranMS <<"\",");
	LogOutNotSync("})" <<endl);
	//Print nodes first
	for (set<UniNode*>::const_iterator it=rn.getUniNodes().begin(); it!=rn.getUniNodes().end(); it++) {
		LogOutNotSync("(\"uni-node\", 0, " <<(*it)->getID() <<", {");
		LogOutNotSync("\"xPos\":\"" <<(*it)->location.getX() <<"\",");
		LogOutNotSync("\"yPos\":\"" <<(*it)->location.getY() <<"\",");
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			LogOutNotSync((*it)->originalDB_ID.getLogItem());
		}
		LogOutNotSync("})" <<endl);

		//Cache all segments
		vector<const RoadSegment*> segs = (*it)->getRoadSegments();
		for (vector<const RoadSegment*>::const_iterator i2=segs.begin(); i2!=segs.end(); ++i2) {
			cachedSegments.insert(*i2);
		}
	}

	for (vector<MultiNode*>::const_iterator it=rn.getNodes().begin(); it!=rn.getNodes().end(); it++) {
		LogOutNotSync("(\"multi-node\", 0, " <<(*it)->getID() <<", {");
		LogOutNotSync("\"xPos\":\"" <<(*it)->location.getX() <<"\",");
		LogOutNotSync("\"yPos\":\"" <<(*it)->location.getY() <<"\",");
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			LogOutNotSync((*it)->originalDB_ID.getLogItem());
		}
		LogOutNotSync("})" <<endl);

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
		LogOutNotSync("(\"link\", 0, " <<(*it)->getLinkId() <<", {");
		LogOutNotSync("\"road-name\":\"" <<(*it)->roadName <<"\",");
		LogOutNotSync("\"start-node\":\"" <<(*it)->getStart()->getID() <<"\",");
		LogOutNotSync("\"end-node\":\"" <<(*it)->getEnd()->getID() <<"\",");
		LogOutNotSync("\"fwd-path\":\"[");
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath().begin(); segIt!=(*it)->getPath().end(); segIt++) {
			LogOutNotSync((*segIt)->getSegmentID() <<",");
		}
		LogOutNotSync("]\",");
/*		LogOutNotSync("\"rev-path\":\"[");
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath(false).begin(); segIt!=(*it)->getPath(false).end(); segIt++) {
			LogOutNotSync((*segIt)->getSegmentID() <<",");
		}
		LogOutNotSync("]\",");*/
		LogOutNotSync("})" <<endl);
	}




	//Now print all Segments
	std::set<const Crossing*,Sorter> cachedCrossings;
	std::set<const BusStop*,Sorter> cachedBusStops;
	int i = 0;

	for (std::set<const RoadSegment*>::const_iterator it=cachedSegments.begin(); it!=cachedSegments.end(); it++) {
		LogOutNotSync("(\"road-segment\", 0, " <<(*it)->getSegmentID() <<", {");
		LogOutNotSync("\"parent-link\":\"" <<(*it)->getLink()->getLinkId() <<"\",");
		LogOutNotSync("\"max-speed\":\"" <<(*it)->maxSpeed <<"\",");
		LogOutNotSync("\"width\":\"" <<(*it)->width <<"\",");
		LogOutNotSync("\"lanes\":\"" <<(*it)->getLanes().size() <<"\",");
		LogOutNotSync("\"from-node\":\"" <<(*it)->getStart()->getID() <<"\",");
		LogOutNotSync("\"to-node\":\"" <<(*it)->getEnd()->getID() <<"\",");
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			LogOutNotSync((*it)->originalDB_ID.getLogItem());
		}
		LogOutNotSync("})" <<endl);

		if (!(*it)->polyline.empty()) {
			LogOutNotSync("(\"polyline\", 0, " /*<<&((*it)->polyline)*/ <<", {");
			LogOutNotSync("\"parent-segment\":\"" <<(*it)->getSegmentID() <<"\",");
			LogOutNotSync("\"points\":\"[");
			for (vector<Point2D>::const_iterator ptIt=(*it)->polyline.begin(); ptIt!=(*it)->polyline.end(); ptIt++) {
				LogOutNotSync("(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),");
			}
			LogOutNotSync("]\",");
			LogOutNotSync("})" <<endl);
		}


		const std::map<centimeter_t, const RoadItem*>& mapBusStops = (*it)->obstacles;
		for(std::map<centimeter_t, const RoadItem*>::const_iterator itBusStops = mapBusStops.begin(); itBusStops != mapBusStops.end(); ++itBusStops) {
			const RoadItem* ri = itBusStops->second;
			const BusStop* resBS = dynamic_cast<const BusStop*>(ri);
			if (resBS) {
				cachedBusStops.insert(resBS);
			}
		}

//TODO: add other types of obstacle.
		//Save crossing info for later
		const std::map<centimeter_t, const RoadItem*>& mapCrossings = (*it)->obstacles;
		for(std::map<centimeter_t, const RoadItem*>::const_iterator itCrossings = mapCrossings.begin();	itCrossings != mapCrossings.end(); ++itCrossings) {
			const Crossing* resC = dynamic_cast<const Crossing*>(itCrossings->second);
				if (resC) {
				cachedCrossings.insert(resC);
			}
		}

		//Save Lane info for later
		//NOTE: For now this relies on somewhat sketchy behavior, which is why we output a "tmp-*"
		//      flag. Once we add auto-polyline generation, that tmp- output will be meaningless
		//      and we can switch to a full "lane" output type.
		std::stringstream laneBuffer; //Put it in its own buffer since getLanePolyline() can throw.
		laneBuffer <<"(\"lane\", 0, " <</*&((*it)->getLanes()) <<*/", {";
		laneBuffer <<"\"parent-segment\":\"" <<(*it)->getSegmentID() <<"\",";
		size_t laneID=0;

		laneID = 0;
		for (std::vector<sim_mob::Lane*>::const_iterator it_l = (*it)->getLanes().begin(); it_l != (*it)->getLanes().end(); it_l++, laneID++) {

			laneBuffer <<"\"lane-" <<(*it_l)->getLaneID() <<"\":\"[";
			const vector<Point2D>& points =(*it_l)->getPolyline(false);

			for (vector<Point2D>::const_iterator ptIt=points.begin(); ptIt!=points.end(); ptIt++) {
				laneBuffer <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
			}
			laneBuffer <<"]\",width: " << (*it_l)->getWidth_real();

			if ((*it_l)->is_pedestrian_lane()) {
				laneBuffer <<"\"line-" <<(*it_l)->getLaneID() <<"is-sidewalk\":\"true\",";
			}
		}
		laneBuffer <<"})" <<endl;
		LogOutNotSync(laneBuffer.str());
	}

	//Crossings are part of Segments
	for (std::set<const Crossing*>::iterator it=cachedCrossings.begin(); it!=cachedCrossings.end(); it++) {
		LogOutNotSync("(\"crossing\", 0, " <<(*it)->getRoadItemID() <<", {");
		LogOutNotSync("\"near-1\":\"" <<(*it)->nearLine.first.getX() <<"," <<(*it)->nearLine.first.getY() <<"\",");
		LogOutNotSync("\"near-2\":\"" <<(*it)->nearLine.second.getX() <<"," <<(*it)->nearLine.second.getY() <<"\",");
		LogOutNotSync("\"far-1\":\"" <<(*it)->farLine.first.getX() <<"," <<(*it)->farLine.first.getY() <<"\",");
		LogOutNotSync("\"far-2\":\"" <<(*it)->farLine.second.getX() <<"," <<(*it)->farLine.second.getY() <<"\",");
		LogOutNotSync("})" <<endl);
	}

	//Bus Stops are part of Segments
		for (std::set<const BusStop*>::iterator it=cachedBusStops.begin(); it!=cachedBusStops.end(); it++) {
			//LogOutNotSync("Surav's loop  is here!");
		LogOutNotSync("(\"busstop\", 0, " <</*(*it) <<*/", {");
		//	LogOutNotSync("\"bus stop id\":\"" <<(*it)->busstopno_<<"\",");
			// LogOutNotSync("\"xPos\":\"" <<(*it)->xPos<<"\",");
		//	LogOutNotSync("\"yPos\":\"" <<(*it)->yPos<<"\",");
		double x = (*it)->xPos;
		double y = (*it)->yPos;
		int angle = 40;
			                                double length = 400;
							        		double width = 250;
							        		double theta = atan(width/length);
							        		double phi = M_PI*angle/180;
							                double diagonal_half = (sqrt(length*length + width*width))/2;

							        		double x1d = x + diagonal_half*cos(phi+theta);
							        		double y1d = y + diagonal_half*sin(phi+theta);
							        		double x2d = x + diagonal_half*cos(M_PI+phi-theta);
							        		double y2d = y + diagonal_half*sin(M_PI+phi-theta);
							        		double x3d = x + diagonal_half*cos(M_PI+phi+theta);
							        		double y3d = y + diagonal_half*sin(M_PI+phi+theta);
							        		double x4d = x + diagonal_half*cos(phi-theta);
							        		double y4d = y + diagonal_half*sin(phi-theta);
			LogOutNotSync("\"near-1\":\""<<std::setprecision(8)<<x<<","<<y<<"\",");
			LogOutNotSync("\"near-2\":\""<<x2d<<","<<y2d<<"\",");
			LogOutNotSync("\"far-1\":\""<<x3d<<","<<y3d<<"\",");
			LogOutNotSync("\"far-2\":\""<<x4d<<","<<y4d<<"\",");
			LogOutNotSync("})" <<endl);
		}


	//Now print all Connectors
	for (std::set<LaneConnector*>::const_iterator it=cachedConnectors.begin(); it!=cachedConnectors.end(); it++) {
		//Retrieve relevant information
		RoadSegment* fromSeg = (*it)->getLaneFrom()->getRoadSegment();
		unsigned int fromLane = (*it)->getLaneFrom()->getLaneID();
		RoadSegment* toSeg = (*it)->getLaneTo()->getRoadSegment();
		unsigned int toLane = (*it)->getLaneTo()->getLaneID();

		//Output
		LogOutNotSync("(\"lane-connector\", 0, " /*<<(*it)*/ <<", {");
		LogOutNotSync("\"from-segment\":\"" <<fromSeg->getSegmentID() <<"\",");
		LogOutNotSync("\"from-lane\":\"" <<fromLane <<"\",");
		LogOutNotSync("\"to-segment\":\"" <<toSeg->getSegmentID() <<"\",");
		LogOutNotSync("\"to-lane\":\"" <<toLane <<"\",");
		LogOutNotSync("})" <<endl);
	}

}

void patchRoadNetworkwithLaneEdgePolyline() {
	//Initial message
	const RoadNetwork& rn = sim_mob::ConfigParams::GetInstance().getNetwork();
	std::set<const RoadSegment*, Sorter> cachedSegments;
	for (set<UniNode*>::const_iterator it = rn.getUniNodes().begin(); it != rn.getUniNodes().end(); it++) {
		//Cache all segments
		vector<const RoadSegment*> segs = (*it)->getRoadSegments();
		for (vector<const RoadSegment*>::const_iterator i2 = segs.begin(); i2 != segs.end(); ++i2) {
			cachedSegments.insert(*i2);
		}
	}
	for (vector<MultiNode*>::const_iterator it = rn.getNodes().begin(); it != rn.getNodes().end(); it++) {
		const Intersection* nodeInt = static_cast<const Intersection*>((*it));
		//Cache all segments
		for (set<RoadSegment*>::const_iterator i2 = nodeInt->getRoadSegments().begin(); i2 != nodeInt->getRoadSegments().end(); ++i2) {
			cachedSegments.insert(*i2);
		}
	}
	for (std::set<const RoadSegment*>::const_iterator it = cachedSegments.begin(); it != cachedSegments.end(); it++) {
		for (size_t laneID = 0; laneID <= (*it)->getLanes().size(); laneID++) {
			const vector<Point2D>& points = const_cast<RoadSegment*>(*it)->getLaneEdgePolyline(laneID);
		}
	}
}

void printRoadNetwork_console()
{
	int sum_segments = 0, sum_lane = 0, sum_lanes = 0;
	std::cout << "Testin Road Network :\n";

	std::vector<Link*>  & links = ConfigParams::GetInstance().getNetworkRW().getLinks();
	std::cout << "\n\n\nList of Links\n";
	for(std::vector<Link*>::iterator it = links.begin(); it != links.end(); it++)
	{
		std::cout << "LinkId: " << (*it)->getLinkId() << std::endl;
	}
	for(std::vector<Link*>::iterator it = links.begin(); it != links.end(); it++)
	{
		std::cout << "\n\n\nNumber of Segments in Link[" << (*it)->getLinkId() << "]=> " << (*it)->getUniqueSegments().size() << std::endl << std::endl;
		sum_lane = 0;
		std::cout << "Segments:\n";

		for(std::vector<sim_mob::RoadSegment*>::const_iterator it_seg = (*it)->getSegments().begin(); it_seg != (*it)->getSegments().end(); it_seg++)
		{
			std::cout << "SegmentId: " << (*it_seg)->getSegmentID() << " NOF polypoints: " << (*it_seg)->polyline.size() << std::endl;
			std::cout << "	Number of lanes in segment[" << (*it_seg)->getSegmentID() << "]=> " << (*it_seg)->getLanes().size() << ":" << std::endl;
			std::vector<sim_mob::Lane*>& tmpLanes = const_cast<std::vector<sim_mob::Lane*>&>((*it_seg)->getLanes());
			std::sort(tmpLanes.begin(), tmpLanes.end(), sorter_);
			for(std::vector<sim_mob::Lane*>::const_iterator lane_it = tmpLanes.begin() ;  lane_it != tmpLanes.end() ; lane_it++)
			{
				std::cout << "		laneId: " << 	(*lane_it)->getLaneID()  << " NOF polypoints: " << (*lane_it)->polyline_.size() << std::endl;
			}
			sum_lane += (*it_seg)->getLanes().size();
		}
/*		std::cout << "\n\nBackward Segments:\n";
		for(std::vector<sim_mob::RoadSegment*>::const_iterator it_seg = (*it)->getRevSegments().begin(); it_seg != (*it)->getRevSegments().end(); it_seg++)
		{
			std::cout << "SegmentId: " << (*it_seg)->getSegmentID() << " NOF polypoints: " << (*it_seg)->polyline.size() << std::endl;
			std::cout << "	Number of lanes in segment[" << (*it_seg)->getSegmentID() << "]=> " << (*it_seg)->getLanes().size() << ":" << std::endl;
			std::vector<sim_mob::Lane*>& tmpLanes = const_cast<std::vector<sim_mob::Lane*>&>((*it_seg)->getLanes());
			std::sort(tmpLanes.begin(), tmpLanes.end(), sorter_);
			for(std::vector<sim_mob::Lane*>::const_iterator lane_it = tmpLanes.begin() ;  lane_it != tmpLanes.end() ; lane_it++)
			{
				std::cout << "		laneId: " << 	(*lane_it)->getLaneID_str()  << " NOF polypoints: " << (*lane_it)->polyline_.size() << std::endl;
			}
			sum_lane += (*it_seg)->getLanes().size();
		}*/
		std::cout << "\n\n\n\n";
		sum_segments += (*it)->getUniqueSegments().size();

		std::cout << "Total Number of Lanes in this Link: " << sum_lane << std::endl << std::endl;
		sum_lanes += sum_lane;
		std::cout << "--------------------------------------------------------\n";
	}


	int temp_rs_cnt = 0;
	for(std::vector<sim_mob::MultiNode*>::const_iterator it = ConfigParams::GetInstance().getNetwork().getNodes().begin() , it_end(ConfigParams::GetInstance().getNetwork().getNodes().end()); it != it_end; it++)
	{
		std::cout << "\n\nConnectors for Node : " << (*it)->getID() << " has connectors for " << (*it)->getConnectors().size() << " segments: \n";
		temp_rs_cnt += (*it)->getConnectors().size();
		for(std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::const_iterator it_cnn = (*it)->getConnectors().begin();it_cnn != (*it)->getConnectors().end() ;it_cnn++ )
		{
			std::cout << "     RoadSegment " << (*it_cnn).first->getSegmentID() << " has " << (*it_cnn).second.size() << " connectors:\n";
			const std::set<sim_mob::LaneConnector*> & tempLC = /*const_cast<std::set<sim_mob::LaneConnector*>& >*/((*it_cnn).second);
			std::set<sim_mob::LaneConnector *, Sorter> s;//(tempLC.begin(), tempLC.end(),MyLaneConectorSorter());
			for(std::set<sim_mob::LaneConnector*>::iterator it = tempLC.begin(); it != tempLC.end(); it++)
			{
				s.insert(*it);
			}
			for(std::set<sim_mob::LaneConnector*>::iterator it_lc = s.begin(); it_lc != s.end(); it_lc++)
			{
				std::cout << "       From [" << (*it_lc)->getLaneFrom()->getRoadSegment()->getLink()->getLinkId() << ":" << (*it_lc)->getLaneFrom()->getRoadSegment()->getSegmentID() << ":" << (*it_lc)->getLaneFrom()->getLaneID() << "]   to   [" << (*it_lc)->getLaneTo()->getRoadSegment()->getLink()->getLinkId() << ":" << (*it_lc)->getLaneTo()->getRoadSegment()->getSegmentID() << ":"  << (*it_lc)->getLaneTo()->getLaneID() << "]\n";
			}
			std::cout << "\n";
		}
	}
	std::cout << "Total Number rs for mn connectors: " << temp_rs_cnt << std::endl << std::endl;


	std::cout << "Total Number of Links: " << links.size() << std::endl;
	std::cout << "Total Number of Segments : " << sum_segments << std::endl;
	std::cout << "Total Number of Lanes : " << sum_lanes << std::endl;
	std::cout << "\n\nTotal Number of Segment Nodes : " <<ConfigParams::GetInstance().getNetworkRW().getNodes().size() << std::endl;
	std::cout << "Total Number of UniNodes : " <<ConfigParams::GetInstance().getNetworkRW().getUniNodes().size() << std::endl;


	std::cout << "Testing Road Network Done\n";
//	getchar();
}


//Returns the error message, or an empty string if no error.
std::string loadXMLConf(TiXmlDocument& document, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, ProfileBuilder* prof)
{
	std::string XML_OutPutFileName = "data/SimMobilityInput.xml";
	//std::string XML_OutPutFileName = "data/XML_OutPut.xml";

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
	int agentWgSize = ReadValue(handle, "agent");
	int signalWgSize = ReadValue(handle, "signal");

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
			return string("Unknown mutex strategy: ") + mutexStrat;
		}
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

	//Misc.: disable dynamic dispatch?
	handle = TiXmlHandle(&document);
	node = handle.FirstChild("config").FirstChild("system").FirstChild("misc").FirstChild("disable_dynamic_dispatch").ToElement();
	if (node) {
		const char* valStr_c = node->Attribute("value");
		if (valStr_c) {
			std::string valStr(valStr_c);
			if (valStr == "true") {
				ConfigParams::GetInstance().dynamicDispatchDisabled = true;
			} else if (valStr == "false") {
				ConfigParams::GetInstance().dynamicDispatchDisabled = false;
			} else {
				return "Invalid parameter; expecting boolean.";
			}
		}
	}

	std::cout <<"Dynamic dispatch: " <<(ConfigParams::GetInstance().dynamicDispatchDisabled ? "DISABLED" : "Enabled") <<std::endl;


	//Series of one-line checks.
	if(baseGran == -1) { return "Config file fails to specify base granularity."; }
	if(totalRuntime == -1) { return "Config file fails to specify total runtime."; }
	if(totalWarmup == -1) { return "Config file fails to specify total warmup."; }
	if(granAgent == -1) { return "Config file fails to specify agent granularity."; }
	if(granSignal == -1) { return "Config file fails to specify signal granularity."; }
	if(agentWgSize == -1) { return "Config file fails to specify agent workgroup size."; }
	if(signalWgSize == -1) { return "Config file fails to specify signal workgroup size."; }
	if (!simStartStr) { return "Config file fails to specify simulation start time."; }

    //Granularity check
    if (granAgent < baseGran) return "Agent granularity cannot be smaller than base granularity.";
    if (granAgent%baseGran != 0) {
    	return "Agent granularity not a multiple of base granularity.";
    }
    if (granSignal < baseGran) return "Signal granularity cannot be smaller than base granularity.";
    if (granSignal%baseGran != 0) {
    	return "Signal granularity not a multiple of base granularity.";
    }
    if (granPaths < baseGran) return "Path granularity cannot be smaller than base granularity.";
    if (granPaths%baseGran != 0) {
    	return "Path granularity not a multiple of base granularity.";
    }
    if (granDecomp < baseGran) return "Decomposition granularity cannot be smaller than base granularity.";
    if (granDecomp%baseGran != 0) {
    	return "Decomposition granularity not a multiple of base granularity.";
    }
    if (totalRuntime < baseGran) return "Total Runtime cannot be smaller than base granularity.";
    if (totalRuntime%baseGran != 0) {
    	std::cout <<"  Warning! Total Runtime will be truncated.\n";
    }
    if (totalWarmup != 0 && totalWarmup < baseGran) std::cout << "Warning! Total Warmup is smaller than base granularity.\n";
    if (totalWarmup%baseGran != 0) {
    	std::cout <<"  Warning! Total Warmup will be truncated.\n";
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
    	config.simStartTime = DailyTime(simStartStr);
    	config.mutexStategy = mtStrat;
    	config.signalTimingMode = signalTimingMode;

    	//add for MPI
#ifndef SIMMOB_DISABLE_MPI
    	if (config.is_run_on_many_computers) {
			sim_mob::PartitionManager& partitionImpl = sim_mob::PartitionManager::instance();
			std::cout << "partition_solution_id in configuration:" << partition_solution_id << std::endl;

			partitionImpl.partition_config->partition_solution_id = partition_solution_id;
    	}
#endif

    }


    //Check the type of geometry
    handle = TiXmlHandle(&document);
    TiXmlElement* geomElem = handle.FirstChild("config").FirstChild("geometry").ToElement();
    if (geomElem) {
    	const char* geomType = geomElem->Attribute("type");
    	if (geomType && string(geomType) == "simple") {
    		//Load boundaries
    		if (!LoadXMLBoundariesCrossings(document, "boundaries", "boundary", ConfigParams::GetInstance().boundaries)) {
    			return "Couldn't load boundaries";
    		}

    		//Load crossings
    		if (!LoadXMLBoundariesCrossings(document, "crossings", "crossing", ConfigParams::GetInstance().crossings)) {
    			return "Couldn't load crossings";
    		}
    	} else if (geomType && string(geomType) == "aimsun") {
    		//Ensure we're loading from a database
    		if (prof) { prof->logGenericStart("Database", "main-prof"); }
    		const char* geomSrc = geomElem->Attribute("source");
    		if (!geomSrc || "database" != string(geomSrc)) {
    			return "Unknown geometry source: " + (geomSrc?string(geomSrc):"");
    		}

    		/**************************************************
    		 *
    		 * ***********  DATABASE **************************
    		 *
    		 ***************************************************/
#ifndef SIMMOB_XML_READER
    		//Load the AIMSUM network details
    		map<string, string> storedProcedures; //Of the form "node" -> "get_node()"
    		if (!LoadDatabaseDetails(*geomElem, ConfigParams::GetInstance().connectionString, storedProcedures)) {
    			return "Unable to load database connection settings....";
    		}

    		//Actually load it
    		string dbErrorMsg = sim_mob::aimsun::Loader::LoadNetwork(ConfigParams::GetInstance().connectionString, storedProcedures, ConfigParams::GetInstance().getNetworkRW(), ConfigParams::GetInstance().getTripChains(), prof);
    		if (!dbErrorMsg.empty()) {
    			return "Database loading error: " + dbErrorMsg;
    		}

#else
       		/**************************************************
       		 *
       		 * ****************  XML-READER *******************
        	 *
        	 *************************************************/
//#ifdef SIMMOB_PARTIAL_XML_READER
    		if (!sim_mob::xml::InitAndLoadXML(XML_OutPutFileName, ConfigParams::GetInstance().getNetworkRW())) {
    			throw std::runtime_error("Error loading/parsing XML file (see stderr).");
    		}
//#else
 //   		geo::InitAndLoadXML(XML_OutPutFileName);
//#endif
    		//Re-enable if you need diagnostic information. ~Seth
    		//runXmlChecks(ConfigParams::GetInstance().getNetwork().getLinks());
#endif
//////////////////////////////////////////////////////////////////////////////////
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
    		return "Unknown geometry type: " + (geomType?string(geomType):"");
    	}
    }

    //TEMP: Test network output via boost.
    BoostSaveXML("NetworkCopy.xml", ConfigParams::GetInstance().getNetworkRW());


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
    patchRoadNetworkwithLaneEdgePolyline();//apparently, must be called before StreetDirectory.init

    std::cout << "Street Directory initialized" << std::endl;
    StreetDirectory::instance().init(ConfigParams::GetInstance().getNetwork(), true);

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
    	return "Couldn't load buscontrollers";
    }

    //Initialize all BusControllers.
	if(BusController::HasBusControllers()) {
		BusController::InitializeAllControllers(active_agents, ConfigParams::GetInstance().getPT_bus_dispatch_freq());
	}

    //Load Agents, Pedestrians, and Trip Chains as specified in loadAgentOrder
    for (vector<string>::iterator it=loadAgentOrder.begin(); it!=loadAgentOrder.end(); it++) {
    	if (((*it) == "database")||((*it) == "xml-tripchains")) {
    	    	 //Create an agent for each Trip Chain in the database.
    	    	    generateAgentsFromTripChain(active_agents, pending_agents, constraints);
    	    	    cout <<"Loaded Database Agents (from Trip Chains)." <<endl;
    	    	} else if ((*it) == "drivers") {
    	    if (!loadXMLAgents(document, active_agents, pending_agents, "driver", constraints)) {
    	    	return	 "Couldn't load drivers";
    	    }
    	    if (!loadXMLAgents(document, active_agents, pending_agents, "busdriver", constraints)) {
    	    	return	 "Couldn't load bus drivers";
    	    }
    		cout <<"Loaded Driver Agents (from config file)." <<endl;

    	} else if ((*it) == "pedestrians") {
    		if (!loadXMLAgents(document, active_agents, pending_agents, "pedestrian", constraints)) {
    			return "Couldn't load pedestrians";
    		}
    		cout <<"Loaded Pedestrian Agents (from config file)." <<endl;
    	} else {
    		return string("Unknown item in load_agents: ") + (*it);
    	}
    }
    std::cout << "Loading Agents, Pedestrians, and Trip Chains as specified in loadAgentOrder Success!" << std::endl;

    //Load signals, which are currently agents
    if (!loadXMLSignals(document,
#if 0
    		Signal::all_signals_,
#endif
    		"signal")) {
    	std::cout << "loadXMLSignals Failed!" << std::endl;
    	return	 "Couldn't load signals";
    }



    //Display
    std::cout <<"Config parameters:\n";
    std::cout <<"------------------\n";
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
    	PrintDB_Network_ptrBased();
    	std::cout <<"------------------\n";
   // }
    std::cout <<"  Agents Initialized: " <<Agent::all_agents.size() <<"\n";
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

	//No error
	return "";
}



} //End anon namespace



//////////////////////////////////////////
// Simple singleton implementation
//////////////////////////////////////////
ConfigParams sim_mob::ConfigParams::instance;

//////////////////////////////////////////
// Main external method
//////////////////////////////////////////

bool sim_mob::ConfigParams::InitUserConf(const string& configPath, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, ProfileBuilder* prof, const Config::BuiltInModels& builtInModels)
{
	//Load a (test) configuration in parallel
	Config cfg;
	cfg.InitBuiltInModels(builtInModels);
	if (sim_mob::xml::InitAndLoadConfigXML("data/simrun_seth.xml", cfg)) {
		std::cout<<"New config XML loader succeeded.\n";
	} else {
		throw std::runtime_error("New config XML loader failed.");
	}

	//Load our config file into an XML document object.
	//NOTE: Do *not* use by-value syntax for doc. For some reason, this crashes OSX.
	TiXmlDocument* doc = new TiXmlDocument(configPath);

	if (prof) { prof->logGenericStart("XML", "main-prof-xml"); }
	if (!doc->LoadFile()) {
		std::cout <<"Error loading config file: " <<doc->ErrorDesc() <<std::endl;
		delete doc;
		return false;
	}
	if (prof) { prof->logGenericEnd("XML", "main-prof-xml"); }

	//Parse it
	string errorMsg = loadXMLConf(*doc, active_agents, pending_agents, prof);
	delete doc;

	if (errorMsg.empty()) {
		std::cout <<"XML config file loaded.\nConfiguration complete." <<std::endl;
	} else {
		std::cout <<"Aborting on Config error: \n" <<errorMsg <<std::endl;
	}
	return errorMsg.empty();
}


