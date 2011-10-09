/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "simpleconf.hpp"

#include <tinyxml.h>

#include <algorithm>

//Include here (forward-declared earlier) to avoid include-cycles.
#include "../entities/Agent.hpp"
#include "../entities/Person.hpp"
#include "../entities/Region.hpp"
#include "../entities/roles/pedestrian/Pedestrian.hpp"
#include "../entities/roles/driver/Driver.hpp"
#include "../geospatial/aimsun/Loader.hpp"
#include "../geospatial/Node.hpp"
#include "../geospatial/UniNode.hpp"
#include "../geospatial/MultiNode.hpp"
#include "../geospatial/Intersection.hpp"
#include "../geospatial/Crossing.hpp"
#include "../geospatial/RoadSegment.hpp"
#include "../geospatial/LaneConnector.hpp"
#include "../geospatial/StreetDirectory.hpp"

using std::cout;
using std::endl;
using std::map;
using std::set;
using std::string;
using std::vector;

using namespace sim_mob;


namespace {


//Helper sort
bool agent_sort_by_id (Agent* i, Agent* j) { return (i->getId()<j->getId()); }


//Of the form xxxx,yyyyy, with optional signs
bool readPoint(const string& str, Point2D& res)
{
	//Does it match the pattern?
	size_t commaPos = str.find(',');
	if (commaPos==string::npos) {
		return false;
	}

	//Try to parse its substrings
	int xPos, yPos;
	std::istringstream(str.substr(0, commaPos)) >> xPos;
	std::istringstream(str.substr(commaPos+1, string::npos)) >> yPos;

	res = Point2D(xPos, yPos);
	return true;
}



int getValueInMS(int value, const std::string& units)
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

    return value;
}


int ReadGranularity(TiXmlHandle& handle, const std::string& granName)
{
	TiXmlElement* node = handle.FirstChild(granName).ToElement();
	if (!node) {
		return -1;
	}

	int value;
	const char* units = node->Attribute("units");
	if (!node->Attribute("value", &value) || !units) {
		return -1;
	}

	return getValueInMS(value, units);
}


bool loadXMLAgents(TiXmlDocument& document, std::vector<Agent*>& agents, const std::string& agentType)
{
	//Quick check.
	if (agentType!="pedestrian" && agentType!="driver") {
		return false;
	}

	//Build the expression dynamically.
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild(agentType+"s").FirstChild(agentType).ToElement();
	if (!node) {
		//Agents are optional
		return true;
	}

	//Loop through all agents of this type
	for (;node;node=node->NextSiblingElement()) {
		Person* agent = nullptr;
		bool foundID = false;
		bool foundXPos = false;
		bool foundYPos = false;
		bool foundOrigPos = false;
		bool foundDestPos = false;
		for (TiXmlAttribute* attr=node->FirstAttribute(); attr; attr=attr->Next()) {
			//Read each attribute.
			std::string name = attr->NameTStr();
			std::string value = attr->ValueStr();
			if (name.empty() || value.empty()) {
				return false;
			}
			int valueI=-1;
			if (name=="id" || name=="xPos" || name=="yPos"||name=="time") {
				std::istringstream(value) >> valueI;
			}

			//Assign it.
			if (name=="id") {
				agent = new Person(valueI);
				if (agentType=="pedestrian") {
					agent->changeRole(new Pedestrian(agent));
				} else if (agentType=="driver") {
					agent->changeRole(new Driver(agent));
				}
				foundID = true;
			} else if (name=="xPos") {
				agent->xPos.force(valueI);
				foundXPos = true;
			} else if (name=="yPos") {
				agent->yPos.force(valueI);
				foundYPos = true;
			} else if (name=="originPos") {
				Point2D pt;
				if (!readPoint(value, pt)) {
					std::cout <<"Couldn't read point from value: " <<value <<"\n";
					return false;
				}
				agent->originNode = ConfigParams::GetInstance().getNetwork().locateNode(pt, true);
				if (!agent->originNode) {
					std::cout <<"Couldn't find position: " <<pt.getX() <<"," <<pt.getY() <<"\n";
					return false;
				}
				foundOrigPos = true;
			} else if (name=="destPos") {
				Point2D pt;
				if (!readPoint(value, pt)) {
					std::cout <<"Couldn't read point from value: " <<value <<"\n";
					return false;
				}
				agent->destNode = ConfigParams::GetInstance().getNetwork().locateNode(pt, true);
				if (!agent->destNode) {
					std::cout <<"Couldn't find position: " <<pt.getX() <<"," <<pt.getY() <<"\n";
					return false;
				}
				foundDestPos = true;
			}
			else if (name=="time"){
				agent->startTime=valueI;
			}
			else {
				return false;
			}
		}

		//Simple checks
		bool foundOldPos = foundXPos && foundYPos;
		if (!foundID) {
			std::cout <<"id attribute not found.\n";
			return false;
		}
		if (!foundOldPos && !foundOrigPos && !foundDestPos) {
			std::cout <<"agent position information not found.\n";
			return false;
		}

		//Slightly more complex checks
		if (foundOldPos && (foundOrigPos || foundDestPos)) {
			std::cout <<"agent contains both old and new-style position information.\n";
			return false;
		}


		//Save it.
		agents.push_back(agent);
	}

	return true;
}


