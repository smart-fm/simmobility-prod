/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "RoadSegment.hpp"

#include "util/DynamicVector.hpp"
#include "util/GeomHelpers.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

#include "Lane.hpp"

using namespace sim_mob;

using std::pair;
using std::vector;


const unsigned long  &  sim_mob::RoadSegment::getSegmentID()const
{
	return segmentID;
}
void sim_mob::RoadSegment::setLanes(std::vector<sim_mob::Lane*> lanes)
{
	this->lanes = lanes;
}

sim_mob::RoadSegment::RoadSegment(Link* parent, unsigned long id) : Pavement(), parentLink(parent),segmentID(/*parent->getLinkId()*100 +*/ id)/*100 segments per link*/
{

}

void sim_mob::RoadSegment::setParentLink(Link* parent)
{
	this->parentLink = parent;
}

bool sim_mob::RoadSegment::isSingleDirectional()
{
	return lanesLeftOfDivider==0 || lanesLeftOfDivider==lanes.size()-1;
}


bool sim_mob::RoadSegment::isBiDirectional()
{
	return !isSingleDirectional();
}


pair<int, const Lane*> sim_mob::RoadSegment::translateRawLaneID(unsigned int rawID)
{
	//TODO: Need to convert the ID into an "effective" lane ID based on road direction
	//      (including bidirectional segments).
	throw std::runtime_error("Not yet defined.");
}



void sim_mob::RoadSegment::specifyEdgePolylines(const vector< vector<Point2D> >& calcdPolylines)
{
	//Save the edge polylines.
	laneEdgePolylines_cached = calcdPolylines;

	//TODO: Optionally reset this Segment's own polyline to laneEdge[0].
}


///This function forces a rebuild of all Lane and LaneEdge polylines.
///There are two ways to calculate the polyline. First, if the parent RoadSegment's "laneEdgePolylines_cached"
/// is non-empty, we can simply take the left lane line and continuously project it onto the right lane line
/// (then scale it back halfway). If these data points are not available, we have to compute it based on the
/// RoadSegment's polyline, which might be less accurate.
///We compute all points at once, since calling getLanePolyline() and then getLaneEdgePolyline() might
/// leave the system in a questionable state.
void sim_mob::RoadSegment::syncLanePolylines() /*const*/
{
	std::cout << "syncLanePolylines started rs width =  " << this->width << " \n";
	//Check our width (and all lane widths) are up-to-date:
	int totalWidth = 0;
    for (vector<Lane*>::const_iterator it=lanes.begin(); it!=lanes.end(); it++) {
    	if ((*it)->getWidth()==0) {
    		(*it)->width_ = 300; //TEMP: Hardcoded. TODO: Put in DB somewhere.
    	}
    	totalWidth += (*it)->getWidth();
    }
    for (vector<Lane*>::const_iterator it=lanes.begin(); it!=lanes.end(); it++) {
    	std::cout << "Lane " << (*it)->getLaneID() << " width: " << (*it)->getWidth_real() << std::endl;
    }
    std::cout <<  std::endl;

    if (width == 0) {
    	width = totalWidth;
    }
    std::cout << "Again rs width =  " << this->width << " \n";
	for (size_t i=0; i<lanes.size(); i++) {
		std::cout << "Again Lane " << lanes.at(i)->getLaneID() << " width: " << lanes.at(i)->getWidth_real() << std::endl;
	}
	//First, rebuild the Lane polylines; these will never be specified in advance.
	bool edgesExist = !laneEdgePolylines_cached.empty();
	for (size_t i=0; i<lanes.size(); i++) {
		if (edgesExist) {
			std::cout << "lane index " << i << "  (id: " << lanes.at(i)->getLaneID() << "): makeLanePolylineFromEdges, sending laneEdgePolylines_cached i, i+1" << " .....\n";
			makeLanePolylineFromEdges(lanes[i], laneEdgePolylines_cached[i], laneEdgePolylines_cached[i+1]);
		} else {
			std::cout << "lane index " << i << "  (id: " << lanes.at(i)->getLaneID() << "): makePolylineFromParentSegment";
			lanes[i]->makePolylineFromParentSegment();
		}
	}
	for (size_t i=0; i<lanes.size(); i++) {
		std::cout << "AAgain Lane " << lanes.at(i)->getLaneID() << " width: " << lanes.at(i)->getWidth_real() << std::endl;
	}
	//Next, if our edges array doesn't exist, re-generate it from the computed lanes.
	if (!edgesExist) {
		std::cout << " makeLaneEdgeFromPolyline  for each lane and push to laneEdgePolylines_cached\n";
		for (size_t i=0; i<=lanes.size(); i++) {
			bool edgeIsRight = i<lanes.size();
			laneEdgePolylines_cached.push_back(makeLaneEdgeFromPolyline(lanes[edgeIsRight?i:i-1], edgeIsRight));
		}
	}
	else
		std::cout << std::endl;

	//TEMP FIX
	//Now, add one more edge and one more lane representing the sidewalk.
	//TODO: This requires our function (and several others) to be declared non-const.
	//      Re-enable const correctness when we remove this code.
	//TEMP: For now, we just add the outer lane as a sidewalk. This won't quite work for bi-directional
	//      segments or for one-way Links. But it should be sufficient for the demo.
	Lane* swLane = new Lane(this, lanes.size());
	swLane->is_pedestrian_lane(true);
	swLane->width_ = lanes.back()->width_/2;
	swLane->polyline_ = sim_mob::ShiftPolyline(lanes.back()->polyline_, lanes.back()->getWidth()/2+swLane->getWidth()/2);
	//Add it, update
	lanes.push_back(swLane);
	std::cout << "swLane: " <<  swLane->getLaneID() <<  " width: :"<< swLane->width_ << std::endl;


	width += swLane->width_;

	vector<Point2D> res = makeLaneEdgeFromPolyline(lanes.back(), false);
	laneEdgePolylines_cached.push_back(res);//crash -vahid
	//Add an extra sidewalk on the other side if it's a road segment on a one-way link.
	sim_mob::Link* parentLink = getLink();

	if(parentLink)
	{
		//Check whether the link is one-way
		if(parentLink->getPath(false).empty() || parentLink->getPath(true).empty())
		{
			//Add a sidewalk on the other side of the road segment
			Lane* swLane2 = new Lane(this, lanes.size());
			swLane2->is_pedestrian_lane(true);
			std::cout << "swLane2......: "  << "lanes.front()->id=" << lanes.front()->getLaneID() << " width=" << lanes.front()->width_ << std::endl;

			swLane2->width_ = lanes.front()->width_/2;
			swLane2->polyline_ = sim_mob::ShiftPolyline(lanes.front()->polyline_, lanes.front()->getWidth()/2+swLane2->getWidth()/2, false);
			lanes.insert(lanes.begin(), swLane2);

			std::cout << "swLane2: " <<  swLane2->getLaneID() <<  " width: :"<< swLane2->width_ << "lanes.front()->id=" << lanes.front()->getLaneID() << "width=" << lanes.front()->width_ << std::endl;
			width += swLane2->width_;
			laneEdgePolylines_cached.insert(laneEdgePolylines_cached.begin(), makeLaneEdgeFromPolyline(lanes[0], true));
		}
	}
}

