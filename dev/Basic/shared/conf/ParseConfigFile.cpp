//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParseConfigFile.hpp"

#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "conf/RawConfigParams.hpp"
#include "geospatial/Point2D.hpp"
#include "util/GeomHelpers.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;
using namespace xercesc;


namespace {
//Helper: turn a Xerces error message into a string.
std::string TranscodeString(const XMLCh* str) {
	char* raw = XMLString::transcode(str);
	std::string res(raw);
	XMLString::release(&raw);
	return res;
}

//Helper: make sure we actually have an element
DOMElement* NodeToElement(DOMNode* node) {
	DOMElement* res = dynamic_cast<DOMElement*>(node);
	if (!res) {
		throw std::runtime_error("DOMNode is expected to be a DOMElement.");
	}
	return res;
}

//Helper: retrieve child elements without leaking memory
std::vector<DOMElement*> GetElementsByName(DOMElement* node, const std::string& key, bool required=false) {
	XMLCh* keyX = XMLString::transcode(key.c_str());
	DOMNodeList* res = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);

	if (res->getLength()==0 && required) {
		throw std::runtime_error("Elements expected, but none returned.");
	}

	std::vector<DOMElement*> resV;
	for (XMLSize_t i=0; i<res->getLength(); i++) {
		resV.push_back(NodeToElement(res->item(i)));
	}

	return resV;
}

//Helper: retrieve a single element; optionally required.
DOMElement* GetSingleElementByName(DOMElement* node, const std::string& key, bool required=false) {
	XMLCh* keyX = XMLString::transcode(key.c_str());
	DOMNodeList* res = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);

	//Check.
	if (res->getLength()>1) {
		throw std::runtime_error("Error: single element expected, but returned more than 1.");
	} else if (res->getLength()==1) {
		return NodeToElement(res->item(0));
	} else if (required) {
		throw std::runtime_error("Error: single element expected, but returned zero.");
	}

	return nullptr;
}

//Helper: retrieve an attribute
DOMAttr* GetNamedAttribute(DOMElement* node, const std::string& key, bool required=false) {
	if (!node) {
		return nullptr;
	}

	XMLCh* keyX = XMLString::transcode(key.c_str());
	DOMNode* res= node->getAttributes()->getNamedItem(keyX);
	XMLString::release(&keyX);

	//Check.
	if (!res) {
		if (required) {
			throw std::runtime_error("Error: attribute expected, but none found.");
		} else {
			return nullptr;
		}
	}

	DOMAttr* resAttr = dynamic_cast<DOMAttr*>(res);
	if (!resAttr) {
		throw std::runtime_error("Error: attribute expected, but couldn't be cast.");
	}

	return resAttr;
}

//Helper
const XMLCh* GetAttributeValue(const DOMAttr* attr) {
	if (!attr) {
		return nullptr;
	}
	return attr->getNodeValue();
}


//Helper: Combined
const XMLCh* GetNamedAttributeValue(DOMElement* node, const std::string& key, bool required=false) {
	return GetAttributeValue(GetNamedAttribute(node, key, required));
}


//Helper: boolean stuff
bool ParseBoolean(const XMLCh* srcX, bool* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		std::transform(src.begin(), src.end(), src.begin(), ::tolower);
		if (src=="true" || src=="yes") {
			return true;
		} else if (src=="false" || src=="no") {
			return false;
		}
		throw std::runtime_error("Expected boolean value.");
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory boolean variable; no default available.");
	}
	return *defValue;
}

//Helper: amount+value for time-granularities.
unsigned int GetValueInMs(double amount, std::string units, unsigned int* defValue) {
	//Handle plural
	if (units=="second") { units="seconds"; }
	if (units=="minute") { units="minutes"; }

	//Detect errors
	if (units.empty() || (units!="minutes" && units!="seconds" && units!="ms")) {
		if (defValue) {
			return *defValue;
		} else {
			throw std::runtime_error("Invalid units in parsing time granularity.");
		}
	}

	//Reduce to ms
    if (units == "minutes") {
    	amount *= 60*1000;
    }
    if (units == "seconds") {
    	amount *= 1000;
    }

    //Check for overflow:
    unsigned int res = static_cast<unsigned int>(amount);
    if (static_cast<double>(res) != amount) {
    	Warn() <<"NOTE: Rounding value in ms from " <<amount <<" to " <<res <<"\n";
    }

    return res;
}

//Helper: Time units such as "10", "seconds"
unsigned int ParseTimegranAsMs(const XMLCh* amountX, const XMLCh* unitsX, unsigned int* defValue) {
	double amount = boost::lexical_cast<double>(TranscodeString(amountX));
	std::string units = TranscodeString(unitsX);

	return GetValueInMs(amount, units, defValue);
}


SystemParams::NetworkSource ParseNetSourceEnum(const XMLCh* srcX, SystemParams::NetworkSource* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		if (src=="xml") {
			return SystemParams::NETSRC_XML;
		} else if (src=="database") {
			return SystemParams::NETSRC_DATABASE;
		}
		throw std::runtime_error("Expected SystemParams::NetworkSource value.");
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory SystemParams::NetworkSource variable; no default available.");
	}
	return *defValue;
}


AuraManager::AuraManagerImplementation ParseAuraMgrImplEnum(const XMLCh* srcX, AuraManager::AuraManagerImplementation* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		if (src=="simtree") {
			return AuraManager::IMPL_SIMTREE;
		} else if (src=="rdu") {
			return AuraManager::IMPL_RDU;
		} else if (src=="rstar") {
			return AuraManager::IMPL_RSTAR;
		}
		throw std::runtime_error("Expected AuraManager::AuraManagerImplementation value.");
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory AuraManager::AuraManagerImplementation variable; no default available.");
	}
	return *defValue;
}


WorkGroup::ASSIGNMENT_STRATEGY ParseWrkGrpAssignEnum(const XMLCh* srcX, WorkGroup::ASSIGNMENT_STRATEGY* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		if (src=="roundrobin") {
			return WorkGroup::ASSIGN_ROUNDROBIN;
		} else if (src=="smallest") {
			return WorkGroup::ASSIGN_SMALLEST;
		}
		throw std::runtime_error("Expected WorkGroup::ASSIGNMENT_STRATEGY value.");
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory WorkGroup::ASSIGNMENT_STRATEGY variable; no default available.");
	}
	return *defValue;
}


