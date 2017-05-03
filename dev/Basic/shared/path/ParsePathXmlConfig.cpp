#include "ParsePathXmlConfig.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <xercesc/dom/DOM.hpp>

#include "logging/Log.hpp"
#include "util/XmlParseHelper.hpp"

using namespace xercesc;
using namespace std;

sim_mob::ParsePathXmlConfig::ParsePathXmlConfig(const std::string& configFileName, PathSetConf& result) :
		cfg(result), ParseConfigXmlBase(configFileName)
{
	parseXmlAndProcess();
}

void sim_mob::ParsePathXmlConfig::processXmlFile(XercesDOMParser& parser)
{
	//Verify that the root node is "config"
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();

	if (TranscodeString(rootNode->getTagName()) != "pathset")
	{
		stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". Root node must be \'pathset\'";
		throw runtime_error(msg.str());
	}

	try
	{
		ProcessPathSetNode(rootNode);
	}
	catch(runtime_error &ex)
	{
		stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". " << ex.what();
		throw runtime_error(msg.str());
	}
}


void sim_mob::ParsePathXmlConfig::ProcessPathSetNode(xercesc::DOMElement* node)
{
	if(!(cfg.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false))) { return; }

	//Path-set generation thread-pool size
	xercesc::DOMElement* poolSize = GetSingleElementByName(node, "thread_pool");

	if(!poolSize)
	{
		Warn() << "\nWARNING! <thread_pool> node not defined in file " << inFilePath
		       << ". Expected: <thread_pool size=\"\">. Defaulting to 4\n";
		cfg.threadPoolSize = 4;
	}
	else
	{
		cfg.threadPoolSize = ParseInteger(GetNamedAttributeValue(poolSize, "size"), 4);
	}

	xercesc::DOMElement* pvtConfNode = GetSingleElementByName(node, "private_pathset");

    if((cfg.privatePathSetEnabled = ParseBoolean(GetNamedAttributeValue(pvtConfNode, "enabled"), false)))
    {
    	processPrivatePathsetNode(pvtConfNode);
    }

    xercesc::DOMElement* publicConfNode = GetSingleElementByName(node, "public_pathset");

    if((cfg.publicPathSetEnabled = ParseBoolean(GetNamedAttributeValue(publicConfNode, "enabled"), false)))
    {
    	processPublicPathsetNode(publicConfNode);
    }

}

void sim_mob::ParsePathXmlConfig::processPublicPathsetNode(xercesc::DOMElement* publicConfNode)
{
	cfg.publicPathSetMode = ParseString(GetNamedAttributeValue(publicConfNode, "mode", true), "");

	if (cfg.publicPathSetMode.empty() || !(cfg.publicPathSetMode == "normal" || cfg.publicPathSetMode == "generation"))
	{
		stringstream msg;
		msg << "Invalid value for <public_pathset mode=\""
		    << cfg.publicPathSetMode << "\">. Expected: \"normal\" or \"generation\"";
		throw runtime_error(msg.str());
	}

	if(cfg.publicPathSetMode == "generation" && cfg.privatePathSetMode == "generation")
	{
		stringstream msg;
		msg << "Invalid value for <public_pathset mode=\""
		    << cfg.publicPathSetMode << "\">. Private travel path-set and public transit path-set "
		    << "cannot run in \"generation\" mode at the same time. Expected: \"normal\"";
		throw runtime_error(msg.str());
	}

	if (cfg.publicPathSetMode == "generation")
	{
		xercesc::DOMElement* PT_odSource = GetSingleElementByName(publicConfNode, "od_source", true);
		cfg.publicPathSetOdSource = ParseString(GetNamedAttributeValue(PT_odSource, "table"), "");

		xercesc::DOMElement* bulk = GetSingleElementByName(publicConfNode, "bulk_generation_output_file", true);
		cfg.publicPathSetOutputFile = ParseString(GetNamedAttributeValue(bulk, "name"), "");
	}

	xercesc::DOMElement* publicPathSetAlgoConf = GetSingleElementByName(publicConfNode, "pathset_generation_algorithms");

	if (publicPathSetAlgoConf)
	{
		cfg.publickShortestPathLevel =
				ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
						publicPathSetAlgoConf, "k_shortest_path"), "level"), 10);

		cfg.simulationApproachIterations =
				ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
						publicPathSetAlgoConf, "simulation_approach"), "iterations"), 10);
	}
}

