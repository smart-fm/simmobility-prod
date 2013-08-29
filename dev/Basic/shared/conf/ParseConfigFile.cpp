/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

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


int ParseInteger(const XMLCh* srcX, int* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		return boost::lexical_cast<int>(src);
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory integer variable; no default available.");
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


} //End un-named namespace


sim_mob::ParseConfigFile::ParseConfigFile(const std::string& configFileName, RawConfigParams& result) : cfg(result), inFilePath(configFileName)
{
	//Print the network legacy format to our log file.
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

	//Now just parse the document recursively.
	ProcessSystemNode(GetSingleElementByName(rootNode,"system", true));
	ProcessGeometryNode(GetSingleElementByName(rootNode, "geometry", true));
	ProcessDriversNode(GetSingleElementByName(rootNode, "drivers"));
	ProcessPedestriansNode(GetSingleElementByName(rootNode, "pedestrians"));
	ProcessBusDriversNode(GetSingleElementByName(rootNode, "busdrivers"));
	ProcessSignalsNode(GetSingleElementByName(rootNode, "signals"));

	//TEMP
	throw 1;
}


void sim_mob::ParseConfigFile::ProcessSystemNode(DOMElement* node)
{
	ProcessSystemSimulationNode(GetSingleElementByName(node, "simulation", true));
	ProcessSystemWorkersNode(GetSingleElementByName(node, "workers", true));
	ProcessSystemSingleThreadedNode(GetSingleElementByName(node, "single_threaded"));
	ProcessSystemMergeLogFilesNode(GetSingleElementByName(node, "merge_log_files"));
	ProcessSystemNetworkSourceNode(GetSingleElementByName(node, "network_source"));
	ProcessSystemNetworkXmlFileNode(GetSingleElementByName(node, "network_xml_file"));
	ProcessSystemXmlSchemaFilesNode(GetSingleElementByName(node, "xsd_schema_files", true));
	ProcessSystemGenericPropsNode(GetSingleElementByName(node, "generic_props"));
}


void sim_mob::ParseConfigFile::ProcessGeometryNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessDriversNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessPedestriansNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessBusDriversNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessSignalsNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessSystemSimulationNode(xercesc::DOMElement* node)
{
	//Several properties are set up as "x ms", or "x seconds", etc.
	cfg.system.simulation.baseGranMS = ProcessTimegranUnits(GetSingleElementByName(node, "base_granularity", true));
	cfg.system.simulation.totalRuntimeMS = ProcessTimegranUnits(GetSingleElementByName(node, "total_runtime", true));
	cfg.system.simulation.totalWarmupMS = ProcessTimegranUnits(GetSingleElementByName(node, "total_warmup"));

	//These should all be moved; for now we copy them over with one command.
	cfg.system.simulation.reacTime_distributionType1 = ProcessValueInteger(GetSingleElementByName(node, "reacTime_distributionType1"));
	cfg.system.simulation.reacTime_distributionType2 = ProcessValueInteger(GetSingleElementByName(node, "reacTime_distributionType2"));
	cfg.system.simulation.reacTime_mean1 = ProcessValueInteger(GetSingleElementByName(node, "reacTime_mean1"));
	cfg.system.simulation.reacTime_mean2 = ProcessValueInteger(GetSingleElementByName(node, "reacTime_mean2"));
	cfg.system.simulation.reacTime_standardDev1 = ProcessValueInteger(GetSingleElementByName(node, "reacTime_standardDev1"));
	cfg.system.simulation.reacTime_standardDev2 = ProcessValueInteger(GetSingleElementByName(node, "reacTime_standardDev2"));

	cfg.system.simulation.simStartTime = ProcessValueDailyTime(GetSingleElementByName(node, "start_time", true));

	//Now we're getting back to real properties.
	ProcessSystemAuraManagerImplNode(GetSingleElementByName(node, "aura_manager_impl"));
	ProcessSystemWorkgroupAssignmentNode(GetSingleElementByName(node, "workgroup_assignment"));
	cfg.system.simulation.partitioningSolutionId = ProcessValueInteger(GetSingleElementByName(node, "partitioning_solution_id"));
	ProcessSystemLoadAgentsOrderNode(GetSingleElementByName(node, "load_agents"));
	cfg.system.simulation.startingAutoAgentID = ProcessValueInteger2(GetSingleElementByName(node, "auto_id_start"), 0);
	ProcessSystemMutexEnforcementNode(GetSingleElementByName(node, "mutex_enforcement"));
	ProcessSystemCommunicationNode(GetSingleElementByName(node, "communication"));
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


void sim_mob::ParseConfigFile::ProcessSystemNetworkXmlFileNode(xercesc::DOMElement* node)
{
	cfg.system.networkXmlFile = ParseNonemptyString(GetNamedAttributeValue(node, "value"), "data/SimMobilityInput.xml");
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
		} else {
			throw std::runtime_error("Unexpected load_agents order param.");
		}
		cfg.system.simulation.loadAgentsOrder.push_back(opt);
	}
}

void sim_mob::ParseConfigFile::ProcessSystemMutexEnforcementNode(xercesc::DOMElement* node)
{
	cfg.system.simulation.mutexStategy = ParseMutexStrategyEnum(GetNamedAttributeValue(node, "strategy"), MtxStrat_Buffered);
}

void sim_mob::ParseConfigFile::ProcessSystemCommunicationNode(xercesc::DOMElement* node)
{
	//The commsim config has an attribute and a child node.
	cfg.system.simulation.commSimEnabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	//TODO: There is a "type" attribute here too, in the latest branch.
	xercesc::DOMElement* androidNode = GetSingleElementByName(node, "android_testbed");
	cfg.system.simulation.androidClientEnabled = ParseBoolean(GetNamedAttributeValue(androidNode, "enabled"), false);
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


