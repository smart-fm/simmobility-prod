//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>
#include <stdexcept>

#include "util/LangHelpers.hpp"

namespace sim_mob {


///Base class for any item that can be identified by a string ID.
class Identifiable {
public:
	Identifiable(const std::string& id);
	std::string getId();

private:
	std::string id;
};


///A mapping of stored procedures for a given datbase format.
///  Contains mappings such as "node" => "get_nodes()", which can be used to
///  retrieve Road Network items from a database in the given format.
class StoredProcedureMap : public Identifiable {
public:
	StoredProcedureMap(const std::string& id="");

	std::string dbFormat; //Usually "aimsun"
	std::map<std::string, std::string> procedureMappings; //key=>value
};


///Contains a database description (host, port, db_name). Does not include login credentials.
class Database : public Identifiable {
public:
	Database(const std::string& id="");

	std::string host;
	std::string port;
	std::string dbName;
};


///Contains login credentials (username+password) for a database. Masks password by default on retrieval.
class Credential : public Identifiable {
public:
	Credential(const std::string& id="");

	std::string getUsername() const;

	std::string getPassword(bool mask=true) const;

	///Load this set of credentials from a file with a given format (documented in, e.g., test_road_network.xml)
	void LoadFileCredentials(const std::vector<std::string>& paths);

	///Set the credentials manually. Note that this method should only be used for trivial passwords.
	void SetPlaintextCredentials(const std::string& username, const std::string& password);

private:
	//Helper: actually load the file.
	void LoadCredFile(const std::string& path);

	std::string username;
	std::string password;
};

class ExternalScriptsMap : public Identifiable {
public:
	ExternalScriptsMap(const std::string& id="", const std::string& scriptFilesPath="", const std::string& scriptsLang="");

	std::string path;
	std::string scriptLanguage;
	std::map<std::string, std::string> scriptFileName; //key=>value
};

class MongoCollectionsMap : public Identifiable {
public:
	MongoCollectionsMap(const std::string& id="", const std::string& dbName="");

	std::string dbName;
	std::map<std::string, std::string> collectionName; //key=>value
};

}