void sim_mob::ParsePathXmlConfig::processPrivatePathsetNode(xercesc::DOMElement* pvtConfNode)
{
	cfg.privatePathSetMode = ParseString(GetNamedAttributeValue(pvtConfNode, "mode", true), "");

	if (cfg.privatePathSetMode.empty() || !(cfg.privatePathSetMode == "normal" || cfg.privatePathSetMode == "generation"))
	{
		stringstream msg;
		msg << "Invalid value for <private_pathset mode=\""
		    << cfg.privatePathSetMode << "\">. Expected: \"normal\" or \"generation\"";
		throw runtime_error(msg.str());
	}

	//bulk path-set generation
	if (cfg.privatePathSetMode == "generation")
	{
		xercesc::DOMElement* odSource = GetSingleElementByName(pvtConfNode, "od_source", true);
		cfg.odSourceTableName = ParseString(GetNamedAttributeValue(odSource, "table"), "");

		xercesc::DOMElement* bulk = GetSingleElementByName(pvtConfNode, "bulk_generation_output_file", true);
		cfg.bulkFile = ParseString(GetNamedAttributeValue(bulk, "name"), "");
	}


	//supply_link_travel_time_file
	xercesc::DOMElement* linkFileNode = GetSingleElementByName(pvtConfNode, "supply_link_travel_time_file");
	cfg.supplyLinkFile = ParseString(GetNamedAttributeValue(linkFileNode, "name"), "");
	if(cfg.supplyLinkFile.empty())
	{
		stringstream msg;
		msg << "Empty value in <tables historical_traveltime=\"\"/>. Expected: supply_link_travel_time_file name";

		throw runtime_error(msg.str());
	}

	xercesc::DOMElement* tableNode = GetSingleElementByName(pvtConfNode, "tables", true);
	cfg.RTTT_Conf = ParseString(GetNamedAttributeValue(tableNode, "historical_traveltime"), "");

	if (cfg.RTTT_Conf.empty())
	{
		stringstream msg;
		msg << "Empty value in <tables historical_traveltime=\"\"/>. Expected: table name";
		throw runtime_error(msg.str());
	}

	cfg.DTT_Conf = ParseString(GetNamedAttributeValue(tableNode, "default_traveltime"), "");

	if (cfg.DTT_Conf.empty())
	{
		stringstream msg;
		msg << "Empty value in <tables default_traveltime=\"\"/>. Expected: table name";
		throw runtime_error(msg.str());
	}

	//function
	xercesc::DOMElement* functionNode = GetSingleElementByName(pvtConfNode, "functions", true);
	cfg.psRetrievalWithoutBannedRegion =
			ParseString(GetNamedAttributeValue(functionNode, "pathset_without_banned_area"), "");

	//recursive path-set generation
	xercesc::DOMElement* recPS = GetSingleElementByName(pvtConfNode, "recursive_pathset_generation");

	if (!recPS)
	{
		Warn() << "\nWARNING! <recursive_pathset_generation> node not defined in file " << inFilePath
		       << ". Expected: <recursive_pathset_generation value=\"\"/>. Defaulting to false\n";
		cfg.recPS = false;
	}
	else
	{
		cfg.recPS = ParseBoolean(GetNamedAttributeValue(recPS, "value"), false);
	}

	//reroute
	xercesc::DOMElement* reroute = GetSingleElementByName(pvtConfNode, "reroute");

	if (!reroute)
	{
		Warn() << "\nWARNING! <reroute> node not defined in file " << inFilePath
		       << ". Expected: <reroute enabled=\"\"/>. Defaulting to false\n";
		cfg.reroute = false;
	}
	else
	{
		cfg.reroute = ParseBoolean(GetNamedAttributeValue(reroute, "enabled"), false);
	}

	//path generators configuration
	xercesc::DOMElement* gen = GetSingleElementByName(pvtConfNode, "path_generators");

	if (gen)
	{
		cfg.maxSegSpeed = ParseFloat(GetNamedAttributeValue(gen, "max_segment_speed"));

		if (cfg.maxSegSpeed <= 0.0)
		{
			stringstream msg;
			msg << "Invalid value for <path_generators max_segment_speed=\"" << cfg.maxSegSpeed
			    << "\">. Expected: Value greater than 0";
			throw runtime_error(msg.str());
		}

		//random perturbation
		xercesc::DOMElement* random = GetSingleElementByName(gen, "random_perturbation");

		if (random)
		{
			cfg.perturbationIteration = ParseInteger(GetNamedAttributeValue(random, "iterations"), 0);
			std::vector<std::string> rangeStrVec;
			std::string rangerStr = ParseString(GetNamedAttributeValue(random, "uniform_range"), "0,0");
			boost::split(rangeStrVec, rangerStr, boost::is_any_of(","));
			cfg.perturbationRange.first = boost::lexical_cast<int>(rangeStrVec[0]);
			cfg.perturbationRange.second = boost::lexical_cast<int>(rangeStrVec[1]);
		}

		//K-shortest Path
		xercesc::DOMElement* ksp = GetSingleElementByName(gen, "k_shortest_path");

		if (ksp)
		{
			cfg.kspLevel = ParseInteger(GetNamedAttributeValue(ksp, "level"), 0);
		}

		//Link Elimination
		xercesc::DOMElement* LE = GetSingleElementByName(gen, "link_elimination");

		if (LE)
		{
			std::string types;
			types = ParseString(GetNamedAttributeValue(LE, "types"), "");
			boost::split(cfg.LE, types, boost::is_any_of(","));
		}
	}

	//utility parameters
	xercesc::DOMElement* utility = GetSingleElementByName(pvtConfNode, "utility_parameters");

	if (utility)
	{
		cfg.params.highwayBias =
				ParseFloat(GetNamedAttributeValue(GetSingleElementByName(utility, "highwayBias"), "value"), 0.5);
	}
}