MutexStrategy ParseMutexStrategyEnum(const XMLCh* srcX, MutexStrategy* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		if (src=="buffered") {
			return MtxStrat_Buffered;
		} else if (src=="locked") {
			return MtxStrat_Locked;
		}
		throw std::runtime_error("Expected sim_mob::MutexStrategy value.");
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory sim_mob::MutexStrategy variable; no default available.");
	}
	return *defValue;
}


Point2D ParsePoint2D(const XMLCh* srcX, Point2D* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		return parse_point(src);
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory Point2D variable; no default available.");
	}
	return *defValue;
}


int ParseInteger(const XMLCh* srcX, int* defValue) {
	if (srcX) {
		int value = 0;
		try {
			std::string src = TranscodeString(srcX);
			value = boost::lexical_cast<int>(src);
		} catch( boost::bad_lexical_cast const& ) {
			throw std::runtime_error("Bad formatted source string for Integer parsing.");
		}
		return value;
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory integer variable; no default available.");
	}
	return *defValue;
}

float ParseFloat(const XMLCh* srcX, float* defValue) {
	if (srcX) {
			std::string src = TranscodeString(srcX);
			return boost::lexical_cast<float>(src);
		}

		//Wasn't found.
		if (!defValue) {
			throw std::runtime_error("Mandatory float variable; no default available.");
		}
		return *defValue;
}

unsigned int ParseUnsignedInt(const XMLCh* srcX, unsigned int* defValue) {
	if (srcX) {
		unsigned int value = 0;
		try {
			std::string src = TranscodeString(srcX);
			value = boost::lexical_cast<unsigned int>(src);
		} catch( boost::bad_lexical_cast const& ) {
			throw std::runtime_error("Bad formatted source string for unsigned integer parsing.");
		}
		return value;
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory unsigned integer variable; no default available.");
	}
	return *defValue;
}

unsigned int ParseGranularitySingle(const XMLCh* srcX, unsigned int* defValue) {
	if (srcX) {
		//Search for "[0-9]+ ?[^0-9]+), roughly.
		std::string src = TranscodeString(srcX);
		size_t digStart = src.find_first_of("1234567890");
		size_t digEnd = src.find_first_not_of("1234567890", digStart+1);
		size_t unitStart = src.find_first_not_of(" ", digEnd);
		if (digStart!=0 || digStart==std::string::npos || digEnd==std::string::npos || unitStart==std::string::npos) {
			throw std::runtime_error("Badly formatted single-granularity string.");
		}

		//Now split/parse it.
		double value = boost::lexical_cast<double>(src.substr(digStart, (digEnd-digStart)));
		std::string units = src.substr(unitStart, std::string::npos);

		return GetValueInMs(value, units, defValue);
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory integer (granularity) variable; no default available.");
	}
	return *defValue;
}

DailyTime ParseDailyTime(const XMLCh* srcX, DailyTime* defValue) {
	if (srcX) {
		return DailyTime(TranscodeString(srcX));
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory integer variable; no default available.");
	}
	return *defValue;
}



std::string ParseString(const XMLCh* srcX, std::string* defValue) {
	if (srcX) {
		return TranscodeString(srcX);
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory string variable; no default available.");
	}

	return *defValue;
}

std::string ParseNonemptyString(const XMLCh* srcX, std::string* defValue) {
	std::string res = ParseString(srcX, defValue);
	if (!res.empty()) {
		return res;
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory string variable; no default available. (Empty strings NOT allowed.)");
	}
	return *defValue;
}


//How to do defaults
bool ParseBoolean(const XMLCh* src, bool defValue) {
	return ParseBoolean(src, &defValue);
}
bool ParseBoolean(const XMLCh* src) { //No default
	return ParseBoolean(src, nullptr);
}
int ParseInteger(const XMLCh* src, int defValue) {
	return ParseInteger(src, &defValue);
}
int ParseInteger(const XMLCh* src) { //No default
	return ParseInteger(src, nullptr);
}
Point2D ParsePoint2D(const XMLCh* src, Point2D defValue) {
	return ParsePoint2D(src, &defValue);
}
Point2D ParsePoint2D(const XMLCh* src) { //No default
	return ParsePoint2D(src, nullptr);
}
unsigned int ParseUnsignedInt(const XMLCh* src, unsigned int defValue) {
	return ParseUnsignedInt(src, &defValue);
}
unsigned int ParseUnsignedInt(const XMLCh* src) { //No default
	return ParseUnsignedInt(src, nullptr);
}
float ParseFloat(const XMLCh* src) {
	return ParseFloat(src, nullptr);
}
float ParseFloat(const XMLCh* src, float defValue) {
	return ParseFloat(src, &defValue);
}
unsigned int ParseGranularitySingle(const XMLCh* src, unsigned int defValue) {
	return ParseGranularitySingle(src, &defValue);
}
unsigned int ParseGranularitySingle(const XMLCh* src) { //No default
	return ParseGranularitySingle(src, nullptr);
}
DailyTime ParseDailyTime(const XMLCh* src, DailyTime defValue) {
	return ParseDailyTime(src, &defValue);
}
DailyTime ParseDailyTime(const XMLCh* src) { //No default
	return ParseDailyTime(src, nullptr);
}
SystemParams::NetworkSource ParseNetSourceEnum(const XMLCh* srcX, SystemParams::NetworkSource defValue) {
	return ParseNetSourceEnum(srcX, &defValue);
}
SystemParams::NetworkSource ParseNetSourceEnum(const XMLCh* srcX) {
	return ParseNetSourceEnum(srcX, nullptr);
}
AuraManager::AuraManagerImplementation ParseAuraMgrImplEnum(const XMLCh* srcX, AuraManager::AuraManagerImplementation defValue) {
	return ParseAuraMgrImplEnum(srcX, &defValue);
}
AuraManager::AuraManagerImplementation ParseAuraMgrImplEnum(const XMLCh* srcX) {
	return ParseAuraMgrImplEnum(srcX, nullptr);
}
WorkGroup::ASSIGNMENT_STRATEGY ParseWrkGrpAssignEnum(const XMLCh* srcX, WorkGroup::ASSIGNMENT_STRATEGY defValue) {
	return ParseWrkGrpAssignEnum(srcX, &defValue);
}
WorkGroup::ASSIGNMENT_STRATEGY ParseWrkGrpAssignEnum(const XMLCh* srcX) {
	return ParseWrkGrpAssignEnum(srcX, nullptr);
}
MutexStrategy ParseMutexStrategyEnum(const XMLCh* srcX, MutexStrategy defValue) {
	return ParseMutexStrategyEnum(srcX, &defValue);
}
MutexStrategy ParseMutexStrategyEnum(const XMLCh* srcX) {
	return ParseMutexStrategyEnum(srcX, nullptr);
}
std::string ParseString(const XMLCh* src, std::string defValue) {
	return ParseString(src, &defValue);
}
std::string ParseString(const XMLCh* src) { //No default
	return ParseString(src, nullptr);
}
std::string ParseNonemptyString(const XMLCh* src, std::string defValue) {
	return ParseNonemptyString(src, &defValue);
}
std::string ParseNonemptyString(const XMLCh* src) { //No default
	return ParseNonemptyString(src, nullptr);
}
unsigned int ParseTimegranAsMs(const XMLCh* amount, const XMLCh* units, unsigned int defValue) {
	return ParseTimegranAsMs(amount, units, &defValue);
}
unsigned int ParseTimegranAsMs(const XMLCh* amount, const XMLCh* units) { //No default
	return ParseTimegranAsMs(amount, units, nullptr);
}


