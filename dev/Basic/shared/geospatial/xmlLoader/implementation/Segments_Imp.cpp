#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Segments_pimpl::pre ()
{
	fwd.clear();
	rev.clear();
}

void sim_mob::xml::Segments_pimpl::FWDSegments (Segments_pimpl::SegmentList value)
{
	fwd = value;
}

void sim_mob::xml::Segments_pimpl::BKDSegments (Segments_pimpl::SegmentList value)
{
	rev = value;
}

std::pair<Segments_pimpl::SegmentList,Segments_pimpl::SegmentList> sim_mob::xml::Segments_pimpl::post_Segments ()
{
	//Register these for lookup later
	for (std::vector<sim_mob::RoadSegment*>::iterator it=fwd.begin(); it!=fwd.end(); it++) {
		book.addSegment(*it);
	}
	for (std::vector<sim_mob::RoadSegment*>::iterator it=rev.begin(); it!=rev.end(); it++) {
		book.addSegment(*it);
	}


	return std::make_pair(fwd,rev);
}
