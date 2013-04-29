/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch
#include "Config.hpp"

//Config.cpp should include GenConfig.h, just to make sure we don't forget any files.
#include "GenConfig.h"

#include "entities/AuraManager.hpp"
#include "entities/Person.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "partitions/PartitionManager.hpp"
#include "workers/WorkGroup.hpp"
#include "geospatial/xmlLoader/implementation/geo10-driver.hpp"

#include "conf/simpleconf.hpp"

using std::string;
using std::map;

using namespace sim_mob;


Config sim_mob::Config::instance_;


namespace {
//Converts, e.g., "driver" into "Car".
//TODO: We should be able to unify our terminology here; why not just use "driver" overall?
std::string translate_mode(const std::string& src) {
	     if (src=="driver")       { return "Car"; }
	else if (src=="pedestrian")   { return "Walk"; }
	else if (src=="busdriver")    { return "Bus"; }
	else if (src=="passenger")    { return "travel"; }
	else                          { return "Unknown"; }
}
} //End unnamed namespace


sim_mob::Granularity::Granularity(int amount, const string& units) : base_gran_ms_(0)
{
	//Set the value in milliseconds.
	if (units=="hours") {
		ms_ = amount * 3600000;
	} else if (units=="minutes" || units=="min") {
		ms_ = amount * 60000;
	} else if (units=="seconds" || units=="s") {
		ms_ = amount * 1000;
	} else if (units=="ms") {
		ms_ = amount;
	} else {
		throw std::runtime_error("Unknown units for Granularity.");
	}
}


void sim_mob::Granularity::setBaseGranMS(int baseGranMS)
{
	//Set it.
	base_gran_ms_ = baseGranMS;

	//Truncate it if necessary.
	if ((base_gran_ms_>0) && (ms_%base_gran_ms_!=0)) {
		ms_ -= (ms_%base_gran_ms_);
	}
}


int sim_mob::Granularity::ticks() const
{
	if (base_gran_ms_==0) {
		throw std::runtime_error("Can't retrieve ticks() of a Granularity with no base.");
	}
	return ms_ / base_gran_ms_;
}


sim_mob::WorkGroup* WorkGroupFactory::getItem()
{
	if (!item) {
		//Sanity check.
		if (agentWG==signalWG) { throw std::runtime_error("agentWG and signalWG should be mutually exclusive."); }

		//Create it. For now, this is bound to the old "ConfigParams" structure; we can change this later once both files build in parallel.
		const ConfigParams& cf = ConfigParams::GetInstance();
		PartitionManager* partMgr = nullptr;
		if (!cf.MPI_Disabled() && cf.using_MPI) {
			partMgr = &PartitionManager::instance();
		}
		if (agentWG) {
			item = WorkGroup::NewWorkGroup(numWorkers, cf.totalRuntimeTicks, cf.granAgentsTicks, &AuraManager::instance(), partMgr);
		} else {
			item = WorkGroup::NewWorkGroup(numWorkers, cf.totalRuntimeTicks, cf.granSignalsTicks);
		}
	}
	return item;
}


void sim_mob::AgentLoader::addAgentSpec(const AgentSpec& ags)
{
	agents.push_back(ags);
}

void sim_mob::AgentLoader::loadAgents(std::list<sim_mob::Agent*>& res, LoadAgents::AgentConstraints& constraints, const sim_mob::Config& cfg)
{
	for (std::vector<AgentSpec>::iterator it=agents.begin(); it!=agents.end(); it++) {
		//Agents can specify manual or automatic IDs. We only need to check manual IDs, since automatic IDs are handled by the Agent class.
		if (it->id >= 0) {
			constraints.validateID(it->id);
		}

		//Check that we know how to create an Agent in this Role.
		bool knownRole = cfg.roleFactory().isKnownRole(it->agentType);
		if (!knownRole) {
			std::stringstream msg;
			msg <<"Unknown role type: \"" <<it->agentType <<"\"";
			throw std::runtime_error(msg.str().c_str());
		}

		//Set the "#mode" flag. The "#" ensures this can never be set in XML accidentally.
		it->properties["#mode"] = translate_mode(it->agentType);

		//WORKAROUND: The old code required an Agent's origin/dest to be in string format.
		//            We should *definitely* get rid of that requirement later; for now I'm
		//            adding this here. ~Seth
		std::stringstream msg;
		msg <<it->origin.getX() <<"," <<it->origin.getY();
		it->properties["originPos"] = msg.str();
		msg.str("");
		msg <<it->dest.getX() <<"," <<it->dest.getY();
		it->properties["destPos"] = msg.str();

		//Now create a shell Agent, assign it any remaining properties, and add it to the results.
		sim_mob::Person* ag = new sim_mob::Person("XML_Def", cfg.mutexStrategy(), it->id);
		ag->setConfigProperties(it->properties);
		ag->setStartTime(it->startTimeMs);
		res.push_back(ag);
	}
}