//TODO: Now we are starting to overlap...
int ProcessValueInteger2(xercesc::DOMElement* node, int defVal)
{
	return ParseInteger(GetNamedAttributeValue(node, "value"), defVal);
}

//TODO: Same issue; needs to be easier access to these things.
std::string ProcessValueString(xercesc::DOMElement* node)
{
	return ParseString(GetNamedAttributeValue(node, "value"));
}

const double MILLISECONDS_IN_SECOND = 1000.0;
} //End un-named namespace


sim_mob::ParseConfigFile::ParseConfigFile(const std::string& configFileName, RawConfigParams& result) : cfg(result), inFilePath(configFileName)
{
	ParseXmlAndProcess();
}

void sim_mob::ParseConfigFile::ParseXmlAndProcess()
{
	//NOTE: I think the order of destruction matters (parser must be listed last). ~Seth
	InitXerces();
	HandlerBase handBase;
	XercesDOMParser parser;

	//Attempt to parse it.
	std::string errorMsg = ParseXmlFile(parser, dynamic_cast<ErrorHandler&>(handBase));

	//If there's an error, throw it as an exception.
	if (!errorMsg.empty()) {
		throw std::runtime_error(errorMsg.c_str());
	}

	//Now process it.
	ProcessXmlFile(parser);
}

void sim_mob::ParseConfigFile::InitXerces()
{
	//Xerces initialization.
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException& error) {
		throw std::runtime_error(TranscodeString(error.getMessage()).c_str());
	}
}

std::string sim_mob::ParseConfigFile::ParseXmlFile(XercesDOMParser& parser, ErrorHandler& errorHandler)
{
	//Build a parser, set relevant properties on it.
	parser.setValidationScheme(XercesDOMParser::Val_Always);
	parser.setDoNamespaces(true); //This is optional.

	//Set an error handler.
	parser.setErrorHandler(&errorHandler);

	//Attempt to parse the XML file.
	try {
		parser.parse(inFilePath.c_str());
	} catch (const XMLException& error) {
		return TranscodeString(error.getMessage());
	} catch (const DOMException& error) {
		return TranscodeString(error.getMessage());
	} catch (...) {
		return "Unexpected Exception parsing config file.\n" ;
	}

	//No error.
	return "";
}


void sim_mob::ParseConfigFile::ProcessXmlFile(XercesDOMParser& parser)
{
	//Verify that the root node is "config"
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	if (TranscodeString(rootNode->getTagName()) != "config") {
		throw std::runtime_error("xml parse error: root node must be \"config\"");
	}

	//Make sure we don't have a geometry node.
	DOMElement* geom = GetSingleElementByName(rootNode, "geometry");
	if (geom) {
		throw std::runtime_error("Config file contains a <geometry> node, which is no longer allowed. See the <constructs> node for documentation.");
	}

	//Now just parse the document recursively.
	ProcessSystemNode(GetSingleElementByName(rootNode,"system", true));
	//ProcessGeometryNode(GetSingleElementByName(rootNode, "geometry", true));
	ProcessFMOD_Node(GetSingleElementByName(rootNode, "fmodcontroller"));
	ProcessIncidentsNode(GetSingleElementByName(rootNode, "incidentsData"));
	ProcessConstructsNode(GetSingleElementByName(rootNode,"constructs"));
	ProcessBusStopScheduledTimesNode(GetSingleElementByName(rootNode, "scheduledTimes"));
	ProcessPersonCharacteristicsNode(GetSingleElementByName(rootNode, "personCharacteristics"));
	ProcessLongTermParamsNode( GetSingleElementByName(rootNode, "longTermParams"));

	//Agents all follow a template.

	ProcessPedestriansNode(GetSingleElementByName(rootNode, "pedestrians"));
	ProcessDriversNode(GetSingleElementByName(rootNode, "drivers"));
	ProcessBusDriversNode(GetSingleElementByName(rootNode, "busdrivers"));
	ProcessPassengersNode(GetSingleElementByName(rootNode, "passengers"));
	ProcessSignalsNode(GetSingleElementByName(rootNode, "signals"));
	ProcessBusControllersNode(GetSingleElementByName(rootNode, "buscontrollers"));
}


