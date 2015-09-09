//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

namespace simmobility_network
{
class Tag
{
private:
	//The 'key' part of the key-value pair
	std::string key;

	//The 'value' part of the key-value pair
	std::string value;

public:
	Tag(std::string key, std::string value) : key(key), value(value) {}
	virtual ~Tag() {}

	void setKey(std::string key)
	{
		this->key = key;
	}

	std::string getKey() const
	{
		return key;
	}

	void setValue(std::string value)
	{
		this->value = value;
	}

	std::string getValue() const
	{
		return value;
	}
};
}
