/* Copyright Singapore-MIT Alliance for Research and Technology */


#include "Pavement.hpp"
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

// this was supposed to be a function to return the left most lane of a segment but since that can directly be accessed
// as in 'Point A' so it has no further functionality
int sim_mob::BusStop::bus_stop_lane(const RoadSegment& segment)
   {
        if (segment.width==0) {
       	throw std::runtime_error("Both the segment and all its lanes have a width of zero.");
       }
        else
        {
        	unsigned int laneID = segment.getLanes().back()->getLaneID(); // Point A
        	const std::vector<sim_mob::Point2D>& res = lane_location->getRoadSegment()->getLaneEdgePolyline(laneID);
        	return res.size();
        }
       }

// this is a funtion to determine the distance between two 2D points
float getDistance(sim_mob::Point2D a,sim_mob::Point2D b){};

int count = 0;
int x2, x1, y2, y1, x_base, y_base;

// This function is used to get the nearest polyline to the given location of Bus Stop passed as an argument to this function
sim_mob::Point2D sim_mob::BusStop:: getNearestPolyline(const sim_mob::Point2D &position)
{
	const std::vector<sim_mob::Point2D> poly = lane_location->getRoadSegment()->getLaneEdgePolyline(0);
	std ::vector<sim_mob::Point2D>:: const_iterator it= poly.begin();
	float distance_measured = getDistance(position,*it); // distance between the position of Bus Stop and the position at which iterator points initially
	for (it= poly.begin()+1; it!=poly.end(); it++) {
	        	// this would find the pair of polypoints which are closest to the current Bus Stop and also calculates the value of count as
		        // the number of polypoint pairs that need to be considered while calculating the sum of polylines
	        	if (distance_measured > getDistance(position,*it)) {
	               distance_measured = getDistance(position,*it);
	               sim_mob::Point2D first_PP = *it;
	               sim_mob::Point2D second_PP = *(it-1);
	               x2 = first_PP.getX();
	               y2 = first_PP.getY();
	               x1 = second_PP.getX();
	               y1 = second_PP.getY();
	               count++;
	            }
	      }

}


float SumofDistances = 0;
sim_mob::Point2D position;
// this function is used to calculate the sum of all polyline lengths *before* the current polyline
float sim_mob:: BusStop:: getSumDistance()
{

	getNearestPolyline(position);
	float m = (y2-y1)/(x2-x1);
	float n = -1/m;
	float Y = position.getY();
	float X = position.getX();
	x_base = (m*x1-n*X+Y-y1)/(m-n);
	y_base = (X-x1+m*Y-n*y1)/(m-n);

	std::vector<Point2D>::const_iterator it = lane_location->getRoadSegment()->getLaneEdgePolyline(0).begin()+1;
    sim_mob ::Point2D first_PP = *it;
	sim_mob::Point2D second_PP = *(it-1);
	for (int i = 0; i< count; i++) {
	        SumofDistances = SumofDistances + sqrt((first_PP.getX()-second_PP.getX())^2 + (first_PP.getY()-second_PP.getY())^2);
	        it++;
	}
	SumofDistances = SumofDistances + sqrt((x_base-first_PP.getX())^2 + (y_base-first_PP.getY())^2);

	return SumofDistances;

}


/*
 Set this Bus Stop as an obstacle at that distance (we use
distance, not percentages).
// I tried the following code but its giving some 'does not name a type error' and i am not getting how to remove it using forward declaration
//Pavement* rs;
//int location = getSumDistance();
//rs->obstacles[location] = new BusStop();
*/

}