void sim_mob::ParseConfigFile::ProcessSystemNode(DOMElement* node)
{
	ProcessSystemSimulationNode(GetSingleElementByName(node, "simulation", true));
	ProcessSystemWorkersNode(GetSingleElementByName(node, "workers", true));
	ProcessSystemSingleThreadedNode(GetSingleElementByName(node, "single_threaded"));
	ProcessSystemMergeLogFilesNode(GetSingleElementByName(node, "merge_log_files"));
	ProcessSystemNetworkSourceNode(GetSingleElementByName(node, "network_source"));
	ProcessSystemNetworkXmlInputFileNode(GetSingleElementByName(node, "network_xml_file_input"));
	ProcessSystemNetworkXmlOutputFileNode(GetSingleElementByName(node, "network_xml_file_output"));
	ProcessSystemDatabaseNode(GetSingleElementByName(node, "network_database"));
	ProcessSystemXmlSchemaFilesNode(GetSingleElementByName(node, "xsd_schema_files", true));
	ProcessSystemGenericPropsNode(GetSingleElementByName(node, "generic_props"));

	//Warn against using the old network file name for xml_file.
	if (GetSingleElementByName(node, "network_xml_file", false)) {
		throw std::runtime_error("Using old parameter: \"network_xml_file\"; please change it to \"network_xml_file_input\".");
	}

	//Warn against the old workgroup sizes field.
	if (GetSingleElementByName(node, "workgroup_sizes", false)) {
		throw std::runtime_error("Using old parameter: \"workgroup_sizes\"; please change review the new \"workers\" field.");
	}
}


/*void sim_mob::ParseConfigFile::ProcessGeometryNode(xercesc::DOMElement* node)
{
	//The geometry tag has some attributes.
	std::string geomType = ParseString(GetNamedAttributeValue(node, "type"));
	std::string geomSource = ParseString(GetNamedAttributeValue(node, "source"));

	//Quick validation
	if (geomType=="simple") {
		throw std::runtime_error("\"simple\" geometry type no longer supported.");
	}
	if (geomType!="aimsun") {
		throw std::runtime_error("Unknown geometry type.");
	}
	if (geomSource!="database") {
		throw std::runtime_error("Unknown geometry source.");
	}

	//This is kind of backwards...
	cfg.geometry.procedures.dbFormat = "aimsun";

	//Now parse the rest.
	ProcessGeomDbConnection(GetSingleElementByName(node, "connection"));
	ProcessGeomDbMappings(GetSingleElementByName(node, "mappings"));
}*/


void sim_mob::ParseConfigFile::ProcessBusStopScheduledTimesNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}

	//Loop through all children
	int count=0;
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		if (TranscodeString(item->getNodeName())!="stop") {
			Warn() <<"Invalid busStopScheduledTimes child node.\n";
			continue;
		}

		//Retrieve properties, add a new item to the vector.
		BusStopScheduledTime res;
		res.offsetAT = ParseUnsignedInt(GetNamedAttributeValue(item, "offsetAT"), static_cast<unsigned int>(0));
		res.offsetDT = ParseUnsignedInt(GetNamedAttributeValue(item, "offsetDT"), static_cast<unsigned int>(0));
		cfg.busScheduledTimes[count++] = res;
	}
}

void sim_mob::ParseConfigFile::ProcessPersonCharacteristicsNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}

	//Loop through all children
	int count=0;
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		if (TranscodeString(item->getNodeName())!="person") {
			Warn() <<"Invalid personCharacteristics child node.\n";
			continue;
		}

		//Retrieve properties, add a new item to the vector.
		PersonCharacteristics res;
		res.lowerAge = ParseUnsignedInt(GetNamedAttributeValue(item, "lowerAge"), static_cast<unsigned int>(0));
		res.upperAge = ParseUnsignedInt(GetNamedAttributeValue(item, "upperAge"), static_cast<unsigned int>(0));
		res.lowerSecs = ParseInteger(GetNamedAttributeValue(item, "lowerSecs"), static_cast<int>(0));
		res.upperSecs = ParseInteger(GetNamedAttributeValue(item, "upperSecs"), static_cast<int>(0));
		cfg.personCharacteristicsParams.personCharacteristics[count++] = res;
	}

	std::map<int, PersonCharacteristics> personCharacteristics =  cfg.personCharacteristicsParams.personCharacteristics;
	// calculate lowest age and highest age in the ranges
	for(std::map<int, PersonCharacteristics>::const_iterator iter=personCharacteristics.begin();iter != personCharacteristics.end();iter++) {
		if(cfg.personCharacteristicsParams.lowestAge > iter->second.lowerAge) {
			cfg.personCharacteristicsParams.lowestAge = iter->second.lowerAge;
		}
		if(cfg.personCharacteristicsParams.highestAge < iter->second.upperAge) {
			cfg.personCharacteristicsParams.highestAge = iter->second.upperAge;
		}
	}
}

void sim_mob::ParseConfigFile::ProcessConstructsNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}

	//Process each item in order.
	//ProcessConstructDistributionsNode(GetSingleElementByName(node, "distributions")); //<TODO
	ProcessConstructDatabasesNode(GetSingleElementByName(node, "databases"));
	ProcessConstructDbProcGroupsNode(GetSingleElementByName(node, "db_proc_groups"));
	ProcessConstructCredentialsNode(GetSingleElementByName(node, "credentials"));
	ProcessConstructExternalScriptsNode(GetSingleElementByName(node, "external_scripts"));
	ProcessConstructMongoCollectionsNode(GetSingleElementByName(node, "mongo_collections"));
}


void sim_mob::ParseConfigFile::ProcessConstructDatabasesNode(xercesc::DOMElement* node)
{
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		if (TranscodeString(item->getNodeName())!="database") {
			Warn() <<"Invalid databases child node.\n";
			continue;
		}

		//Retrieve some attributes from the Node itself.
		Database db(ParseString(GetNamedAttributeValue(item, "id")));
		std::string dbType = ParseString(GetNamedAttributeValue(item, "dbtype"), "");
		if (dbType != "postgres" && dbType != "mongodb") {
			throw std::runtime_error("Database type not supported.");
		}

		//Now retrieve the required parameters from child nodes.
		db.host = ProcessValueString(GetSingleElementByName(item, "host"));
		db.port = ProcessValueString(GetSingleElementByName(item, "port"));
		db.dbName = ProcessValueString(GetSingleElementByName(item, "dbname"));

		cfg.constructs.databases[db.getId()] = db;
	}
}