bool loadXMLSignals(TiXmlDocument& document, std::vector<Agent*>& agents, const std::string& signalKeyID)
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

	//Loop through all agents of this type
	for (;node;node=node->NextSiblingElement()) {
		Signal* sig = nullptr;
		bool foundID = false;
		for (TiXmlAttribute* attr=node->FirstAttribute(); attr; attr=attr->Next()) {
			std::string name = attr->NameTStr();
			std::string value = attr->ValueStr();
			if (name.empty() || value.empty()) {
				return false;
			}
			int valueI=-1;
			if (name=="id" || name=="xPos" || name=="yPos") {
				std::istringstream(value) >> valueI;
			}

			//Assign it.
			if (name=="id") {
				sig = new Signal(valueI);
				foundID = true;
			} else {
				return false;
			}
		}
		if (!foundID) {
			return false;
		}

		agents.push_back(sig);
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
		if (storedProcedures.count(name)!=0) {
			return false;
		}

		storedProcedures[name] = string(value);
	}

	//Done; we'll check the storedProcedures in detail later.
	return !connectionString.empty() && storedProcedures.size()==6;
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
	//Save RoadSegments/Connectors to make output simpler
	std::set<const RoadSegment*> cachedSegments;
	std::set<LaneConnector*> cachedConnectors;

	//Initial message
	RoadNetwork& rn = ConfigParams::GetInstance().getNetwork();
	std::ostream& logout = BufferedBase::log_file();
	logout <<"Printing node network" <<endl;
	logout <<"NOTE: All IDs in this section are consistent for THIS simulation run, but will change if you run the simulation again." <<endl;

	//Print nodes first
	for (set<UniNode*>::const_iterator it=rn.getUniNodes().begin(); it!=rn.getUniNodes().end(); it++) {
		logout <<"(\"uni-node\", 0, " <<*it <<", {";
		logout <<"\"xPos\":\"" <<(*it)->location->getX() <<"\",";
		logout <<"\"yPos\":\"" <<(*it)->location->getY() <<"\",";
		logout <<"})" <<endl;

		//Cache all segments
		vector<const RoadSegment*> segs = (*it)->getRoadSegments();
		for (vector<const RoadSegment*>::const_iterator i2=segs.begin(); i2!=segs.end(); ++i2) {
			cachedSegments.insert(*i2);
		}
	}
	for (vector<MultiNode*>::const_iterator it=rn.getNodes().begin(); it!=rn.getNodes().end(); it++) {
		logout <<"(\"multi-node\", 0, " <<*it <<", {";
		logout <<"\"xPos\":\"" <<(*it)->location->getX() <<"\",";
		logout <<"\"yPos\":\"" <<(*it)->location->getY() <<"\",";
		logout <<"})" <<endl;

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
			if (nodeInt->hasOutgoingLanes(**rsIt)) {
				for (set<LaneConnector*>::iterator i2=nodeInt->getOutgoingLanes(**rsIt).begin(); i2!=nodeInt->getOutgoingLanes(**rsIt).end(); i2++) {
					//Cache the connector
					cachedConnectors.insert(*i2);
				}
			}
		}
	}

	//Links can go next.
	for (vector<Link*>::const_iterator it=rn.getLinks().begin(); it!=rn.getLinks().end(); it++) {
		logout <<"(\"link\", 0, " <<*it <<", {";
		logout <<"\"road-name\":\"" <<(*it)->roadName <<"\",";
		logout <<"\"start-node\":\"" <<(*it)->getStart() <<"\",";
		logout <<"\"end-node\":\"" <<(*it)->getEnd() <<"\",";
		logout <<"\"fwd-path\":\"[";
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath(true).begin(); segIt!=(*it)->getPath(true).end(); segIt++) {
			logout <<*segIt <<",";
		}
		logout <<"]\",";
		logout <<"\"rev-path\":\"[";
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath(false).begin(); segIt!=(*it)->getPath(false).end(); segIt++) {
			logout <<*segIt <<",";
		}
		logout <<"]\",";
		logout <<"})" <<endl;
	}

	//Now print all Segments
	std::set<const Crossing*> cachedCrossings;
	for (std::set<const RoadSegment*>::const_iterator it=cachedSegments.begin(); it!=cachedSegments.end(); it++) {
		logout <<"(\"road-segment\", 0, " <<*it <<", {";
		logout <<"\"parent-link\":\"" <<(*it)->getLink() <<"\",";
		logout <<"\"max-speed\":\"" <<(*it)->maxSpeed <<"\",";
		logout <<"\"lanes\":\"" <<(*it)->getLanes().size() <<"\",";
		logout <<"\"from-node\":\"" <<(*it)->getStart() <<"\",";
		logout <<"\"to-node\":\"" <<(*it)->getEnd() <<"\",";
		logout <<"})" <<endl;

		if (!(*it)->polyline.empty()) {
			logout <<"(\"polyline\", 0, " <<&((*it)->polyline) <<", {";
			logout <<"\"parent-segment\":\"" <<*it <<"\",";
			logout <<"\"points\":\"[";
			for (vector<Point2D>::const_iterator ptIt=(*it)->polyline.begin(); ptIt!=(*it)->polyline.end(); ptIt++) {
				logout <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
			}
			logout <<"]\",";
			logout <<"})" <<endl;
		}

		//Save crossing info for later
		RoadItemAndOffsetPair res = (*it)->nextObstacle(0, true);
		if (res.item) {
			const Crossing* resC  = dynamic_cast<const Crossing*>(res.item);
			if (resC) {
				cachedCrossings.insert(resC);
			} else {
				std::cout <<"NOTE: Unknown obstacle!\n";
			}
		}

		//Save Lane info for later
		//NOTE: For now this relies on somewhat sketchy behavior, which is why we output a "tmp-*"
		//      flag. Once we add auto-polyline generation, that tmp- output will be meaningless
		//      and we can switch to a full "lane" output type.
		std::stringstream laneBuffer; //Put it in its own buffer since getLanePolyline() can throw.
		laneBuffer <<"(\"lane\", 0, " <<&((*it)->getLanes()) <<", {";
		laneBuffer <<"\"parent-segment\":\"" <<*it <<"\",";
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
		laneBuffer <<"})" <<endl;
		logout <<laneBuffer.str();
	}

	//Crossings are part of Segments
	for (std::set<const Crossing*>::iterator it=cachedCrossings.begin(); it!=cachedCrossings.end(); it++) {
		logout <<"(\"crossing\", 0, " <<*it <<", {";
		logout <<"\"near-1\":\"" <<(*it)->nearLine.first.getX() <<"," <<(*it)->nearLine.first.getY() <<"\",";
		logout <<"\"near-2\":\"" <<(*it)->nearLine.second.getX() <<"," <<(*it)->nearLine.second.getY() <<"\",";
		logout <<"\"far-1\":\"" <<(*it)->farLine.first.getX() <<"," <<(*it)->farLine.first.getY() <<"\",";
		logout <<"\"far-2\":\"" <<(*it)->farLine.second.getX() <<"," <<(*it)->farLine.second.getY() <<"\",";
		logout <<"})" <<endl;
	}


	//Now print all Connectors
	for (std::set<LaneConnector*>::const_iterator it=cachedConnectors.begin(); it!=cachedConnectors.end(); it++) {
		//Retrieve relevant information
		RoadSegment* fromSeg = (*it)->getLaneFrom()->getRoadSegment();
		unsigned int fromLane = (*it)->getLaneFrom()->getLaneID();
		RoadSegment* toSeg = (*it)->getLaneTo()->getRoadSegment();
		unsigned int toLane = (*it)->getLaneTo()->getLaneID();

		//Output
		logout <<"(\"lane-connector\", 0, " <<*it <<", {";
		logout <<"\"from-segment\":\"" <<fromSeg <<"\",";
		logout <<"\"from-lane\":\"" <<fromLane <<"\",";
		logout <<"\"to-segment\":\"" <<toSeg <<"\",";
		logout <<"\"to-lane\":\"" <<toLane <<"\",";
		logout <<"})" <<endl;
	}

	//Temp: Print ordering of output Links
	for (vector<MultiNode*>::const_iterator it=rn.getNodes().begin(); it!=rn.getNodes().end(); it++) {
		size_t count = 1;
		std::vector< std::pair<RoadSegment*, bool> >& vec = (*it)->roadSegmentsCircular;
		for (std::vector< std::pair<RoadSegment*, bool> >::iterator it2=vec.begin(); it2!=vec.end(); it2++) {
			logout <<"(\"tmp-circular\", 0, " <<0 <<", {";
			logout <<"\"at-node\":\"" <<*it <<"\",";
			logout <<"\"at-segment\":\"" <<it2->first <<"\",";
			logout <<"\"fwd\":\"" <<it2->second <<"\",";
			logout <<"\"number\":\"" <<count++ <<"\",";
			logout <<"})" <<endl;
		}
	}
}



