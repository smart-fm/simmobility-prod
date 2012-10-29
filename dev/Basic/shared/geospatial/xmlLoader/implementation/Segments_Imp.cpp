#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


//TODO: Using a static field like this means we can't support multi-threaded loading of different networks.
//      It shouldn't be too hard to add some "temporary" object at a scope global to the parser classes. ~Seth
std::map<unsigned int,sim_mob::RoadSegment*> sim_mob::xml::Segments_pimpl::Lookup;

//Functionality for retrieving a Road Segment from the list of known Nodes.
sim_mob::RoadSegment* sim_mob::xml::Segments_pimpl::LookupNode(unsigned int id) {
	std::map<unsigned int,sim_mob::RoadSegment*>::iterator it = Lookup.find(id);
	if (it!=Lookup.end()) {
		return it->second;
	}
	throw std::runtime_error("Unknown Segment id.");
}

//Functionality for registering a RoadSegment so that it can be retrieved later.
void sim_mob::xml::Segments_pimpl::RegisterSegment(unsigned int id, sim_mob::RoadSegment* seg) {
	if (Lookup.count(id)>0) {
		throw std::runtime_error("Segment id is already registered.");
	}
	Lookup[id] = seg;
}


void sim_mob::xml::Segments_pimpl::pre ()
{
	fwd.clear();
	bck.clear();
	Lookup.clear();
}

void sim_mob::xml::Segments_pimpl::FWDSegments (Segments_pimpl::SegmentList value)
{
	fwd = value;
}

void sim_mob::xml::Segments_pimpl::BKDSegments (Segments_pimpl::SegmentList value)
{
	bck = value;
}

std::pair<Segments_pimpl::SegmentList,Segments_pimpl::SegmentList> sim_mob::xml::Segments_pimpl::post_Segments ()
{
	return std::make_pair(fwd,bck);
}