#ifndef SIMMOB_DISABLE_MPI


#endif

vector<Point2D> sim_mob::RoadSegment::makeLaneEdgeFromPolyline(Lane* refLane, bool edgeIsRight) const
{
	//Sanity check
	if (refLane->polyline_.size()<=1) {
		throw std::runtime_error("Can't manage with a Lane polyline of 0 or 1 points.");
	}
	else
	{
		std::cout << refLane->getLaneID() << " : refLane->polyline_.size() = " << refLane->polyline_.size() << std::endl;
	}
	if (refLane->width_==0) {
		throw std::runtime_error("Can't manage with a Segment/Lane with zero width.");
	}

	//Create a vector from start to end
	DynamicVector fullLine(refLane->polyline_.front().getX(), refLane->polyline_.front().getY(), refLane->polyline_.back().getX(), refLane->polyline_.back().getY());

	//Iterate over every point on the midline
	vector<Point2D> res;
	const Point2D* lastPt = nullptr;
	for (vector<Point2D>::const_iterator it=refLane->polyline_.begin(); it!=refLane->polyline_.end(); it++) {
		//Scale and translate the primary vector?
		if (lastPt) {
			double segDist = sim_mob::dist(*lastPt, *it);
			fullLine.scaleVectTo(segDist).translateVect();
		}

		//Make another vector, rotate right/left, scale half the width and add it to our result.
		DynamicVector currLine(fullLine);
		currLine.flipNormal(edgeIsRight).scaleVectTo(refLane->width_/2.0).translateVect();
		res.push_back(Point2D(currLine.getX(), currLine.getY()));

		//Save for the next round
		lastPt = &(*it);
	}

	//TEMP:
//	std::cout <<"Line: " <<edgeIsRight <<"\n";
//	std::cout <<"  Median:" <<refLane->polyline_.front().getX() <<"," <<refLane->polyline_.front().getY() <<" => " <<refLane->polyline_.back().getX() <<"," <<refLane->polyline_.back().getY()  <<"\n";
//	std::cout <<"  Edge:" <<res.front().getX() <<"," <<res.front().getY() <<" => " <<res.back().getX() <<"," <<res.back().getY()  <<"\n";
	return res;
}



