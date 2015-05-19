//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Tag.hpp"

using namespace simmobility_network;

Tag::Tag(std::string key, std::string value) :
key(key), value(value)
{
}

Tag::Tag(const Tag& orig)
{
	this->key = orig.key;
	this->value = orig.value;
}

Tag::~Tag()
{
}

void Tag::setKey(std::string key)
{
	this->key = key;
}

std::string Tag::getKey() const
{
	return key;
}

void Tag::setValue(std::string value)
{
	this->value = value;
}

std::string Tag::getValue() const
{
	return value;
}