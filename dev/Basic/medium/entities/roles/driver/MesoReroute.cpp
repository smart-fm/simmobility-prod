#include "MesoReroute.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Link.hpp"
#include "entities/roles/driver/DriverFacets.hpp"
#include "path/PathSetManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

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
	//	the current segment is a good candidate if it is the last segment
	if(currSegment->getEnd() != currSegment->getLink()->getEnd())
	{
//		 {
//			 std::stringstream fileName("");
//			 fileName << "reroute-" << dm.getParent()->getId() << &dm;
//			 sim_mob::BasicLogger &logger = sim_mob::Logger::log(fileName.str());
//			 logger << "No rerouting from " << currSegment->getEnd()->getID() << "\n";
//		 }
		//not the last segment
		return false;
	}

	return true;
}

void sim_mob::medium::MesoReroute::preReroute()
{
	currSegment = dm.pathMover.getCurrSegStats()->getRoadSegment();
	dm.laneConnectorOverride = true;
}

void sim_mob::medium::MesoReroute::postReroute()
{
	dm.laneConnectorOverride = false;
}

bool sim_mob::medium::MesoReroute::doReroute()
{
	std::cout << "[" << this << "] Remaining Path: " ;
	const sim_mob::RoadSegment* curSeg = nullptr;
	for(sim_mob::medium::MesoPathMover::Path::const_iterator it = dm.pathMover.getCurrSegStatsIt(); it != dm.pathMover.getPath().end(); it++)
	{
		if(curSeg != (*it)->getRoadSegment())
		{
			curSeg = (*it)->getRoadSegment();
			std::cout << curSeg->getId() << ",";
		}
	}
	std::cout << std::endl;

	 //STEP-1 you need a subtrip(with some fields updated) for pathset manager to work;
	 sim_mob::SubTrip subTrip(*(dm.getParent()->currSubTrip));
	 subTrip.startTime = DailyTime(dm.getParentDriver()->getParams().now.ms()) + sim_mob::ConfigManager::GetInstance().FullConfig().simStartTime();
	 subTrip.fromLocation = sim_mob::WayPoint(currSegment->getEnd());
	 subTrip.toLocation = sim_mob::WayPoint((*(dm.pathMover.getPath().rbegin()))->getRoadSegment()->getEnd());

	 //STEP-2: find a new path
	 std::vector<WayPoint> newWP_Path = sim_mob::PathSetManager::getInstance()->getPath(subTrip, true, currSegment);//new waypoint path

	 //STEP-3: check for circles: if any segment in the new path had been traversed before, discard this new path
	 //(which means: based on current implementation, just forget about rerouting from this point)
	 //populate
	std::vector<const sim_mob::SegmentStats*>::const_iterator itSS = dm.pathMover.getCurrSegStatsIt();
	 for(sim_mob::medium::MesoPathMover::Path::const_iterator itPath = dm.pathMover.getPath().begin(); itPath != itSS; itPath++)
	 {
		 traversed.insert((*(itPath))->getRoadSegment());
	 }
	 traversed.insert((*(itSS))->getRoadSegment());
//	 //debug
//	 	 std::stringstream fileName("");
//	 	 fileName << "reroute-" << dm.getParent()->getId() << &dm;
//	 	 sim_mob::BasicLogger &logger = sim_mob::Logger::log(fileName.str());
//	 //debug ..
	 //now check:
	 for(std::vector<WayPoint>::iterator it = newWP_Path.begin(); it != newWP_Path.end(); it++)
	 {
		 if(traversed.find(it->roadSegment_) != traversed.end())
		 {
//			 //debug
//			 logger.prof("discard").addUp(1);
//			 //debug ..
			 return false;
		 }
	 }
//	 //debug
//	 logger << "Current Path: " << sim_mob::medium::MesoPathMover::printPath(dm.pathMover.getPath());
//	 logger << "Rerouting Point: " <<  currSegment->getEnd()->getID() << "\n";
//	 //debug...
	 std::vector<const sim_mob::SegmentStats*> newSS_Path;
	 //STEP-4:	prepend part of the path remaining to the rerouting point
	while((*itSS)->getRoadSegment() == currSegment)
	{
		newSS_Path.push_back((*itSS));
		itSS++;
	}
	//	STEP:5 now append the new path
	dm.initSegStatsPath(newWP_Path, newSS_Path);
	dm.pathMover.setPath(newSS_Path);
//	//debug
//	logger << "New Path: " <<  sim_mob::medium::MesoPathMover::printPath(newSS_Path);
//	std::cout << "[" << this << "] New Path: " <<  sim_mob::medium::MesoPathMover::printPath(newSS_Path);
//	//debug...

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
