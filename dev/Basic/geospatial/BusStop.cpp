/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Pavement.hpp"
#include "Pavement.cpp"
#include "BusStop.hpp"
#include "RoadSegment.hpp"
#include "Lane.hpp"
#include "util/GeomHelpers.hpp"
#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "util/GeomHelpers.hpp"
#endif

using namespace sim_mob;
using std::vector;
using std::set;
using std::string;


namespace sim_mob{

namespace {
/*
  a) Find the nearest polyline (pair of polypoints) on that Lane's polyline.
*/

const std::vector<sim_mob :: Lane*>& getLanes() const { return lanes;}

int bus_stop_lane(const RoadSegment& segment)
   {
        if (segment.width==0) {
       	throw std::runtime_error("Both the segment and all its lanes have a width of zero.");
       }
        else
        {
        	unsigned int laneID = segment.getLanes().end();
        	const std::vector<sim_mob :: Point2D>& getLaneEdgePolyline(laneID);
        	return getLaneEdgePolyline(laneID);
        }
       }



int count = 0;
int x2, x1, y2, y1, x_base, y_base;
sim_mob::Point2D getNearestPolyline(const sim_mob::Point2D &position)
{

	for (sim_mob::vector<Point2D*>::const_iterator it=bus_stop_lane(segment).begin(); it!=bus_stop_lane(segment).end(); it++) {
	        	float distance_measured = getDistance(position,it);
	        	if (distance_measured>getDistance(position,it)) {
	               distance_measured = getDistance(position,it);
	               sim_mob::Point2D first_PP = it;
	               sim_mob::Point2D second_PP = it-1;
	               x2 = first_PP.getX();
	               y2 = first_PP.getY();
	               x1 = second_PP.getX();
	               y1 = second_PP.getY();
	               count++;
	            }
	      }

}



/*
  b) Find the intersection point between the bus stop and that
polyline. Determine how far along that polypoint you are currently
located (e.g, 5m).
*/

/*
  c) Add the result from (b) to the sum of all polyline lengths
*before* the current polyline. (For example, if you are polyling 5 out
of 6, you have to add the lengths of polylines 1,2,3, and 4).
*/

float SumofDistances = 0;
float getSumDistance()
{

	getNearestPolyline(position);
	float m = (y2-y1)/(x2-x1);
	float n = -1/m;
	float Y = position.getY();
	float X = position.getX();
	x_base = (m*x1-n*X+Y-y1)/(m-n);
	y_base = (X-x1+m*Y-n*y1)/(m-n);

	sim_mob::vector<Point2D*>::const_iterator it=getLaneEdgePolyline(segment.getlanes.end()).begin()+1;
    sim_mob::Point2D first_PP = it;
	sim_mob::Point2D second_PP = it-1;
	for (int i = 0; i< count; i++) {
	        SumofDistances = SumofDistances + sqrt((first_PP.getX()-second_PP.getX())^2 + (first_PP.getY()-second_PP.getY())^2);
	        it++;
	}
	SumofDistances = SumofDistances + sqrt((x_base-first_PP.getX())^2 + (y_base-first_PP.getY())^2);

	return SumofDistances;
}







/*
  d) Set this Bus Stop as an obstacle at that distance (we use
distance, not percentages).
*/


//RoadItem* item = /* "get next obstacle" */;

// Yet to implement. ~ Saurabh
Lane* laneOfBusStop = dynamic_cast<BusStop*>(item)->lane_location;



}
}