void sim_mob::RoadSegment::makeLanePolylineFromEdges(Lane* lane, const vector<Point2D>& inner, const vector<Point2D>& outer) const
{
	std::cout << "inner.size() = " << inner.size() << "      outer.size()" << outer.size() << std::endl << std::endl;
	for(int i= 0; i < inner.size(); i++)
	{
		std::cout << "inner(" << inner.at(i).getX() << "," << inner.at(i).getY() << ")" << std::endl;
	}
	std::cout << "\n";
	for(int i= 0; i < outer.size(); i++)
	{
		std::cout << "outer(" << outer.at(i).getX() << "," << outer.at(i).getY() << ")" << std::endl;
	}
	std::cout << "\n";
	for (size_t i=0; i<lanes.size(); i++) {
		std::cout << "+Again Lane " << lanes.at(i)->getLaneID() << " width: " << lanes.at(i)->getWidth_real() << std::endl;
	}
	std::cout << "\n";
	//Sanity check
	if (outer.size()<=1 || inner.size()<=1) {
		throw std::runtime_error("Can't manage with a Lane Edge polyline of 0 or 1 points.");
	}

	//Get the offset of inner[0] to outer[0]
	double magX = outer.front().getX() - inner.front().getX(); std::cout << "outer.front().getX() - inner.front().getX() = " << magX << std::endl;
	double magY = outer.front().getY() - inner.front().getY(); std::cout << "outer.front().getY() - inner.front().getY() = " << magY << std::endl;
	double magTotal = sqrt(magX*magX + magY*magY);std::cout << "magTotal =" << magTotal << std::endl;

	for (size_t i=0; i<lanes.size(); i++) {
		std::cout << "1+Again Lane " << lanes.at(i)->getLaneID() << " width: " << lanes.at(i)->getWidth_real() << std::endl;
	}
	std::cout << "\n";
	//Update the lane's width
	lane->width_ = magTotal;

	for (size_t i=0; i<lanes.size(); i++) {
		std::cout << "2+Again Lane " << lanes.at(i)->getLaneID() << " width: " << lanes.at(i)->getWidth_real() << std::endl;
	}
	std::cout << "\n";
	//Travel along the inner path. Essentially, the inner and outer paths should line up, but if there's an extra point
	// or two, we don't want our algorithm to go crazy.
	lane->polyline_.clear();
	for (size_t i=0; i<lanes.size(); i++) {
		std::cout << "3+Again Lane " << lanes.at(i)->getLaneID() << " width: " << lanes.at(i)->getWidth_real() << std::endl;
	}
	std::cout << "\n";
	for (size_t i=0; i<lanes.size(); i++) {
		std::cout << "++Again Lane " << lanes.at(i)->getLaneID() << " width: " << lanes.at(i)->getWidth_real() << std::endl;
	}
	for (vector<Point2D>::const_iterator it=inner.begin(); it!=inner.end(); it++) {
		DynamicVector line(it->getX(), it->getY(), it->getX()+magX, it->getY()+magY);
		line.scaleVectTo(magTotal/2.0).translateVect();
		lane->polyline_.push_back(Point2D(line.getX(), line.getY()));
	}
	for (size_t i=0; i<lanes.size(); i++) {
		std::cout << "+++Again Lane " << lanes.at(i)->getLaneID() << " width: " << lanes.at(i)->getWidth_real() << std::endl;
	}
}


//TODO: Restore const-correctness after cleaning up sidewalks.
const vector<Point2D>& sim_mob::RoadSegment::getLaneEdgePolyline(unsigned int laneID) /*const*/
{
	std::cout<< "getLaneEdgePolyline for " << this->getSegmentID() << "(width:" << width << "),  started\n";
	//TEMP: Due to the way we manually insert sidewalks, this is needed for now.
	bool syncNeeded = false;
	for (size_t i=0; i<lanes.size(); i++) {
		if (lanes.at(i)->polyline_.empty()) {
			std::cout<< "getLaneEdgePolyline : lane index " << i << " , id: " << lanes.at(i)->getLaneID() << "(width:" << lanes.at(i)->getWidth_real() << ") laneEdgePolylines_cached.size()" << laneEdgePolylines_cached.size() << "  laneEdgePolylines_cached[" << i << "]=" << laneEdgePolylines_cached.at(i).size() << " has no polyline\n";
			syncNeeded = true;
//			break;
		}
	}

	//Rebuild if needed
	if (laneEdgePolylines_cached.empty() || syncNeeded) {
		std::cout << this->getSegmentID() << " : laneEdgePolylines_cached is " << (laneEdgePolylines_cached.empty() ? "empty" : "NOT empty") << " and syncNeeded is " << (syncNeeded ? "True" : "False") << std::endl;
		syncLanePolylines();
	}
	else
	{
		std::cout << this->getSegmentID() << " : getLaneEdgePolyline is not doing anythin\n";
	}
	return laneEdgePolylines_cached[laneID];
}


