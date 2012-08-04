/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
#include <map>

namespace sim_mob
{

/**
 * Class which handles the creation of Roles.
 *
 * \author Seth N. Hetu
 *
 * This class allows one to create Roles without the knowledge of where those Roles are
 * implemented. This is particularly useful when two roles have the same identity (short + medium) but
 * very different functionality.
 *
 * In addition, it does the following:
 *   \li Allows for future language-independence (e.g., a Role written in Python/Java)
 *   \li Allows the association of Roles with strings (needed by the config file).
 *   \li Allows full delayed loading of Roles, by saving the actual config string.
 */
class RoleFactory {
public:
	///Is this a Role that our Factory knows how to construct?
	bool isKnownRole(const std::string& roleName) const;

	///Return a map of required attributes, with a flag on each set to false.
	std::map<std::string, bool> getRequiredAttributes(const std::string& roleName) const;

private:


};


}
