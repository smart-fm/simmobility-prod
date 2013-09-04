//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RawConfigParams.hpp"

#include <fstream>

#include <boost/filesystem.hpp>
#include <jsoncpp/json/json.h>

#include "password/password.hpp"
#include "util/Base64.hpp"

using namespace sim_mob;

sim_mob::RawConfigParams::RawConfigParams()
{}

sim_mob::EntityTemplate::EntityTemplate() : startTimeMs(0)
{}

sim_mob::SystemParams::SystemParams() : singleThreaded(false), mergeLogFiles(false), networkSource(NETSRC_XML)
{}

sim_mob::WorkerParams::Worker::Worker() : count(0), granularityMs(0)
{}

sim_mob::SimulationParams::SimulationParams() :
	baseGranMS(0), totalRuntimeMS(0), totalWarmupMS(0), auraManagerImplementation(AuraManager::IMPL_RSTAR),
	workGroupAssigmentStrategy(WorkGroup::ASSIGN_ROUNDROBIN), partitioningSolutionId(0), startingAutoAgentID(0),
	mutexStategy(MtxStrat_Buffered), commSimEnabled(false), androidClientEnabled(false),
    /*reacTime_distributionType1(0), reacTime_distributionType2(0), reacTime_mean1(0), reacTime_mean2(0),
    reacTime_standardDev1(0), reacTime_standardDev2(0),*/ passenger_distribution_busstop(0),
    passenger_mean_busstop(0), passenger_standardDev_busstop(0), passenger_percent_boarding(0),
    passenger_percent_alighting(0), passenger_min_uniform_distribution(0), passenger_max_uniform_distribution(0)
{}


void sim_mob::Credential::LoadFileCredentials(const std::vector<std::string>& paths)
{
	//Find the first file that actually exists.
	for (std::vector<std::string>::const_iterator it=paths.begin(); it!=paths.end(); it++) {
		if (boost::filesystem::exists(*it)) {
			//Convert it to an absolute path.
			boost::filesystem::path abs_path = boost::filesystem::absolute(*it);
			LoadCredFile(abs_path.string());
			break; //We only allow the first match to actually load.
		}
	}
}


void sim_mob::Credential::LoadCredFile(const std::string& path)
{
	//Parse a JSON file.
	Json::Value root;
	Json::Reader reader;
	std::ifstream inFile(path.c_str(), std::ifstream::binary);
	if (!reader.parse(inFile, root, false)) {
		Warn() <<"Could not parse json credentials file: " <<path <<std::endl;
		Warn() <<reader.getFormatedErrorMessages() <<std::endl;
		return;
	}

	//TEMP:
	std::string temp = "X";
	temp = Base64::decode(temp);
	simple_password::decryptionFunc(temp);
	std::cout <<"\"" <<temp <<"\"\n";


	//todo
	throw 1;
}


void sim_mob::Credential::SetPlaintextCredentials(const std::string& username, const std::string& password)
{
	this->username = username;
	this->password = password;
}