void sim_mob::ParseConfigFile::ProcessConstructDbProcGroupsNode(xercesc::DOMElement* node)
{
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		if (TranscodeString(item->getNodeName())!="proc_map") {
			Warn() <<"Invalid db_proc_groups child node.\n";
			continue;
		}

		//Retrieve some attributes from the Node itself.
		StoredProcedureMap pm(ParseString(GetNamedAttributeValue(item, "id")));
		pm.dbFormat = ParseString(GetNamedAttributeValue(item, "format"), "");
		if (pm.dbFormat != "aimsun" && pm.dbFormat != "long-term") {
			throw std::runtime_error("Stored procedure map format not supported.");
		}

		//Loop through and save child attributes.
		for (DOMElement* mapItem=item->getFirstElementChild(); mapItem; mapItem=mapItem->getNextElementSibling()) {
			if (TranscodeString(mapItem->getNodeName())!="mapping") {
				Warn() <<"Invalid proc_map child node.\n";
				continue;
			}

			std::string key = ParseString(GetNamedAttributeValue(mapItem, "name"), "");
			std::string val = ParseString(GetNamedAttributeValue(mapItem, "procedure"), "");
			if (key.empty() || val.empty()) {
				Warn() <<"Invalid mapping; missing \"name\" or \"procedure\".\n";
				continue;
			}

			pm.procedureMappings[key] = val;
		}

		cfg.constructs.procedureMaps[pm.getId()] = pm;
	}
}

void sim_mob::ParseConfigFile::ProcessConstructCredentialsNode(xercesc::DOMElement* node)
{
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		std::string name = TranscodeString(item->getNodeName());
		if (name!="file-based-credential" && name!="plaintext-credential") {
			Warn() <<"Invalid db_proc_groups child node.\n";
			continue;
		}

		//Retrieve some attributes from the Node itself.
		Credential cred(ParseString(GetNamedAttributeValue(item, "id")));

		//Setting the actual credentials depends on the type of node.
		if (name=="file-based-credential") {
			//Scan children for "path" nodes.
			std::vector<std::string> paths;
			for (DOMElement* pathItem=item->getFirstElementChild(); pathItem; pathItem=pathItem->getNextElementSibling()) {
				if (TranscodeString(pathItem->getNodeName())!="file") {
					Warn() <<"file-based credentials contain invalid child node; expected path.\n";
					continue;
				}
				std::string path = ParseString(GetNamedAttributeValue(pathItem, "path"), "");
				if (!path.empty()) {
					paths.push_back(path);
				}
			}

			//Try setting it.
			cred.LoadFileCredentials(paths);
		} else if (name=="plaintext-credential") {
			//Retrieve children manually.
			std::string username = ParseString(GetNamedAttributeValue(GetSingleElementByName(item, "username"), "value"), "");
			std::string password = ParseString(GetNamedAttributeValue(GetSingleElementByName(item, "password"), "value"), "");
			cred.SetPlaintextCredentials(username, password);
		} else {
			throw std::runtime_error("Unexpected (but allowed) credentials.");
		}

		//Save it.
		cfg.constructs.credentials[cred.getId()] = cred;
	}
}

void sim_mob::ParseConfigFile::ProcessConstructExternalScriptsNode(xercesc::DOMElement* node)
{
	std::string id = ParseString(GetNamedAttributeValue(node, "id"), "");
	if(id.empty()) {
		throw std::runtime_error("id cannot be empty. Check external_scripts node.");
	}

	std::string format = ParseString(GetNamedAttributeValue(node, "format"), "");
	if(format.empty() || format != "lua") {
		throw std::runtime_error("Unsupported script format");
	}

	std::string scriptsDirectoryPath = ParseString(GetNamedAttributeValue(node, "path"), "");
	if (scriptsDirectoryPath.empty()) {
		throw std::runtime_error("path to scripts is not provided");
	}
	if((*scriptsDirectoryPath.rbegin()) != '/') {
		//add a / to the end of the path string if it is not already there
		scriptsDirectoryPath.push_back('/');
	}
	ExternalScriptsMap sm(id, scriptsDirectoryPath , format);
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		std::string name = TranscodeString(item->getNodeName());
		if (name!="script") {
			Warn() <<"Invalid db_proc_groups child node.\n";
			continue;
		}

		std::string key = ParseString(GetNamedAttributeValue(item, "name"), "");
		std::string val = ParseString(GetNamedAttributeValue(item, "file"), "");
		if (key.empty() || val.empty()) {
			Warn() <<"Invalid script; missing \"name\" or \"file\".\n";
			continue;
		}

		sm.scriptFileName[key] = val;
	}
	cfg.constructs.externalScriptsMap[sm.getId()] = sm;
}

void sim_mob::ParseConfigFile::ProcessConstructMongoCollectionsNode(xercesc::DOMElement* node) {
	MongoCollectionsMap mongoColls(ParseString(GetNamedAttributeValue(node, "id"), ""), ParseString(GetNamedAttributeValue(node, "db_name"), ""));
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		std::string name = TranscodeString(item->getNodeName());
		if (name!="mongo_collection") {
				Warn() <<"Invalid db_proc_groups child node.\n";
				continue;
		}
		std::string key = ParseString(GetNamedAttributeValue(item, "name"), "");
		std::string val = ParseString(GetNamedAttributeValue(item, "collection"), "");
		if (key.empty() || val.empty()) {
			Warn() <<"Invalid mongo_collection; missing \"name\" or \"collection\".\n";
			continue;
		}
		mongoColls.collectionName[key] = val;
	}
	cfg.constructs.mongoCollectionsMap[mongoColls.getId()] = mongoColls;
}


void sim_mob::ParseConfigFile::ProcessLongTermParamsNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}

	//The longtermParams tag has an attribute
	cfg.ltParams.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	//Now set the rest.
	cfg.ltParams.days = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "days"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.maxIterations = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "maxIterations"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.tickStep = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "tickStep"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.workers = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "workers"), "value"), static_cast<unsigned int>(0));
}


void sim_mob::ParseConfigFile::ProcessFMOD_Node(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}

	//The fmod tag has an attribute
	cfg.fmod.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	//Now set the rest.
	cfg.fmod.ipAddress = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "ip_address"), "value"), "");
	cfg.fmod.port = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "port"), "value"), static_cast<unsigned int>(0));
	cfg.fmod.updateTimeMS = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "update_time_ms"), "value"), static_cast<unsigned int>(0));
	cfg.fmod.mapfile = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "map_file"), "value"), "");
	cfg.fmod.blockingTimeSec = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "blocking_time_sec"), "value"), static_cast<unsigned int>(0));
}


