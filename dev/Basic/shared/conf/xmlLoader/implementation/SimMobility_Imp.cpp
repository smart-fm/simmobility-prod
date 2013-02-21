#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

#include "conf/Config.hpp"

using std::string;
using std::map;

namespace {
//Helper: Hook up a prototype pointer.
template <class ModelType>
void hookUpPrototype(sim_mob::Mapping<ModelType> mapping, map<string, ModelType*> availableModels) {
	typename map<string, ModelType*>::iterator it = availableModels.find(mapping.name);
	if (it==availableModels.end()) {
		throw std::runtime_error("Model key not found.");
	}
	if (!it->second) {
		throw std::runtime_error("Model key found, but model is null.");
	}
	mapping.prototype = it->second;
}
} //End anon namespace.

void sim_mob::conf::SimMobility_pimpl::pre ()
{
}

void sim_mob::conf::SimMobility_pimpl::post_SimMobility ()
{
	//Initialize our default models.
	hookUpPrototype(config->system().defaultModels.intDriving, config->constructs().intDriveModels);
	config->system().defaultModels.intDriving.prototype = config->constructs().intDriveModels[config->system().defaultModels.intDriving.name];

	//Initialize our default workgroups.
	//TODO: Later; this will mess up our current WorkGroups if we do it now.
}

void sim_mob::conf::SimMobility_pimpl::single_threaded (bool single_threaded)
{
	config->singleThreaded() = single_threaded;
}

