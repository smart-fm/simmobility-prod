//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TurningGroup.hpp"

using namespace simmobility_network;

TurningGroup::TurningGroup() :
turningGroupId(0), fromLinkId(0), phases(""), rules(TURNING_GROUP_RULE_NO_STOP_SIGN), toLinkId(0), visibility(0)
{
}

TurningGroup::TurningGroup(const TurningGroup& orig)
{
	this->turningGroupId = orig.turningGroupId;
	this->fromLinkId = orig.fromLinkId;
	this->phases = orig.phases;
	this->rules = orig.rules;
	this->tags = orig.tags;
	this->toLinkId = orig.toLinkId;
	this->visibility = orig.visibility;
}

TurningGroup::~TurningGroup()
{
	tags.clear();
}

unsigned int TurningGroup::getTurningGroupId() const
{
	return turningGroupId;
}

void TurningGroup::setTurningGroupId(unsigned int turningGroupId)
{
	this->turningGroupId = turningGroupId;
}

unsigned int TurningGroup::getFromLinkId() const
{
	return fromLinkId;
}

void TurningGroup::setFromLinkId(unsigned int fromLinkId)
{
	this->fromLinkId = fromLinkId;
}

std::string TurningGroup::getPhases() const
{
	return phases;
}

void TurningGroup::setPhases(std::string phases)
{
	this->phases = phases;
}

Rules TurningGroup::getRules() const
{
	return rules;
}

void TurningGroup::setRules(Rules rules)
{
	this->rules = rules;
}

const std::vector<Tag>& TurningGroup::getTags() const
{
	return tags;
}

void TurningGroup::setTag(std::vector<Tag>& tags)
{
	this->tags = tags;
}

unsigned int TurningGroup::getToLinkId() const
{
	return toLinkId;
}

void TurningGroup::setToLinkId(unsigned int toLinkId)
{
	this->toLinkId = toLinkId;
}

double TurningGroup::getVisibility() const
{
	return visibility;
}

void TurningGroup::setVisibility(double visibility)
{
	this->visibility = visibility;
}