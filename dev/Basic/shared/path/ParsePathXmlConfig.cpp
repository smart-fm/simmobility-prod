#include "ParsePathXmlConfig.hpp"
#include "util/XmlParseHelper.hpp"
#include <xercesc/dom/DOM.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>

using namespace xercesc;
using namespace std;
//Helper: turn a Xerces error message into a string.
std::string TranscodeString(const XMLCh* str) {
	char* raw = XMLString::transcode(str);
	std::string res(raw);
	XMLString::release(&raw);
	return res;
}

sim_mob::ParsePathXmlConfig::ParsePathXmlConfig(const std::string& configFileName, PathSetConf& result) : cfg(result), ParseConfigXmlBase(configFileName)
{
	if(configFileName.size() == 0)
	{
		throw std::runtime_error("pathset config file not specified");
	}
	parseXmlAndProcess();
}

void sim_mob::ParsePathXmlConfig::processXmlFile(XercesDOMParser& parser)
{
	//Verify that the root node is "config"
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	if (TranscodeString(rootNode->getTagName()) != "pathset") {
		throw std::runtime_error("xml parse error: root node of path set configuration file must be \"pathset\"");
	}

	ProcessPathSetNode(rootNode);
}


void sim_mob::ParsePathXmlConfig::ProcessPathSetNode(xercesc::DOMElement* node){

	if (!node) {
		std::cerr << "Pathset Configuration Not Found\n" ;
		return;
	}

	if(!(cfg.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), "false")))
	{
		return;
	}

	if((cfg.mode = ParseString(GetNamedAttributeValue(node, "mode"), "")) == "")
	{
		throw  std::runtime_error ("No pathset mode specified, \"normal\" and \"generation\" modes are supported supported\n");
		//todo remove hardcoding and add a container, if required
		if(cfg.mode != "normal" && cfg.mode != "generation")
		{
			std::cerr << "currently, only \"normal\" and \"generation\" is supported as pathset mode, resetting to \"normal\" mode\n";
			cfg.mode = "normal";
		}
	}

	//bulk pathset generation
	if(cfg.mode == "generation")
	{
		xercesc::DOMElement* bulk = GetSingleElementByName(node, "bulk_generation_output_file_name");
		if(!bulk){
			throw std::runtime_error("pathset is in \"generation\" mode but bulk_generation_output_file_name is not found in pathset xml config\n");
		}
		else
		{
			cfg.bulkFile = ParseString(GetNamedAttributeValue(bulk, "value"), "");
		}
	}

	//pathset generation threadpool size
	xercesc::DOMElement* poolSize = GetSingleElementByName(node, "thread_pool");
	if(!poolSize){
		std::cerr << "Pathset generation thread pool size not specified, defaulting to 5\n";
	}
	else
	{
		cfg.threadPoolSize = ParseInteger(GetNamedAttributeValue(poolSize, "size"), 5);
	}

	xercesc::DOMElement* dbNode = GetSingleElementByName(node, "pathset_database");
	if(!dbNode){
		throw std::runtime_error("Path Set Data Base Credentials not found\n");
	}
	else
	{
		cfg.networkDatabase.database = ParseString(GetNamedAttributeValue(dbNode, "database"), "");
		cfg.networkDatabase.credentials = ParseString(GetNamedAttributeValue(dbNode, "credentials"), "");
	}

	xercesc::DOMElement* tableNode = GetSingleElementByName(node, "tables");
	if(!tableNode){
		throw std::runtime_error("Pathset Tables specification not found");
	}
	else
	{
		cfg.pathSetTableName = ParseString(GetNamedAttributeValue(tableNode, "singlepath_table"), "");
		cfg.RTTT_Conf  = ParseString(GetNamedAttributeValue(tableNode, "realtime_traveltime"), "");
		cfg.DTT_Conf  = ParseString(GetNamedAttributeValue(tableNode, "default_traveltime"), "");
	}
	//function
	xercesc::DOMElement* functionNode = GetSingleElementByName(node, "functions");
	if(!functionNode){
		throw std::runtime_error("Pathset Stored Procedure Not Found\n");
	}
	else
	{
		cfg.psRetrieval = ParseString(GetNamedAttributeValue(functionNode, "pathset"), "");
		cfg.upsert = ParseString(GetNamedAttributeValue(functionNode, "travel_time"), "");
	}

	//recirsive pathset generation
	xercesc::DOMElement* recPS = GetSingleElementByName(node, "recursive_pathset_generation");
	if(!recPS){
		std::cerr << "recursive_pathset_generation Not Found, setting to false\n";
		cfg.recPS = false;
	}
	else
	{
		cfg.recPS = ParseBoolean(GetNamedAttributeValue(recPS, "value"), false);
	}

	//reroute
	xercesc::DOMElement* reroute = GetSingleElementByName(node, "reroute_enabled");
	if(!reroute){
		std::cerr << "reroute_enabled Not Found, setting to false\n";
		cfg.reroute = false;
	}
	else
	{
		cfg.reroute = ParseBoolean(GetNamedAttributeValue(reroute, "value"), false);
	}

	//CBD //todo: usage still unclear
	xercesc::DOMElement* cbd = GetSingleElementByName(node, "CBD_enabled");
	if(!cbd){
		std::cerr << "CBD_enabled Not Found, setting to false\n";
		cfg.cbd = false;
	}
	else
	{
		cfg.cbd = ParseBoolean(GetNamedAttributeValue(cbd, "value"), false);
	}


	//subtrip output for preday
	xercesc::DOMElement* predayOP = GetSingleElementByName(node, "subtrip_travel_metrics_output");
	if(predayOP)
	{
		const XMLCh* enabledSwitch = GetNamedAttributeValue(predayOP, "enabled");
		if(!enabledSwitch)
		{
			throw std::runtime_error("mandatory subtrip_travel_metrics_output \"enabled\" switch is missing");
		}
		if(ParseBoolean(enabledSwitch))
		{
			cfg.subTripOP = ParseString(GetNamedAttributeValue(predayOP, "file"), "");
			if(!cfg.subTripOP.size())
			{
				throw std::runtime_error("mandatory subtrip_travel_metrics_output filename is missing");
			}
		}
	}

	///	path generators configuration
	xercesc::DOMElement* gen = GetSingleElementByName(node, "path_generators");
	if(gen)
	{
		//random perturbation
		xercesc::DOMElement* random = GetSingleElementByName(gen, "random_perturbation");
		if(random)
		{
			cfg.perturbationIteration  = ParseInteger(GetNamedAttributeValue(random, "iteration"),0);
			std::vector<std::string> rangeStrVec;
			std::string rangerStr = ParseString(GetNamedAttributeValue(random, "uniform_range"),"0,0");
			boost::split(rangeStrVec,rangerStr,boost::is_any_of(","));
			cfg.perturbationRange.first = boost::lexical_cast<int>(rangeStrVec[0]);
			cfg.perturbationRange.second = boost::lexical_cast<int>(rangeStrVec[1]);
		}

		//K-shortest Path
		xercesc::DOMElement* ksp = GetSingleElementByName(gen, "k_shortest");
		if(ksp)
		{
			cfg.kspLevel  = ParseInteger(GetNamedAttributeValue(ksp, "level"),0);
		}

		//Link Elimination
		xercesc::DOMElement* LE = GetSingleElementByName(gen, "link_elimination");
		if(LE)
		{
			std::string types;
			types  = ParseString(GetNamedAttributeValue(LE, "types"),"");
			boost::split(cfg.LE, types, boost::is_any_of(","));
		}
	}
	//path generators ...

	//utility parameters
	xercesc::DOMElement* utility = GetSingleElementByName(node, "utility_parameters");
	if(utility)
	{
		cfg.params.bTTVOT = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "bTTVOT"), "value"),-0.01373);//-0.0108879;
		cfg.params.bCommonFactor = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "bCommonFactor"), "value"),1.0);
		cfg.params.bLength = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "bLength"), "value"),-0.001025);//0.0; //negative sign proposed by milan
		cfg.params.bHighway = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "bHighway"), "value"),0.00052);//0.0;
		cfg.params.bCost = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "bCost"), "value"),0.0);
		cfg.params.bSigInter = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "bSigInter"), "value"),-0.13);//0.0;
		cfg.params.bLeftTurns = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "bLeftTurns"), "value"),0.0);
		cfg.params.bWork = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "bWork"), "value"),0.0);
		cfg.params.bLeisure = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "bLeisure"), "value"),0.0);
		cfg.params.highwayBias = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "highwayBias"), "value"),0.5);
		cfg.params.minTravelTimeParam = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "minTravelTimeParam"), "value"),0.879);
		cfg.params.minDistanceParam = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "minDistanceParam"), "value"),0.325);
		cfg.params.minSignalParam = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "minSignalParam"), "value"),0.256);
		cfg.params.maxHighwayParam = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "maxHighwayParam"), "value"),0.422);
	}


//	//sanity check
	std::stringstream out("");
	if(cfg.networkDatabase.database == "")
	{
		out << "single path's data base, ";
	}
	if(cfg.networkDatabase.credentials == "")
	{
		out << "single path's data base credentials, ";
	}
	if(cfg.pathSetTableName == "")
	{
		out << "single path's table name, ";
	}
	if(cfg.RTTT_Conf == "")
	{
		out << "single path's realtime TT table name, ";
	}
	if(cfg.DTT_Conf == "")
	{
		out << "single path's default TT table name, ";
	}
	if(cfg.psRetrieval  == "")
	{
		out << "path set retieval stored procedure, ";
	}
	if(cfg.upsert  == "")
	{
		out << "travel time updation stored procedure, ";
	}
	if(out.str().size())
	{
		std::string err = std::string("Missing:") + out.str();
		throw std::runtime_error(err);
	}
}