void sim_mob::ParseConfigFile::ProcessDriversNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}
	/*XMLCh* keyX = XMLString::transcode("driver");
	DOMNodeList* nodes = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);*/

	ProcessFutureAgentList(node, "driver", cfg.driverTemplates);
}

void sim_mob::ParseConfigFile::ProcessPedestriansNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}
	/*XMLCh* keyX = XMLString::transcode("pedestrian");
	DOMNodeList* nodes = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);*/

	ProcessFutureAgentList(node, "pedestrian", cfg.pedestrianTemplates);
}

void sim_mob::ParseConfigFile::ProcessBusDriversNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}
	/*XMLCh* keyX = XMLString::transcode("busdriver");
	DOMNodeList* nodes = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);*/

	ProcessFutureAgentList(node, "busdriver", cfg.busDriverTemplates);
}

void sim_mob::ParseConfigFile::ProcessPassengersNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}
	/*XMLCh* keyX = XMLString::transcode("busdriver");
	DOMNodeList* nodes = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);*/

	ProcessFutureAgentList(node, "passenger", cfg.passengerTemplates);
}

void sim_mob::ParseConfigFile::ProcessSignalsNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}
	/*XMLCh* keyX = XMLString::transcode("signal");
	DOMNodeList* nodes = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);*/

	ProcessFutureAgentList(node, "signal", cfg.signalTemplates, true, true, false, false);
}

void sim_mob::ParseConfigFile::ProcessBusControllersNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}
	/*XMLCh* keyX = XMLString::transcode("signal");
	DOMNodeList* nodes = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);*/

	ProcessFutureAgentList(node, "buscontroller", cfg.busControllerTemplates, false, false, true, false);
}

void sim_mob::ParseConfigFile::ProcessSystemSimulationNode(xercesc::DOMElement* node)
{
	//Several properties are set up as "x ms", or "x seconds", etc.
	cfg.system.simulation.baseGranMS = ProcessTimegranUnits(GetSingleElementByName(node, "base_granularity", true));
	cfg.system.simulation.baseGranSecond = cfg.system.simulation.baseGranMS / MILLISECONDS_IN_SECOND;
	cfg.system.simulation.totalRuntimeMS = ProcessTimegranUnits(GetSingleElementByName(node, "total_runtime", true));
	cfg.system.simulation.totalWarmupMS = ProcessTimegranUnits(GetSingleElementByName(node, "total_warmup"));

	//These should all be moved; for now we copy them over with one command.
	cfg.system.simulation.reactTimeDistribution1.typeId = ProcessValueInteger(GetSingleElementByName(node, "reacTime_distributionType1"));
	cfg.system.simulation.reactTimeDistribution2.typeId = ProcessValueInteger(GetSingleElementByName(node, "reacTime_distributionType2"));
	cfg.system.simulation.reactTimeDistribution1.mean = ProcessValueInteger(GetSingleElementByName(node, "reacTime_mean1"));
	cfg.system.simulation.reactTimeDistribution2.mean = ProcessValueInteger(GetSingleElementByName(node, "reacTime_mean2"));
	cfg.system.simulation.reactTimeDistribution1.stdev = ProcessValueInteger(GetSingleElementByName(node, "reacTime_standardDev1"));
	cfg.system.simulation.reactTimeDistribution2.stdev = ProcessValueInteger(GetSingleElementByName(node, "reacTime_standardDev2"));

	cfg.system.simulation.simStartTime = ProcessValueDailyTime(GetSingleElementByName(node, "start_time", true));
	//save travel time table name
	if( ConfigManager::GetInstance().FullConfig().PathSetMode() )
	{
		DOMElement* rn = GetSingleElementByName(node, "pathset_traveltime_save_table",true);
		if (rn) {
		cfg.system.simulation.travelTimeTmpTableName  =
				ParseString(GetNamedAttributeValue(rn, "value"),"aa");
		}
		else
		{
			cfg.system.simulation.travelTimeTmpTableName = "no_name";
		}
//				ParseString(GetNamedAttributeValue(node, "database"), "");
//		TiXmlElement* node_table_name = handle.FirstChild("pathset_travletime_save_table").ToElement();
//		const char* node_table_name_char = node_table_name ? node_table_name->Attribute("value") : nullptr;
//		if(node_table_name_char==nullptr)
//		{
//			throw std::runtime_error("pls add pathset_travletime_save_table to config file");
//		}
//		std::string node_table_name_str = std::string(node_table_name_char);
//		ConfigParams::GetInstance().travelTimeTmpTableName = node_table_name_str;
//		PathSetManager::getInstance()->setTravleTimeTmpTableName(node_table_name_str);
	}
	//Now we're getting back to real properties.
	ProcessSystemAuraManagerImplNode(GetSingleElementByName(node, "aura_manager_impl"));
	ProcessSystemWorkgroupAssignmentNode(GetSingleElementByName(node, "workgroup_assignment"));
	cfg.system.simulation.partitioningSolutionId = ProcessValueInteger(GetSingleElementByName(node, "partitioning_solution_id"));
	ProcessSystemLoadAgentsOrderNode(GetSingleElementByName(node, "load_agents"));
	cfg.system.simulation.startingAutoAgentID = ProcessValueInteger2(GetSingleElementByName(node, "auto_id_start"), 0);
	ProcessSystemMutexEnforcementNode(GetSingleElementByName(node, "mutex_enforcement"));
	ProcessSystemCommsimNode(GetSingleElementByName(node, "commsim"));

	//Warn against the old communication format.
	if (GetSingleElementByName(node, "communication", false)) {
		throw std::runtime_error("Using old parameter: \"communication\"; please review the new \"XXX\" format.");
	}
}

void sim_mob::ParseConfigFile::ProcessSystemWorkersNode(xercesc::DOMElement* node)
{
	ProcessWorkerPersonNode(GetSingleElementByName(node, "person", true));
	ProcessWorkerSignalNode(GetSingleElementByName(node, "signal", true));
	ProcessWorkerCommunicationNode(GetSingleElementByName(node, "communication", true));

}

