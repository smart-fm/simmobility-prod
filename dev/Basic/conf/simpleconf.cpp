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
	unsigned int baseGranVal;
	std::string baseGranValStr = evaluateXPath(xpContext, "/config/system/simulation/base_granularity/@value");
	std::string baseGranUnits = evaluateXPath(xpContext, "/config/system/simulation/base_granularity/@units");
	if (!baseGranValStr.empty()) {
		std::istringstream(baseGranValStr) >> baseGranVal;
	}
	unsigned int totalRuntimeVal;
	std::string totalRuntimeValStr = evaluateXPath(xpContext, "/config/system/simulation/total_runtime/@value");
	std::string totalRuntimeUnits = evaluateXPath(xpContext, "/config/system/simulation/total_runtime/@units");
	if (!totalRuntimeValStr.empty()) {
		std::istringstream(totalRuntimeValStr) >> totalRuntimeVal;
	}
	unsigned int totalWarmupVal;
	std::string totalWarmupValStr = evaluateXPath(xpContext, "/config/system/simulation/total_warmup/@value");
	std::string totalWarmupUnits = evaluateXPath(xpContext, "/config/system/simulation/total_warmup/@units");
	if (!totalWarmupValStr.empty()) {
		std::istringstream(totalWarmupValStr) >> totalWarmupVal;
	}

	//Check
    if(		baseGranValStr.empty() || baseGranUnits.empty()
    	 || totalRuntimeValStr.empty() || totalRuntimeUnits.empty()
    	 || totalWarmupValStr.empty() || totalWarmupUnits.empty()) {
        return "Unable to evaluate xpath expression";
    }

    //Convert to seconds
    if (baseGranUnits == "minutes") {
    	baseGranVal *= 60;
    	baseGranUnits = "seconds";
    }
    if (totalRuntimeUnits == "minutes") {
    	totalRuntimeVal *= 60;
    	totalRuntimeUnits = "seconds";
    }
    if (totalWarmupUnits == "minutes") {
    	totalWarmupVal *= 60;
    	totalWarmupUnits = "seconds";
    }

    //Double-check
    if (baseGranUnits!="seconds" || totalRuntimeUnits!="seconds" || totalWarmupUnits!="seconds") {
    	return "Unable unit specifier: " + baseGranUnits;
    }

    //Display
    std::cout <<"Config parameters:\n";
    std::cout <<"------------------\n";
    std::cout <<"  Base Granularity: " <<baseGranVal <<" " <<baseGranUnits <<"\n";
    std::cout <<"  Total Runtime: " <<totalRuntimeVal <<" " <<totalRuntimeUnits <<"\n";
    if (totalRuntimeVal%baseGranVal != 0) {
    	std::cout <<"    Warning! This value will be truncated to "<<totalRuntimeVal-(totalRuntimeVal/baseGranVal) <<" " <<totalRuntimeUnits <<"\n";
    }
    std::cout <<"  Total Warmup: " <<totalWarmupVal <<" " <<totalWarmupUnits <<"\n";
    if (totalWarmupVal%baseGranVal != 0) {
    	std::cout <<"    Warning! This value will be truncated to "<<totalWarmupVal-(totalWarmupVal%baseGranVal) <<" " <<totalRuntimeUnits <<"\n";
    }
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



