/* Copyright Singapore-MIT Alliance for Research and Technology */

/**
 * \file Config.hpp
 * Central location for configuration-loading code. Designed to replace simpleconf.cpp
 *
 * \note
 * Developers should make sure NOT to include "GenConfig.h" here; otherwise, this forces most of the source
 * tree to be rebuilt whenever a single parameter changes. (In other words, don't implement any functions in
 * this header file, except perhaps constructors.)
 *
 * \author Seth N. Hetu
 */

#pragma once

#include <boost/noncopyable.hpp>
#include <map>
#include <string>

//NOTE: Try to include only a minimum subset of files here, since Config.hpp is linked to from many places.
#include "Constructs.hpp"
#include "System.hpp"
#include "Simulation.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "buffering/Shared.hpp"


namespace sim_mob {


/**
 * Class containing CMake-defined parameters.
 */
class CMakeConfig : private boost::noncopyable {
public:
	//@{
	///Synced to the value of SIMMOB_DISABLE_MPI; used for runtime checks.
	bool MPI_Enabled() const;
	bool MPI_Disabled() const;
	//@}

	//@{
	///Synced to the value of SIMMOB_DISABLE_OUTPUT; used for runtime checks.
	bool OutputEnabled() const;
	bool OutputDisabled() const;
	//@}

	///Synced to the value of SIMMOB_STRICT_AGENT_ERRORS; used for runtime checks.
	bool StrictAgentErrors() const;

	///Synced to the value of SIMMOB_AGENT_UPDATE_PROFILE; used for runtime checks.
	bool GenerateAgentUpdateProfile() const;

	///Synced to the value of SIMMOB_AGENT_UPDATE_PROFILE; used for runtime checks.
	bool NewSignalModelEnabled() const;
};


/**
 * Class containing our configuration parameters. Intended to be used as a singleton.
 */
class Config : public CMakeConfig {
public:
	///Construct a new Config file.
	///
	///\note
	///Config is typically used as a Singleton class, so you should rarely have to
	///call its constructor.
	Config() : single_threaded(false), mtx_strat(MtxStrat_Buffered) {}

	///Helper struct: Which built-in models are available in each category
	struct BuiltInModels {
		std::map<std::string, sim_mob::CarFollowModel*> carFollowModels;
		std::map<std::string, sim_mob::LaneChangeModel*> laneChangeModels;
		std::map<std::string, sim_mob::IntersectionDrivingModel*> intDrivingModels;
	};

	///Informs the Config object which models are available of the "built-in" type.
	void InitBuiltInModels(const BuiltInModels& models);


	//@{
	///Accessor for the singleThreaded property.
	///If true, attempt to run all Workers on the same thread.
	bool& singleThreaded() { return single_threaded; }
	const bool& singleThreaded() const { return single_threaded; }
	///@}

	//@{
	///Accessor for the constructs array.
	///A construct is anything that can be created (dynamically) from the XML config file.
	sim_mob::Constructs& constructs() { return constructs_; }
	const sim_mob::Constructs& constructs() const { return constructs_; }
	///@

	//@{
	///Accessor for the system struct.
	sim_mob::System& system() { return system_; }
	const sim_mob::System& system() const { return system_; }
	///@

	//@{
	///Accessor for the simulation struct.
	sim_mob::Simulation& simulation() { return simulation_; }
	const sim_mob::Simulation& simulation() const { return simulation_; }
	///@

	//@{
	///Accessor for the road network.
	///TODO: Add back in the "sealed" property; check it in the non-const version of this function.
	sim_mob::RoadNetwork& network() { return network_; }
	const sim_mob::RoadNetwork& network() const { return network_; }
	///@

	//@{
	///Accessor for the build in models struct..
	///These models represent features of Sim Mobility which are tightly bound to the
	///infrastructure and cannot exist (for now) as plugins.
	sim_mob::Config::BuiltInModels& builtInModels() { return built_in_models; }
	const sim_mob::Config::BuiltInModels& builtInModels() const { return built_in_models; }
	///@

	//@{
	///Accessor for the mutex enforcement strategy
	///This strategy is used to handle our Shared<> variables, which can either be handled
	///  via locking or buffering.
	sim_mob::MutexStrategy& mutexStrategy() { return mtx_strat; }
	const sim_mob::MutexStrategy& mutexStrategy() const { return mtx_strat; }
	///@

	//@{
	///Accessor for the role factory.
	///This is used for creating roles based on string names. Later we can probably do this entirely with plugins.
	sim_mob::RoleFactory& roleFactory() { return role_fact_; }
	const sim_mob::RoleFactory& roleFactory() const { return role_fact_; }
	///@

private:
	//Data
	sim_mob::Constructs constructs_;
	bool single_threaded;
	sim_mob::System system_;
	sim_mob::Simulation simulation_;

	//Default built-in models
	BuiltInModels built_in_models;

	//Our Road Network.
	sim_mob::RoadNetwork network_;

	//Mutex enforcement strategy
	sim_mob::MutexStrategy mtx_strat;

	//Factory for creating Roles
	sim_mob::RoleFactory role_fact_;

public:
	///Retrieve an instance of the singleton Config object.
	static const Config& GetInstance();

	///Retrieve a mutable instance of the singleton Config object.
	///Don't use this function unless you know that it's ok to modify the Config object.
	static Config& GetInstanceRW();

private:
	static Config instance_;
};

}
