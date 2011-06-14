#include "simpleconf.hpp"

using std::map;
using std::string;

using namespace sim_mob;


namespace {


/**
 * NOTE: LibXML apparently likes to use casting to get from char* to xmlChar* (which is really of
 *       type unsigned char*). This seems wrong; perhaps there is a better way of doing it?
 */
std::string evaluateXPath(xmlXPathContext* xpContext, const std::string& expression)
{
	xmlXPathObject* xpObject = xmlXPathEvalExpression((xmlChar*)expression.c_str(), xpContext);
	if (xpObject==NULL) {
		return "";
	}

	//Ensure there's only one attribute result
	if (xpObject->nodesetval->nodeNr!=1) {
		xmlXPathFreeObject(xpObject);
		return "";
	}

	//Get it.
	xmlNode* curr = *xpObject->nodesetval->nodeTab;

	//Get its content
	// TODO: Something tells me curr->children->content isn't the right way to do things with XPath
	std::string res = (char*)curr->children->content;
	xmlXPathFreeObject(xpObject);
	return res;
}



int getValueInMS(const std::string& valueStr, const std::string& unitsStr)
{
	//Detect errors
	if (valueStr.empty() || unitsStr.empty()) {
		return -1;
	}

	//Convert to integer.
	int value;
	std::string units = unitsStr;
	std::istringstream(valueStr) >> value;

	//Reduce to ms
    if (units == "minutes") {
    	value *= 60;
    	units = "seconds";
    }
    if (units == "seconds") {
    	value *= 1000;
    	units = "ms";
    }

    //Final check
    if (units != "ms") {
    	return -1;
    }

    return value;
}



bool loadXMLAgents(xmlXPathContext* xpContext, std::vector<Agent>& agents)
{
	std::string expression = "/config/pedestrians/pedestrian";
	xmlXPathObject* xpObject = xmlXPathEvalExpression((xmlChar*)expression.c_str(), xpContext);
	if (xpObject==NULL) {
		return false;
	}

	//Move through results
	agents.clear();
	for (xmlNode** it=xpObject->nodesetval->nodeTab; *it!=NULL; it++) {
		xmlNode* curr = *it;
		Agent agent;
		unsigned int flagCheck = 0;
		for (xmlAttr* attrs=curr->properties; attrs!=NULL; attrs=attrs->next) {
			//Read each attribute.
			std::string name = (char*)attrs->name;
			std::string value = (char*)attrs->children->content;
			if (name.empty() || value.empty()) {
				return false;
			}
			int valueI;
			std::istringstream(value) >> valueI;

			//Assign it.
			if (name=="id") {
				agent = Agent(valueI);
				flagCheck |= 1;
			} else if (name=="xPos") {
				agent.xPos.force(valueI);
				flagCheck |= 2;
			} else if (name=="yPos") {
				agent.yPos.force(valueI);
				flagCheck |= 4;
			} else {
				return false;
			}
		}

		if (flagCheck!=7) {
			return false;
		}

		//Save it.
		agents.push_back(agent);
	}




	xmlXPathFreeObject(xpObject);
	return true;
}



bool loadXMLBoundariesCrossings(xmlXPathContext* xpContext, const string& expression, map<string, Point>& result)
{
	xmlXPathObject* xpObject = xmlXPathEvalExpression((xmlChar*)expression.c_str(), xpContext);
	if (xpObject==NULL) {
		return false;
	}

	//Move through results
	result.clear();
	for (xmlNode** it=xpObject->nodesetval->nodeTab; *it!=NULL; it++) {
		xmlNode* curr = *it;
		string key;
		Point val;
		unsigned int flagCheck = 0;
		for (xmlAttr* attrs=curr->properties; attrs!=NULL; attrs=attrs->next) {
			//Read each attribute.
			std::string name = (char*)attrs->name;
			std::string value = (char*)attrs->children->content;
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
					val.xPos = valueI;
					flagCheck |= 2;
				} else if (name=="yPos") {
					val.yPos = valueI;
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
		result[key] = val;
	}




	xmlXPathFreeObject(xpObject);
	return true;
}



std::string loadXMLConf(xmlDoc* document, xmlXPathContext* xpContext, std::vector<Agent>& agents)
{
	//Ensure we loaded a real document
	if (document==NULL) {
		return "Couldn't load XML config file.";
	}

	//Create an X-Path evaluation context
	xpContext = xmlXPathNewContext(document);
	if (xpContext==NULL) {
		return "Couldn't get an X-Path context.";
	}

    //Perform a series of XPath evaluations
	int baseGran = getValueInMS(
			evaluateXPath(xpContext, "/config/system/simulation/base_granularity/@value"),
			evaluateXPath(xpContext, "/config/system/simulation/base_granularity/@units"));
	int totalRuntime = getValueInMS(
			evaluateXPath(xpContext, "/config/system/simulation/total_runtime/@value"),
			evaluateXPath(xpContext, "/config/system/simulation/total_runtime/@units"));
	int totalWarmup = getValueInMS(
			evaluateXPath(xpContext, "/config/system/simulation/total_warmup/@value"),
			evaluateXPath(xpContext, "/config/system/simulation/total_warmup/@units"));
	int granAgent = getValueInMS(
			evaluateXPath(xpContext, "/config/system/granularities/agent/@value"),
			evaluateXPath(xpContext, "/config/system/granularities/agent/@units"));
	int granSignal = getValueInMS(
			evaluateXPath(xpContext, "/config/system/granularities/signal/@value"),
			evaluateXPath(xpContext, "/config/system/granularities/signal/@units"));
	int granPaths = getValueInMS(
			evaluateXPath(xpContext, "/config/system/granularities/paths/@value"),
			evaluateXPath(xpContext, "/config/system/granularities/paths/@units"));
	int granDecomp = getValueInMS(
			evaluateXPath(xpContext, "/config/system/granularities/decomp/@value"),
			evaluateXPath(xpContext, "/config/system/granularities/decomp/@units"));


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

    //Load agents
    if (!loadXMLAgents(xpContext, agents)) {
    	return "Couldn't load agents";
    }

    //Load boundaries
    if (!loadXMLBoundariesCrossings(xpContext, "/config/boundaries/boundary", ConfigParams::GetInstance().boundaries)) {
    	return "Couldn't load boundaries";
    }

    //Load crossings
    if (!loadXMLBoundariesCrossings(xpContext, "/config/crossings/crossing", ConfigParams::GetInstance().crossings)) {
    	return "Couldn't load crossings";
    }

    //Save
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
    std::cout <<"  Boundaries Found: " <<ConfigParams::GetInstance().boundaries.size() <<"\n";
    for (map<string, Point>::iterator it=ConfigParams::GetInstance().boundaries.begin(); it!=ConfigParams::GetInstance().boundaries.end(); it++) {
    	std::cout <<"    Boundary[" <<it->first <<"] = (" <<it->second.xPos <<"," <<it->second.yPos <<")\n";
    }
    std::cout <<"  Crossings Found: " <<ConfigParams::GetInstance().crossings.size() <<"\n";
    for (map<string, Point>::iterator it=ConfigParams::GetInstance().crossings.begin(); it!=ConfigParams::GetInstance().crossings.end(); it++) {
    	std::cout <<"    Crossing[" <<it->first <<"] = (" <<it->second.xPos <<"," <<it->second.yPos <<")\n";
    }
    std::cout <<"  Agents Initialized: " <<agents.size() <<"\n";
    for (size_t i=0; i<agents.size(); i++) {
    	std::cout <<"    Agent(" <<agents[i].getId() <<") = " <<agents[i].xPos.get() <<"," <<agents[i].yPos.get() <<"\n";
    }
    std::cout <<"------------------\n";

	//No error
	return "";
}

}



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

bool sim_mob::ConfigParams::InitUserConf(std::vector<Agent>& agents, std::vector<Region>& regions,
		          std::vector<TripChain>& trips, std::vector<ChoiceSet>& chSets,
		          std::vector<Vehicle>& vehicles)
{
	//Data
	xmlDoc* document = NULL;
	xmlXPathContext* xpContext = NULL;


	//Load an XML document containing our config file.
	document = xmlParseFile("data/config.xml");
	std::string errorMsg = loadXMLConf(document, xpContext, agents);
	if (errorMsg.empty()) {
		std::cout <<"XML config file loaded." <<std::endl;
	} else {
		std::cout <<"Error: " <<errorMsg <<std::endl;
	}

	//Clean up data
	if (document != NULL) {
		xmlFreeDoc(document);
	}
	if (xpContext != NULL) {
		xmlXPathFreeContext(xpContext);
	}


	//TEMP:
	for (size_t i=0; i<5; i++)
		regions.push_back(Region(i));
	for (size_t i=0; i<6; i++)
		trips.push_back(TripChain(i));
	for (size_t i=0; i<15; i++)
		chSets.push_back(ChoiceSet(i));
	for (size_t i=0; i<10; i++)
		vehicles.push_back(Vehicle(i));
	std::cout <<"Configuration complete." <<std::endl;


	return errorMsg.empty();

}