void sim_mob::ParseConfigFile::ProcessSystemSingleThreadedNode(xercesc::DOMElement* node)
{
	//TODO: GetAttribute not working; there's a null throwing it off.
	cfg.system.singleThreaded = ParseBoolean(GetNamedAttributeValue(node, "value"), false);
}


void sim_mob::ParseConfigFile::ProcessSystemMergeLogFilesNode(xercesc::DOMElement* node)
{
	cfg.system.mergeLogFiles = ParseBoolean(GetNamedAttributeValue(node, "value"), false);
}


void sim_mob::ParseConfigFile::ProcessSystemNetworkSourceNode(xercesc::DOMElement* node)
{
	cfg.system.networkSource = ParseNetSourceEnum(GetNamedAttributeValue(node, "value"), SystemParams::NETSRC_XML);
}


void sim_mob::ParseConfigFile::ProcessSystemNetworkXmlInputFileNode(xercesc::DOMElement* node)
{
	cfg.system.networkXmlInputFile = ParseNonemptyString(GetNamedAttributeValue(node, "value"), "private/SimMobilityInput.xml");
}

void sim_mob::ParseConfigFile::ProcessSystemNetworkXmlOutputFileNode(xercesc::DOMElement* node)
{
	cfg.system.networkXmlOutputFile = ParseNonemptyString(GetNamedAttributeValue(node, "value"), "");//or NetworkCopy.xml
}

void sim_mob::ParseConfigFile::ProcessSystemDatabaseNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}
	cfg.system.networkDatabase.database = ParseString(GetNamedAttributeValue(node, "database"), "");
	cfg.system.networkDatabase.credentials = ParseString(GetNamedAttributeValue(node, "credentials"), "");
	cfg.system.networkDatabase.procedures = ParseString(GetNamedAttributeValue(node, "proc_map"), "");
}

void sim_mob::ParseConfigFile::ProcessSystemXmlSchemaFilesNode(xercesc::DOMElement* node)
{
	//For now, only the Road Network has an XSD file (doing this for the config file from within it would be difficult).
	DOMElement* rn = GetSingleElementByName(node, "road_network");
	if (rn) {
		std::vector<DOMElement*> options = GetElementsByName(rn, "option");
		for (std::vector<DOMElement*>::const_iterator it=options.begin(); it!=options.end(); it++) {
			std::string path = ParseString(GetNamedAttributeValue(*it, "value"), "");
			if (!path.empty()) {
				//See if the file exists.
				if (boost::filesystem::exists(path)) {
					//Convert it to an absolute path.
					boost::filesystem::path abs_path = boost::filesystem::absolute(path);
					cfg.system.roadNetworkXsdSchemaFile = abs_path.string();
					break;
				}
			}
		}

		//Did we try and find nothing?
		if (!options.empty() && cfg.system.roadNetworkXsdSchemaFile.empty()) {
			Warn() <<"Warning: No viable options for road_network schema file." <<std::endl;
		}
	}
}


void sim_mob::ParseConfigFile::ProcessSystemGenericPropsNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}

	std::vector<DOMElement*> properties = GetElementsByName(node, "property");
	for (std::vector<DOMElement*>::const_iterator it=properties.begin(); it!=properties.end(); it++) {
		std::string key = ParseString(GetNamedAttributeValue(*it, "key"), "");
		std::string val = ParseString(GetNamedAttributeValue(*it, "value"), "");
		if (!(key.empty() && val.empty())) {
			cfg.system.genericProps[key] = val;
		}
	}
}


unsigned int sim_mob::ParseConfigFile::ProcessTimegranUnits(xercesc::DOMElement* node)
{
	return ParseTimegranAsMs(GetNamedAttributeValue(node, "value"), GetNamedAttributeValue(node, "units"));
}

bool sim_mob::ParseConfigFile::ProcessValueBoolean(xercesc::DOMElement* node)
{
	return ParseBoolean(GetNamedAttributeValue(node, "value"));
}

int sim_mob::ParseConfigFile::ProcessValueInteger(xercesc::DOMElement* node)
{
	return ParseInteger(GetNamedAttributeValue(node, "value"));
}

DailyTime sim_mob::ParseConfigFile::ProcessValueDailyTime(xercesc::DOMElement* node)
{
	return ParseDailyTime(GetNamedAttributeValue(node, "value"));
}

void sim_mob::ParseConfigFile::ProcessSystemAuraManagerImplNode(xercesc::DOMElement* node)
{
	cfg.system.simulation.auraManagerImplementation = ParseAuraMgrImplEnum(GetNamedAttributeValue(node, "value"), AuraManager::IMPL_RSTAR);
}

void sim_mob::ParseConfigFile::ProcessSystemWorkgroupAssignmentNode(xercesc::DOMElement* node)
{
	cfg.system.simulation.workGroupAssigmentStrategy = ParseWrkGrpAssignEnum(GetNamedAttributeValue(node, "value"), WorkGroup::ASSIGN_SMALLEST);
}

void sim_mob::ParseConfigFile::ProcessSystemLoadAgentsOrderNode(xercesc::DOMElement* node)
{
	//Separate into a string array.
	std::string value = ParseString(GetNamedAttributeValue(node, "order"), "");
	std::vector<std::string> valArray;
	boost::split(valArray, value, boost::is_any_of(", "), boost::token_compress_on);

	//Now, turn into an enum array.
	for (std::vector<std::string>::const_iterator it=valArray.begin(); it!=valArray.end(); it++) {
		SimulationParams::LoadAgentsOrderOption opt(SimulationParams::LoadAg_Database);
		if ((*it) == "database") {
			opt = SimulationParams::LoadAg_Database;
		} else if ((*it) == "drivers") {
			opt = SimulationParams::LoadAg_Drivers;
		} else if ((*it) == "pedestrians") {
			opt = SimulationParams::LoadAg_Pedestrians;
		} else if ((*it) == "passengers") {
			opt = SimulationParams::LoadAg_Passengers;
		} else if ((*it) == "xml-tripchains") {
			opt = SimulationParams::LoadAg_XmlTripChains;
		} else {
			std::stringstream out;
			out.str("");
			out << "Unexpected load_agents order param." << "[" << *it << "]";
			throw std::runtime_error(out.str());
		}
		cfg.system.simulation.loadAgentsOrder.push_back(opt);
	}
}

