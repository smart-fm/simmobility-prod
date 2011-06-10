//Placeholder file while we figure out how config files will work.


#include "simpleconf.hpp"


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
		return "";
	}

	//Get it.
	xmlNode* curr = *xpObject->nodesetval->nodeTab;

	//Get its content
	// TODO: Something tells me curr->children->content isn't the right way to do things with XPath
	std::string res = (char*)curr->children->content;
	if (xpObject != NULL) {
		xmlXPathFreeObject(xpObject);
	}
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



std::string loadXMLConf(xmlDoc* document, xmlXPathContext* xpContext)
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

    //Display
    std::cout <<"Config parameters:\n";
    std::cout <<"------------------\n";
    std::cout <<"  Base Granularity: " <<baseGran <<" " <<"ms" <<"\n";
    std::cout <<"  Total Runtime: " <<totalRuntime <<" " <<"ms" <<"\n";
    if (totalRuntime%baseGran != 0) {
    	std::cout <<"    Warning! This value will be truncated to "<<totalRuntime-(totalRuntime/baseGran) <<" " <<"ms" <<"\n";
    }
    std::cout <<"  Total Warmup: " <<totalWarmup <<" " <<"ms" <<"\n";
    if (totalWarmup%baseGran != 0) {
    	std::cout <<"    Warning! This value will be truncated to "<<totalWarmup-(totalWarmup%baseGran) <<" " <<"ms" <<"\n";
    }
    std::cout <<"  Agent Granularity: " <<granAgent <<" " <<"ms" <<"\n";
    std::cout <<"  Signal Granularity: " <<granSignal <<" " <<"ms" <<"\n";
    std::cout <<"  Paths Granularity: " <<granPaths <<" " <<"ms" <<"\n";
    std::cout <<"  Decomp Granularity: " <<granDecomp <<" " <<"ms" <<"\n";
    std::cout <<"------------------\n";

	//No error
	return "";
}

}



bool loadUserConf(std::vector<Agent>& agents, std::vector<Region>& regions,
		          std::vector<TripChain>& trips, std::vector<ChoiceSet>& chSets,
		          std::vector<Vehicle>& vehicles)
{
	//Data
	xmlDoc* document = NULL;
	xmlXPathContext* xpContext = NULL;


	//Load an XML document containing our config file.
	document = xmlParseFile("data/config.xml");
	std::string errorMsg = loadXMLConf(document, xpContext);
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
	for (size_t i=0; i<20; i++)
		agents.push_back(Agent(i));
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



