#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::segment_t_pimpl::pre ()
{
	model = sim_mob::RoadSegment();
}

sim_mob::RoadSegment* sim_mob::xml::segment_t_pimpl::post_segment_t ()
{
	sim_mob::RoadSegment* res = new sim_mob::RoadSegment(model);

	//set parentsegment for each lane
	const std::vector<sim_mob::Lane*>& lanes = res->getLanes();
	for(std::vector<sim_mob::Lane*>::const_iterator it = lanes.begin(); it!=lanes.end(); it++) {
		//TODO: We need a better way of managing RoadNetwork const-ness.
		sim_mob::Lane* ln = const_cast<sim_mob::Lane*>(*it);
		if (ln) {
			ln->setParentSegment(res);
		}
	}

	//we set roadSegment* element of Crossing(and similar roadItems) in here
	//street directory has already done that, but that is not a good place to do this setting
	//for one reason, this XML reader can be used in GUI also, and there is no mechanism to set such elements there.
	for(std::map<sim_mob::centimeter_t,const RoadItem*>::iterator it = res->obstacles.begin(); it != res->obstacles.end(); it++) {
		RoadItem* temp = const_cast<RoadItem*>(it->second);
		if (temp) {
			//temp->setParentSegment(res);
		}
	}

	return res;
}

void sim_mob::xml::segment_t_pimpl::segmentID (unsigned long long value)
{
	model.setID(value);

	//TODO: Bookkeeping
	//geo_Segments_[this->rs->getSegmentID()] = rs;
}

void sim_mob::xml::segment_t_pimpl::startingNode (unsigned int value)
{
	model.setStart(book.getNode(value));
}

void sim_mob::xml::segment_t_pimpl::endingNode (unsigned int value)
{
	model.setEnd(book.getNode(value));
}

void sim_mob::xml::segment_t_pimpl::maxSpeed (short value)
{
	model.maxSpeed = value;
}

void sim_mob::xml::segment_t_pimpl::Length (unsigned int value)
{
	model.length = value;
}

void sim_mob::xml::segment_t_pimpl::Width (unsigned int value)
{
	model.width = value;
}

void sim_mob::xml::segment_t_pimpl::originalDB_ID (const ::std::string& value)
{
	model.originalDB_ID = value;
}

void sim_mob::xml::segment_t_pimpl::polyline (std::vector<sim_mob::Point2D> value)
{
	model.polyline = value;
}

void sim_mob::xml::segment_t_pimpl::laneEdgePolylines_cached (std::vector<std::vector<sim_mob::Point2D> > value)
{
	model.laneEdgePolylines_cached = value;
}

void sim_mob::xml::segment_t_pimpl::Lanes (std::vector<sim_mob::Lane*> value)
{
	model.setLanes(value);
	model.setCapacity(); //to be removed after xml is changed to provide capacity values
}

void sim_mob::xml::segment_t_pimpl::Obstacles (std::map<sim_mob::centimeter_t,const RoadItem*> value)
{
	model.obstacles = value;
}

void sim_mob::xml::segment_t_pimpl::KurbLine (std::vector<sim_mob::Point2D> value)
{
  // TODO
}