//Returns the error message, or an empty string if no error.
std::string loadXMLConf(TiXmlDocument& document, std::vector<Agent*>& agents)
{
	//Save granularities: system
	TiXmlHandle handle(&document);
	handle = handle.FirstChild("config").FirstChild("system").FirstChild("simulation");
	int baseGran = ReadGranularity(handle, "base_granularity");
	int totalRuntime = ReadGranularity(handle, "total_runtime");
	int totalWarmup = ReadGranularity(handle, "total_warmup");

	//Save more granularities
	handle = TiXmlHandle(&document);
	handle = handle.FirstChild("config").FirstChild("system").FirstChild("granularities");
	int granAgent = ReadGranularity(handle, "agent");
	int granSignal = ReadGranularity(handle, "signal");
	int granPaths = ReadGranularity(handle, "paths");
	int granDecomp = ReadGranularity(handle, "decomp");

	//Check
    if(    baseGran==-1 || totalRuntime==-1 || totalWarmup==-1
    	|| granAgent==-1 || granSignal==-1 || granPaths==-1 || granDecomp==-1) {
        return "Unable to read config file.";
    }

    //Granularity check
    if (granAgent%baseGran != 0) {
    	return "Agent granularity not a multiple of base granularity.";
    }
    if (granSignal%baseGran != 0) {
    	return "Signal granularity not a multiple of base granularity.";
    }
    if (granPaths%baseGran != 0) {
    	return "Path granularity not a multiple of base granularity.";
    }
    if (granDecomp%baseGran != 0) {
    	return "Decomposition granularity not a multiple of base granularity.";
    }
    if (totalRuntime%baseGran != 0) {
    	std::cout <<"  Warning! Total Runtime will be truncated.\n";
    }
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
    		const char* geomSrc = geomElem->Attribute("source");
    		if (!geomSrc || "database" != string(geomSrc)) {
    			return "Unknown geometry source: " + (geomSrc?string(geomSrc):"");
    		}

    		//Load the AIMSUM network details
    		map<string, string> storedProcedures; //Of the form "node" -> "get_node()"
    		if (!LoadDatabaseDetails(*geomElem, ConfigParams::GetInstance().connectionString, storedProcedures)) {
    			return "Unable to load database connection settings.";
    		}

    		//Confirm that all stored procedures have been set.
    		if (
    			   storedProcedures.count("node")==0 || storedProcedures.count("section")==0
    			|| storedProcedures.count("turning")==0 || storedProcedures.count("polyline")==0
    			|| storedProcedures.count("crossing")==0 || storedProcedures.count("lane")==0
    		) {
    			return "Not all stored procedures were specified.";
    		}

    		//Actually load it
    		string dbErrorMsg = sim_mob::aimsun::Loader::LoadNetwork(ConfigParams::GetInstance().connectionString, storedProcedures, ConfigParams::GetInstance().getNetwork());
    		if (!dbErrorMsg.empty()) {
    			return "Database loading error: " + dbErrorMsg;
    		}
                StreetDirectory::instance().init(ConfigParams::GetInstance().getNetwork(), true);

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


    //Load ALL agents: drivers and pedestrians.
    //  (Note: Use separate config files if you just want to test one kind of agent.)
    if (!loadXMLAgents(document, agents, "pedestrian")) {
    	return "Couldn't load pedestrians";
    }
    if (!loadXMLAgents(document, agents, "driver")) {
    	return	 "Couldn't load drivers";
    }

    //Load signals, which are currently agents
    if (!loadXMLSignals(document, agents, "signal")) {
    	return	 "Couldn't load signals";
    }

    //Sort agents by id.
    //TEMP: Eventually, we'll need a more sane way to deal with agent IDs.
    std::sort(agents.begin(), agents.end(), agent_sort_by_id);


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
    if (!ConfigParams::GetInstance().connectionString.empty()) {
    	//Output AIMSUN data
    	std::cout <<"Network details loaded from connection: " <<ConfigParams::GetInstance().connectionString <<"\n";
    	std::cout <<"------------------\n";
    	PrintDB_Network();
    	std::cout <<"------------------\n";
    }
    std::cout <<"  Agents Initialized: " <<agents.size() <<"\n";
    for (size_t i=0; i<agents.size(); i++) {
    	std::cout <<"    Agent(" <<agents[i]->getId() <<") = " <<agents[i]->xPos.get() <<"," <<agents[i]->yPos.get() <<"\n";
    }
    std::cout <<"------------------\n";

	//No error
	return "";
}



} //End anon namespace