void sim_mob::ParseConfigFile::ProcessSystemMutexEnforcementNode(xercesc::DOMElement* node)
{
	cfg.system.simulation.mutexStategy = ParseMutexStrategyEnum(GetNamedAttributeValue(node, "strategy"), MtxStrat_Buffered);
}

void sim_mob::ParseConfigFile::ProcessSystemCommsimNode(xercesc::DOMElement* node)
{
	if (!node) { return; }

	//Enabled?
	cfg.system.simulation.commsim.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	//Minimum clients
	cfg.system.simulation.commsim.minClients = ProcessValueInteger(GetSingleElementByName(node, "min_clients", true));

	//Hold tick
	cfg.system.simulation.commsim.holdTick = ProcessValueInteger(GetSingleElementByName(node, "hold_tick", true));

	//Use ns-3 for routing?
	cfg.system.simulation.commsim.useNs3 = ProcessValueBoolean(GetSingleElementByName(node, "use_ns3", true));
}

void sim_mob::ParseConfigFile::ProcessWorkerPersonNode(xercesc::DOMElement* node)
{
	cfg.system.workers.person.count = ParseInteger(GetNamedAttributeValue(node, "count"));
	cfg.system.workers.person.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
}


void sim_mob::ParseConfigFile::ProcessWorkerSignalNode(xercesc::DOMElement* node)
{
	cfg.system.workers.signal.count = ParseInteger(GetNamedAttributeValue(node, "count"));
	cfg.system.workers.signal.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
}

void sim_mob::ParseConfigFile::ProcessWorkerCommunicationNode(xercesc::DOMElement* node)
{
	cfg.system.workers.communication.count = ParseInteger(GetNamedAttributeValue(node, "count"));
	cfg.system.workers.communication.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
}

/*void sim_mob::ParseConfigFile::ProcessGeomDbConnection(xercesc::DOMElement* node)
{
	std::set<std::string> passed; //Kind of messy; format needs a redesign.
	std::vector<DOMElement*> params = GetElementsByName(node, "param");
	for (std::vector<DOMElement*>::const_iterator it=params.begin(); it!=params.end(); it++) {
		std::string pKey = ParseString(GetNamedAttributeValue(*it, "name", true));
		std::transform(pKey.begin(), pKey.end(), pKey.begin(), ::tolower);
		std::string pVal = ParseString(GetNamedAttributeValue(*it, "value", true));

		//Make sure we tag each property.
		if (pKey=="host") {
			cfg.geometry.connection.host = pVal;
		} else if (pKey=="port") {
			cfg.geometry.connection.port = pVal;
		} else if (pKey=="dbname") {
			cfg.geometry.connection.dbName = pVal;
		} else if (pKey=="user") {
			cfg.geometry.connection.user = pVal;
		} else if (pKey=="password") {
			cfg.geometry.connection.password = Password(pVal);
		} else {
			//Skip unknown properties (can also do an error).
			continue;
		}

		//Tag it.
		passed.insert(pKey);
	}

	//Did we get everything?
	if (passed.size()<5) {
		Warn() <<"Database connection has too few parameters; connection attempts may fail.\n";
	}
}*/


/*void sim_mob::ParseConfigFile::ProcessGeomDbMappings(xercesc::DOMElement* node)
{
	//These are so backwards...
	for (DOMElement* prop=node->getFirstElementChild(); prop; prop=prop->getNextElementSibling()) {
		//Save its name/procedure type in the procedures map.
		cfg.geometry.procedures.procedureMappings[TranscodeString(prop->getNodeName())] = TranscodeString(GetNamedAttributeValue(prop, "procedure", true));
	}
}*/


void sim_mob::ParseConfigFile::ProcessFutureAgentList(xercesc::DOMElement* node, const std::string& itemName, std::vector<EntityTemplate>& res, bool originReq, bool destReq, bool timeReq, bool laneReq)
{
	//We use the existing "element child" functions, it's significantly slower to use "getElementsByTagName()"
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		if (TranscodeString(item->getNodeName())==itemName) {
			EntityTemplate ent;
			ent.originPos = ParsePoint2D(GetNamedAttributeValue(item, "originPos", originReq),Point2D());
			ent.destPos = ParsePoint2D(GetNamedAttributeValue(item, "destPos", destReq), Point2D());
			ent.startTimeMs = ParseUnsignedInt(GetNamedAttributeValue(item, "time", timeReq), static_cast<unsigned int>(0));
			ent.laneIndex = ParseUnsignedInt(GetNamedAttributeValue(item, "lane", laneReq), static_cast<unsigned int>(0));
			res.push_back(ent);
		}
	}
}

void sim_mob::ParseConfigFile::ProcessIncidentsNode(xercesc::DOMElement* node)
{
	if(!node) {
		return;
	}

	bool enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
	if(!enabled){
		return;
	}

	for(DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		IncidentParams incident;
		incident.incidentId = ParseUnsignedInt(GetNamedAttributeValue(item, "id"));
		incident.visibilityDistance = ParseFloat(GetNamedAttributeValue(item, "visibility"));
		incident.segmentId = ParseUnsignedInt(GetNamedAttributeValue(item, "segment") );
		incident.position = ParseFloat(GetNamedAttributeValue(item, "position"));
		incident.capFactor = ParseFloat(GetNamedAttributeValue(item, "cap_factor") );
		incident.startTime = ParseDailyTime(GetNamedAttributeValue(item, "start_time") ).getValue();
		incident.duration = ParseDailyTime(GetNamedAttributeValue(item, "duration") ).getValue();
		incident.length = ParseFloat(GetNamedAttributeValue(item, "length") );
		incident.compliance = ParseFloat(GetNamedAttributeValue(item, "compliance") );
		incident.accessibility = ParseFloat(GetNamedAttributeValue(item, "accessibility") );

		for(DOMElement* child=item->getFirstElementChild(); child; child=child->getNextElementSibling()){
			IncidentParams::LaneParams lane;
			lane.laneId = ParseUnsignedInt(GetNamedAttributeValue(child, "laneId"));
			lane.speedLimit = ParseFloat(GetNamedAttributeValue(child, "speedLimitFactor") );
			incident.laneParams.push_back(lane);
		}

		cfg.incidents.push_back(incident);
	}
}

