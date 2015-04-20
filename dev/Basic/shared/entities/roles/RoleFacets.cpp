//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoleFacets.hpp"

#include "entities/conflux/Conflux.hpp"
#include "entities/Person.hpp"
#include "workers/Worker.hpp"
#include "geospatial/LaneConnector.hpp"
unsigned int sim_mob::Facet::msgHandlerId = FACET_MSG_HDLR_ID;
sim_mob::NullableOutputStream sim_mob::Facet::Log()
{
	return NullableOutputStream(parent->currWorkerProvider->getLogFile());
}

sim_mob::Person* sim_mob::Facet::getParent()
{
	return parent;
}

void sim_mob::Facet::setParent(sim_mob::Person* parent)
{
	this->parent = parent;
}

void sim_mob::Facet::handleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{}

sim_mob::MovementFacet::MovementFacet(sim_mob::Person* parentAgent) : Facet(parentAgent) {}
sim_mob::MovementFacet::~MovementFacet() {}

bool sim_mob::MovementFacet::isConnectedToNextSeg(const Lane* lane, const sim_mob::RoadSegment *nxtRdSeg)
{
	if(!nxtRdSeg) { throw std::runtime_error("isConnectedToNextSeg() - destination road segment is null"); }
	if(!lane) { throw std::runtime_error("isConnectedToNextSeg() - null lane is passed"); }
	const sim_mob::RoadSegment* currSeg = lane->getRoadSegment();
	if (nxtRdSeg->getLink() != currSeg->getLink())
	{
		const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (currSeg->getEnd());
		if (currEndNode)
		{
			const std::set<sim_mob::LaneConnector*>& lcs = currEndNode->getOutgoingLanes(currSeg);
			for (std::set<sim_mob::LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++)
			{
				if ((*it)->getLaneTo()->getRoadSegment() == nxtRdSeg && (*it)->getLaneFrom() == lane) { return true; }
			}
		}
	}
	else
	{
		//if (lane->getRoadSegment()->getLink() == nxtRdSeg->getLink()) we are
		//crossing a uni-node. At uninodes, we assume all lanes of the current
		//segment are connected to all lanes of the next segment
		return true;
	}
	return false;
}

bool sim_mob::MovementFacet::isConnectedToNextSeg(const sim_mob::RoadSegment *srcRdSeg, const sim_mob::RoadSegment *nxtRdSeg)
{
	if(!nxtRdSeg || !srcRdSeg) { throw std::runtime_error("DriverMovement::getConnectionsToNextSeg() - one or both of the Road Segments are not available!"); }
	if (nxtRdSeg->getLink() != srcRdSeg->getLink())
	{
		const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (srcRdSeg->getEnd());
		if (currEndNode)
		{
			const std::set<sim_mob::LaneConnector*>& lcs = currEndNode->getOutgoingLanes(srcRdSeg);
			for (std::set<sim_mob::LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++)
			{
				if ((*it)->getLaneTo()->getRoadSegment() == nxtRdSeg) { return true; }
			}
		}
	}
	else
	{
		//if (lane->getRoadSegment()->getLink() == nxtRdSeg->getLink()) we are
		//crossing a uni-node. At uninodes, we assume all lanes of the current
		//segment are connected to all lanes of the next segment
		return true;
	}
	return false;
}

sim_mob::Conflux* sim_mob::MovementFacet::getStartingConflux() const { return nullptr; }
