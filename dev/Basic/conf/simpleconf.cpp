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
	if (xpObject==NULL || xpObject->stringval==NULL) {
		return "";
	}

	std::string res = (char*)xpObject->stringval;
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
	std::string basGranVal = evaluateXPath(xpContext, "/config/system/simulation/base_granularity/@value");
	std::string basGranUnits = evaluateXPath(xpContext, "/config/system/simulation/base_granularity/@units");

	//Check
    if(basGranVal.empty() || basGranUnits.empty()) {
        return "Unable to evaluate xpath expression";
    }

    std::cout <<"Base granularity: " <<basGranVal <<" " <<basGranUnits <<"\n";

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



