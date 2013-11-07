//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Constructs.hpp"

#include <fstream>

#include <boost/filesystem.hpp>
#include <jsoncpp/json/json.h>

#include "logging/Log.hpp"
#include "password/password.hpp"
#include "util/Base64.hpp"

using namespace sim_mob;


sim_mob::Identifiable::Identifiable(const std::string& id) : id(id)
{}

std::string sim_mob::Identifiable::getId()
{
	return id;
}

sim_mob::StoredProcedureMap::StoredProcedureMap(const std::string& id) : Identifiable(id)
{}

sim_mob::Database::Database(const std::string& id) : Identifiable(id)
{}

sim_mob::Credential::Credential(const std::string& id) : Identifiable(id)
{}

sim_mob::ExternalScriptsMap::ExternalScriptsMap(const std::string& id, const std::string& scriptFilesPath, const std::string& scriptsLang) : Identifiable(id), path(scriptFilesPath), scriptLanguage(scriptsLang)
{}

std::string sim_mob::Credential::getUsername() const
{
	return username;
}

std::string sim_mob::Credential::getPassword(bool mask) const
{
	if (mask) {
		return std::string(password.size(), '*');
	} else {
		return password;
	}
}

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

	//The username is easy.
	username = root.get("username", "").asString();

	//The password requires loading the algorithm, which is something like this:
	//["clear","xor23","base64","cipher"]
	//It describes which mutations (xor23, base64) must be applied to get from the cleartext to the cipher, and vice-versa.
	std::vector<std::string> algorithm;
	const Json::Value steps = root["algorithm"];
	for (int i=0; i<steps.size(); i++) {
		std::string step = steps[i].asString();
		algorithm.push_back(step);

		//Double-check:
		if (step!="clear" && step!="xor23" && step!="base64" && step!="cipher") {
			throw std::runtime_error("Bad step in \"algorithm\" block.");
		}
	}

	//Minimum size.
	if (algorithm.size()<2) {
		throw std::runtime_error("File credentials requires an \"algorithm\" with at least 2 steps.");
	}

	//Reverse, if necessary.
	if (algorithm.front()=="clear" && algorithm.back()=="cipher") {
		std::reverse(algorithm.begin(), algorithm.end());
	}

	//Make sure our first 2 elements are clear/cipher.
	if (algorithm.front()!="cipher" || algorithm.back()!="clear") {
		throw std::runtime_error("File credentials \"algorithm\" requires a clear/cipher boundaries.");
	}

	//Now retrieve the password and run it.
	password = root.get("password", "").asString();
	if (!password.empty()) {
		for (std::vector<std::string>::const_iterator it=algorithm.begin(); it!=algorithm.end(); it++) {
			if ((*it)=="clear" || (*it)=="cipher") {
				continue; //TODO: This allows clear/cipher to appear in the middle, which is not allowed.
			} else if ((*it)=="xor23") {
				simple_password::decryptionFunc(password);
			} else if ((*it)=="base64") {
				password = Base64::decode(password);
			} else {
				throw std::runtime_error("Unexpected missing algorithm in file credentials.");
			}
		}
	}
}


void sim_mob::Credential::SetPlaintextCredentials(const std::string& username, const std::string& password)
{
	this->username = username;
	this->password = password;
}
