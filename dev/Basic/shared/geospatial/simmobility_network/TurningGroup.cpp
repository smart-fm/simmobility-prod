//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TurningGroup.hpp"

using namespace simmobility_network;

TurningGroup::TurningGroup(unsigned int id, unsigned int fromLink, unsigned int nodeId, std::string phases, Rules rules,
						   Tag *tag, unsigned int toLink) :
turningGroupId(id), fromLinkId(fromLink), nodeId(nodeId), phases(phases), rules(rules), tag(tag), toLinkId(toLink)
{
}

TurningGroup::TurningGroup(const TurningGroup& orig)
{
	this->turningGroupId = orig.turningGroupId;
	this->fromLinkId = orig.fromLinkId;
	this->nodeId = orig.nodeId;
	this->phases = orig.phases;
	this->rules = orig.rules;
	this->tag = orig.tag;
	this->toLinkId = orig.toLinkId;
}

TurningGroup::~TurningGroup()
{
	if(tag)
	{
		delete tag;
		tag = NULL;
	}
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

unsigned int TurningGroup::getNodeId() const
{
	return nodeId;
}

void TurningGroup::setNodeId(unsigned int nodeId)
{
	this->nodeId = nodeId;
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

Tag* TurningGroup::getTag() const
{
	return tag;
}

void TurningGroup::setTag(Tag* tag)
{
	this->tag = tag;
}

unsigned int TurningGroup::getToLinkId() const
{
	return toLinkId;
}

void TurningGroup::setToLinkId(unsigned int toLinkId)
{
	this->toLinkId = toLinkId;
}

