#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::link_t_pimpl::pre ()
{
	model = sim_mob::Link();
}

sim_mob::Link* sim_mob::xml::link_t_pimpl::post_link_t ()
{
	sim_mob::Link* res = new sim_mob::Link(model);

	//parentLink
	//{
	const std::vector<sim_mob::RoadSegment*>& fwd = res->getPath();
	for(std::vector<sim_mob::RoadSegment*>::const_iterator it = fwd.begin(); it != fwd.end(); it++) {
		const_cast<sim_mob::RoadSegment*>(*it)->setParentLink(res);
	}
	//}

	/*{
	const std::vector<sim_mob::RoadSegment*>& rev = res->getPath(false);
	for(std::vector<sim_mob::RoadSegment*>::const_iterator it = rev.begin(); it != rev.end(); it++) {
		const_cast<sim_mob::RoadSegment*>(*it)->setParentLink(res);
	}
	}*/

	return res;
}

void sim_mob::xml::link_t_pimpl::linkID (unsigned int value)
{
	model.linkID = value;

	//TODO: Bookkeeping
	//geo_Links_[link->linkID] = link;
}

void sim_mob::xml::link_t_pimpl::roadName (const ::std::string& value)
{
	model.roadName = value;
}

void sim_mob::xml::link_t_pimpl::StartingNode (unsigned int value)
{
	model.setStart(book.getNode(value));
}

void sim_mob::xml::link_t_pimpl::EndingNode (unsigned int value)
{
	model.setEnd(book.getNode(value));
}

void sim_mob::xml::link_t_pimpl::Segments (std::vector<sim_mob::RoadSegment*> value)
{
	//Copy over.
	model.setSegments(value);
}

