
#pragma once

#include <string>
#include <map>
#include <stdexcept>

namespace sim_mob
{

/**
 * A general purpose class for prototype design patter.
 *
 * \author Vahid Saber
 */
template<class T>
class Factory {
public:
	///Register a Type, and a prototype we can clone to create members of this type.
	void Register(const std::string& name, const T* prototype)
	{
		if (prototypes.count(name)>0) {
			throw std::runtime_error("Duplicate role type.");
		}

		prototypes[name] = prototype;
	}
	///Is this a Type that our Factory knows how to construct?
	bool isKnown(const std::string& name) const
	{
		return getPrototype(name);
	}
	///Create a Type based on its name
	T* createType(const std::string& name) const
	{
		const T* prot = getPrototype(name);
		if (!prot) {
			throw std::runtime_error("Unknown role type; cannot clone.");
		}
		T* role = prot->clone();
		return role;
	}

	void clear() { prototypes.clear(); }

public:
	//Helper
	const T* getPrototype(const std::string& name) const
	{
		typename std::map<std::string, const T*>::const_iterator it = prototypes.find(name);
		if (it!=prototypes.end()) {
			//std::cout << name << " found in the prototypes\n";
			const T* t = it->second;
			return t;
		}
		return nullptr;
	}

private:
	//List of Types and prototypes
	std::map<std::string, const T*> prototypes;
};


}
