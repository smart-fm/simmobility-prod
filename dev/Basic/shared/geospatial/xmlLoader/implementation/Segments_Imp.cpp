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
	return std::make_pair(fwd,rev);
}
