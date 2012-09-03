/*
 * PedestrianPathMover.cpp
 *
 *  Created on: Aug 20, 2012
 *      Author: redheli
 */

#include "entities/roles/pedestrian/PedestrianPathMover.hpp"
#include <boost/random.hpp>

using boost::unordered_map;
using std::vector;
using std::map;
using namespace sim_mob;
boost::uniform_int<> zero_to_max(0, RAND_MAX);

typedef vector<WayPoint> PEDESTRIAN_PATH;
typedef PEDESTRIAN_PATH::iterator PEDESTRIAN_PATH_ITERATOR;

typedef vector<sim_mob::Point2D> POLYLINEPOINTS;
typedef POLYLINEPOINTS::const_iterator POLYLINEPOINTS_ITERATOR;
typedef POLYLINEPOINTS::const_reverse_iterator POLYLINEPOINTS_REVERSE_ITERATOR;

typedef unordered_map<sim_mob::Point2D,WayPoint* > POLYLINEPOINTS_WAYPOINT_MAP;

sim_mob::PedestrianPathMover::PedestrianPathMover():
		distAlongPolyline(0.0),isDoneWithEntirePath(false),nextWaypoint(NULL) {
	// TODO Auto-generated constructor stub

}

sim_mob::PedestrianPathMover::~PedestrianPathMover() {
	// TODO Auto-generated destructor stub
}

void sim_mob::PedestrianPathMover::setPath(const PEDESTRIAN_PATH path){
	pedestrian_path = path;
	pedestrian_path_iter = pedestrian_path.begin();

	// set polyline points array and map
	for(PEDESTRIAN_PATH_ITERATOR it = pedestrian_path.begin(); it != pedestrian_path.end(); ++it)
	{
		WayPoint *wp = &(*it);
		if (wp->type_ == WayPoint::SIDE_WALK)
		{
			const POLYLINEPOINTS *p = &(wp->lane_->getPolyline());
			if (wp->directionReverse)
			{
				//polylinePoints.insert(polylinePoints.end(),p->end(),p->begin());
				for(POLYLINEPOINTS_REVERSE_ITERATOR itt = p->rbegin(); itt != p->rend(); ++itt)
				{
					polylinePoints.push_back(*itt);
					std::cout<<"PedestrianPathMover::setPath: "<<(*itt).getX()<<" "<<(*itt).getY()<<std::endl;
					polylinePoint_wayPoint_map[*itt] = wp;
				}
			}
			else {
				for(POLYLINEPOINTS_ITERATOR itt = p->begin(); itt != p->end(); ++itt)
				{
					polylinePoints.push_back(*itt);
					std::cout<<"PedestrianPathMover::setPath: "<<(*itt).getX()<<" "<<(*itt).getY()<<std::endl;
					polylinePoint_wayPoint_map[*itt] = wp;
				}
			}

		}
		else if (wp->type_ == WayPoint::CROSSING)
		{
			POLYLINEPOINTS p = getCrossingPolylinePoints(wp);

			if(wp->directionReverse)
			{
				for(POLYLINEPOINTS_REVERSE_ITERATOR itt = p.rbegin(); itt != p.rend(); ++itt)
				{
					polylinePoints.push_back(*itt);
					std::cout<<"PedestrianPathMover::setPath: "<<(*itt).getX()<<" "<<(*itt).getY()<<std::endl;
					polylinePoint_wayPoint_map[*itt] = wp;
				}
			}
			else
				for(POLYLINEPOINTS_ITERATOR itt = p.begin(); itt != p.end(); ++itt)
				{
					polylinePoints.push_back(*itt);
					std::cout<<"PedestrianPathMover::setPath: "<<(*itt).getX()<<" "<<(*itt).getY()<<std::endl;
					polylinePoint_wayPoint_map[*itt] = wp;
				}
		}
	} //end of for

	//Sanity check.
	if (polylinePoints.empty()) {
		throw std::runtime_error("Cannot set pedestrian path: no path!");
	}

	//Save it.
	isDoneWithEntirePath = false;
	currPolylineStartpoint = polylinePoints.begin();
	currPolylineEndpoint = polylinePoints.begin() + 1;
	currentWaypoint = polylinePoint_wayPoint_map[*currPolylineStartpoint];
	nextWaypoint = polylinePoint_wayPoint_map[*currPolylineEndpoint];
}
POLYLINEPOINTS sim_mob::PedestrianPathMover::getCrossingPolylinePoints(WayPoint *wp)
{
	POLYLINEPOINTS points;

	double xRel, yRel;
	double xAbs, yAbs;
	double width, length, tmp;
	const Crossing *currCross = wp->crossing_;

	Point2D far1 = currCross->farLine.first;
	Point2D far2 = currCross->farLine.second;
	Point2D near1 = currCross->nearLine.first;
	Point2D near2 = currCross->nearLine.second;

	double cStartX = (double) near1.getX();
	double cStartY = (double) near1.getY();
	double cEndX = (double) near2.getX();
	double cEndY = (double) near2.getY();
	absToRel(cEndX, cEndY, length, tmp,cStartX,cStartY,cEndX,cEndY);
	absToRel((double) far1.getX(), (double) far1.getY(), tmp, width,cStartX,cStartY,cEndX,cEndY);

	boost::mt19937 gen;

	xRel = 0;
	if(width<0)
		yRel = -((double)(zero_to_max(gen)%(int(abs(width)/2+1)))+(double)(zero_to_max(gen)%(int(abs(width)/2+1))));
	else
		yRel = (double)(zero_to_max(gen)%(int(width/2+1)))+(double)(zero_to_max(gen)%(int(width/2+1)));
	xRel = (yRel*tmp)/width;
	relToAbs(xRel,yRel,xAbs,yAbs,cStartX,cStartY,cEndX,cEndY);
	//parent->xPos.set((int)xAbs);
	//parent->yPos.set((int)yAbs);
	points.push_back(Point2D((int)xAbs,(int)yAbs));

	xRel = xRel+length;
	relToAbs(xRel,yRel,xAbs,yAbs,cStartX,cStartY,cEndX,cEndY);
	//goalInLane = Point2D((int)xAbs,(int)yAbs);
	points.push_back(Point2D((int)xAbs,(int)yAbs));

	return points;
}
void sim_mob::PedestrianPathMover::relToAbs(double xRel, double yRel, double & xAbs, double & yAbs,
		double cStartX,double cStartY,double cEndX,double cEndY) {
	double xDir = cEndX - cStartX;
	double yDir = cEndY - cStartY;
	double magnitude = sqrt(xDir * xDir + yDir * yDir);
	double xDirection = xDir / magnitude;
	double yDirection = yDir / magnitude;
	xAbs = xRel * xDirection - yRel * yDirection + cStartX;
	yAbs = xRel * yDirection + yRel * xDirection + cStartY;
}
void sim_mob::PedestrianPathMover::absToRel(double xAbs, double yAbs, double & xRel, double & yRel,
		double cStartX,double cStartY,double cEndX,double cEndY) {
	double xDir = cEndX - cStartX;
	double yDir = cEndY - cStartY;
	double xOffset = xAbs - cStartX;
	double yOffset = yAbs - cStartY;
	double magnitude = sqrt(xDir * xDir + yDir * yDir);
	double xDirection = xDir / magnitude;
	double yDirection = yDir / magnitude;
	xRel = xOffset * xDirection + yOffset * yDirection;
	yRel = -xOffset * yDirection + yOffset * xDirection;
}
double sim_mob::PedestrianPathMover::currPolylineLength()
{
	DynamicVector temp(currPolylineStartpoint->getX(), currPolylineStartpoint->getY(),currPolylineEndpoint->getX(), currPolylineEndpoint->getY());
	return temp.getMagnitude();
}
double sim_mob::PedestrianPathMover::advance(double fwdDistance)
{
	//Taking precedence above everything else is the intersection model. If we are in an intersection,
	//  simply update the total distance and return (let the user deal with it). Also udpate the
	//  current polyline length to always be the same as the forward distance.
//	if (currentWaypoint->type_ == WayPoint::CROSSING)
//	{
//		throw std::runtime_error("Calling \"advance\" within an Intersection currently doesn't work right; use the Intersection model.");
//
//		distAlongPolyline += fwdDistance;
//		//currPolylineLength = distAlongPolyline;
//		return 0;
//	}

	//Move down the current polyline
	distAlongPolyline += fwdDistance;
	//distMovedInSegment += fwdDistance;

	// current polyline go through?
	// has next polyline?
	while (distAlongPolyline >= currPolylineLength())
	{
		// get exceed distance
		double exceed_distance = distAlongPolyline - currPolylineLength();

		if (advanceToNextPolylinePoint())
		{
			// has next waypoint , so reset distAlongPolyline
			distAlongPolyline = exceed_distance;
		}
		else
		{
			isDoneWithEntirePath = true;
			distAlongPolyline -= fwdDistance;
		}
	}

	return distAlongPolyline;
}

