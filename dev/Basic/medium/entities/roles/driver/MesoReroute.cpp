#include "MesoReroute.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Link.hpp"
#include "entities/roles/driver/DriverFacets.hpp"
#include "path/PathSetManager.hpp"

/*
 * NOTE:
 * Current state of medium supply simulator requires us to decide & initiate rerouting mechanism
 * at list one segment before the rerouting point so that supply has the opportunity to choose the
 * correct lane.
 */
sim_mob::medium::MesoReroute::MesoReroute(sim_mob::medium::DriverMovement &dm):dm(dm)
{

}

bool sim_mob::medium::MesoReroute::shouldReroute()
{
	//	it is a good candidate if next segment is the last segment
	if(nextSegment->getEnd() != nextSegment->getLink()->getEnd())
	{
		//not the last segment
		return false;
	}
	return true;
}

void sim_mob::medium::MesoReroute::preReroute()
{
	currSegment = dm.pathMover.getCurrSegStats()->getRoadSegment();
	nextSegment = dm.pathMover.getNextSegStats(false)->getRoadSegment();
	dm.laneConnectorOverride = true;
}

void sim_mob::medium::MesoReroute::postReroute()
{
	dm.laneConnectorOverride = false;
}

bool sim_mob::medium::MesoReroute::doReroute()
{
	 // you need a subtrip(with some fields updated) for pathset manager to work;
	 sim_mob::SubTrip subTrip(*(dm.getParent()->currSubTrip));
	 subTrip.startTime;//todo
	 subTrip.fromLocation = sim_mob::WayPoint(nextSegment->getStart());
	 subTrip.toLocation = sim_mob::WayPoint((*(dm.pathMover.getPath().rbegin()))->getRoadSegment()->getEnd());
	 std::vector<WayPoint> newWP_Path = sim_mob::PathSetManager::getInstance()->getPath(subTrip, true, nextSegment);//new waypoint path
	 std::vector<const sim_mob::SegmentStats*> newSS_Path;
	//	first prepend part of the path remaiining to the rerouting point
	std::vector<const sim_mob::SegmentStats*>::iterator it = dm.pathMover.getCurrSegStatsIt();
	while((*it)->getRoadSegment() == currSegment || (*it)->getRoadSegment() == nextSegment)
	{
		newSS_Path.push_back((*it));
		it++;
	}
	//	now append the new path
	dm.initSegStatsPath(newWP_Path, newSS_Path);
	dm.pathMover.setPath(newSS_Path);

}

bool sim_mob::medium::MesoReroute::reroute()
{
	preReroute();
	if(!shouldReroute())
	{
	 return false;
	}

	doReroute();
	postReroute();
}
