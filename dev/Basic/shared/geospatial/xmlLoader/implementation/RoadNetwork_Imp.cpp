//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::RoadNetwork_t_pimpl::pre ()
{
}


sim_mob::RoadNetwork& sim_mob::xml::RoadNetwork_t_pimpl::post_RoadNetwork_t ()
{
	throw_if_null();
	return *modelRef;
}


void sim_mob::xml::RoadNetwork_t_pimpl::coordinate_map (const std::vector<sim_mob::CoordinateTransform*>& value)
{
	throw_if_null();
	modelRef->coordinateMap = value;
}

void sim_mob::xml::RoadNetwork_t_pimpl::roadrunner_regions (const std::map<int, sim_mob::RoadRunnerRegion>& value)
{
	throw_if_null();
	modelRef->roadRunnerRegions = value;
}



void sim_mob::xml::RoadNetwork_t_pimpl::Nodes (const helper::NodesRes& value)
{
	throw_if_null();
	modelRef->segmentnodes = value.uniNodes;
	modelRef->nodes.insert(modelRef->nodes.end(), value.multiNodes.begin(), value.multiNodes.end());

	// set nodes type
	for(int i=0;i<modelRef->nodes.size();++i)
	{
		sim_mob::MultiNode* mn = modelRef->nodes[i];
		std::string idStr = boost::lexical_cast<std::string>(mn->getAimsunId());
		sim_mob::SimNodeType nt = (sim_mob::SimNodeType)modelRef->getNodeType(idStr);
		mn->type = nt;
	}

	//TODO: Why isn't this a set?
	//modelRef.nodes = value.second;
}

void sim_mob::xml::RoadNetwork_t_pimpl::Links (const std::vector<sim_mob::Link*>& value)
{
	throw_if_null();
	modelRef->setLinks(value);

	//set segments type
	for(int i=0;i<modelRef->links.size();++i)
	{
		sim_mob::Link* l=modelRef->links[i];
		const std::vector<sim_mob::RoadSegment*> segs = l->getSegments();
		for(int j=0;j<segs.size();++j)
		{
			sim_mob::RoadSegment* rs = segs[j];
			std::string idStr = boost::lexical_cast<std::string>(rs->getSegmentAimsunId());
			sim_mob::SimSegmentType st = (sim_mob::SimSegmentType)modelRef->getSegmentType(idStr);
			rs->type = st;
		}
	}
}