bool sim_mob::PedestrianPathMover::advanceToNextPolylinePoint()
{
	if (isDoneWithEntirePath)
		return false;

	bool hasNextPloylinePoint = false;
	//find next polyline point
	POLYLINEPOINTS_ITERATOR it = currPolylineEndpoint;
	it++;
	if (it != polylinePoints.end())
	{
		hasNextPloylinePoint = true;

		// reset polyline start ,end point
		currPolylineEndpoint++;
		currPolylineStartpoint++;
		currentWaypoint = polylinePoint_wayPoint_map[*currPolylineStartpoint];
		nextWaypoint = polylinePoint_wayPoint_map[*currPolylineEndpoint];
	}
	else {
		hasNextPloylinePoint = false;
		// we reach end of polyline point list
		//currPolylineEndpoint = currPolylineStartpoint+1;
	}

	return hasNextPloylinePoint;
}
bool sim_mob::PedestrianPathMover::isAtCrossing()
{
	bool isoc = false;
	if(currentWaypoint)
	{
		// maybe ped is on the polyline which connect corssing and segment ,which is gap whatever.
		// so need check current polyline 's start point's waypoint and end's.
		if (currentWaypoint->type_ == WayPoint::CROSSING && nextWaypoint->type_ == WayPoint::CROSSING )
			isoc = true;
	}

	return isoc;
}
DPoint sim_mob::PedestrianPathMover::getPosition() const
{
	//Else, scale a vector like normal
	DynamicVector movementVect(currPolylineStartpoint->getX(), currPolylineStartpoint->getY(), currPolylineEndpoint->getX(), currPolylineEndpoint->getY());
	movementVect.scaleVectTo(distAlongPolyline).translateVect();
	return DPoint(movementVect.getX(), movementVect.getY());
}