namespace {
void GenerateAgentsFromTripChains(std::list<sim_mob::Agent*>& res, LoadAgents::AgentConstraints& constraints, MutexStrategy mtxStrat, const std::map<std::string, std::vector<sim_mob::TripChainItem*> >& tripChains) {
	typedef std::map<std::string, std::vector<sim_mob::TripChainItem*> > TripChainList;

	//Create one person per trip-chain, as required.
	for (TripChainList::const_iterator it=tripChains.begin(); it!=tripChains.end(); it++) {
		//Create the person; the constructor *should* handle the rest.
		sim_mob::Person* ag = new sim_mob::Person("DB_TripChain", mtxStrat, it->second);
		res.push_back(ag);
	}
}
}  //End un-named namespace


void sim_mob::DatabaseAgentLoader::loadAgents(std::list<sim_mob::Agent*>& res, LoadAgents::AgentConstraints& constraints, const sim_mob::Config& cfg)
{
	//Find our stored procedure map (this should be guaranteed from our previous XML code).
	std::map<std::string, StoredProcedureMap>::const_iterator it = cfg.constructs().storedProcedureMaps.find(mappings);
	if (it==cfg.constructs().storedProcedureMaps.end()) { throw std::runtime_error("Unexpected; can't find mapping in loadAgents."); }

	//Ensure we're loading the right type.
	if (it->second.dbFormat!="aimsun") { throw std::runtime_error("DatabaseAgentLoader error: only the aimsun format supported (for now)."); }

	//Load using the same code found in our NetworkLoader.
	typedef std::map<std::string, std::vector<sim_mob::TripChainItem*> > TripChainList;
	TripChainList tripChains = sim_mob::aimsun::Loader::LoadTripChainsFromNetwork(connection, it->second.procedureMappings);

	//Now use the same code for Db and Xml trip-chains.
	GenerateAgentsFromTripChains(res, constraints, cfg.mutexStrategy(), tripChains);
}



void sim_mob::XmlAgentLoader::loadAgents(std::list<sim_mob::Agent*>& res, LoadAgents::AgentConstraints& constraints, const sim_mob::Config& cfg)
{
	//Create an output parameter.
	typedef std::map<std::string, std::vector<sim_mob::TripChainItem*> > TripChainList;
	TripChainList tripChains;

	//    		todo: this is commented to speed up compilations. enable before pushing the code[XMLCOMMENT]
	//Use code similar to our XML loading code to retrieve our TripChains.
	sim_mob::xml::InitAndLoadTripChainsFromXML(fileName, rootNode, tripChains);

	//Now use the same code for Db and Xml trip-chains.
	GenerateAgentsFromTripChains(res, constraints, cfg.mutexStrategy(), tripChains);
}




void sim_mob::Config::InitBuiltInModels(const BuiltInModels& models)
{
	built_in_models = models;
}


bool sim_mob::CMakeConfig::MPI_Enabled() const
{
	return !MPI_Disabled();
}
bool sim_mob::CMakeConfig::MPI_Disabled() const
{
#ifdef SIMMOB_DISABLE_MPI
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfig::OutputEnabled() const
{
	return !OutputDisabled();
}
bool sim_mob::CMakeConfig::OutputDisabled() const
{
#ifdef SIMMOB_DISABLE_OUTPUT
	return true;
#else
	return false;
#endif
}


bool sim_mob::CMakeConfig::StrictAgentErrors() const
{
#ifdef SIMMOB_STRICT_AGENT_ERRORS
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfig::ProfileOn() const
{
#ifdef SIMMOB_PROFILE_ON
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfig::ProfileAgentUpdates(bool accountForOnFlag) const
{
#ifdef SIMMOB_PROFILE_AGENT_UPDATES
	if (accountForOnFlag) {
		return ProfileOn();
	}
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfig::ProfileWorkerUpdates(bool accountForOnFlag) const
{
#ifdef SIMMOB_PROFILE_WORKER_UPDATES
	if (accountForOnFlag) {
		return ProfileOn();
	}
	return true;
#else
	return false;
#endif
}