//////////////////////////////////////////
// Simple singleton implementation
//////////////////////////////////////////
ConfigParams sim_mob::ConfigParams::instance;
sim_mob::ConfigParams::ConfigParams() {

}
ConfigParams& sim_mob::ConfigParams::GetInstance() {
	return ConfigParams::instance;
}





//////////////////////////////////////////
// Main external method
//////////////////////////////////////////

bool sim_mob::ConfigParams::InitUserConf(const string& configPath, std::vector<Agent*>& agents, std::vector<Region*>& regions,
		          std::vector<TripChain*>& trips, std::vector<ChoiceSet*>& chSets,
		          std::vector<Vehicle*>& vehicles)
{
	//Load our config file into an XML document object.
	TiXmlDocument doc(configPath);
	if (!doc.LoadFile()) {
		std::cout <<"Error loading config file: " <<doc.ErrorDesc() <<std::endl;
		return false;
	}

	//Parse it
	string errorMsg = loadXMLConf(doc, agents);
	if (errorMsg.empty()) {
		std::cout <<"XML config file loaded." <<std::endl;
	} else {
		std::cout <<"Aborting on Config Error: " <<errorMsg <<std::endl;
	}

	//TEMP:
	for (size_t i=0; i<5; i++)
		regions.push_back(new Region(i));
	for (size_t i=0; i<6; i++)
		trips.push_back(new TripChain(i));
	for (size_t i=0; i<15; i++)
		chSets.push_back(new ChoiceSet(i));
	for (size_t i=0; i<10; i++)
		vehicles.push_back(new Vehicle(i));
	if (errorMsg.empty()) {
		std::cout <<"Configuration complete." <<std::endl;
	}


	return errorMsg.empty();

}
