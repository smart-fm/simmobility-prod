/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Pavement.hpp"
//#include "Pavement.cpp"
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


//namespace sim_mob{

//namespace {
/*
  a) Find the nearest polyline (pair of polypoints) on that Lane's polyline.
*/

//Moved to header file
//const std::vector<sim_mob :: Lane*>& getLanes() const { return lanes;}

int sim_mob::BusStop::bus_stop_lane(const RoadSegment& segment)
   {
        if (segment.width==0) {
       	throw std::runtime_error("Both the segment and all its lanes have a width of zero.");
       }
        else
        {
        	unsigned int laneID = segment.getLanes().back()->getLaneID();


        	//Temporarily disabling:
        	//const std::vector<sim_mob :: Point2D>& getLaneEdgePolyline(laneID);
        	//return getLaneEdgePolyline(laneID);

        	const std::vector<sim_mob::Point2D>& res = lane_location->getRoadSegment()->getLaneEdgePolyline(laneID);
        	return res.size(); //Not sure what you're trying to return here.
        }
       }



sim_mob::Point2D sim_mob::BusStop::getNearestPolyline(const sim_mob::Point2D &position)
{
	int count = 0;
	int x2, x1, y2, y1;

	//Not sure what you're trying to do here. ~Seth
	const std::vector<Point2D> poly = lane_location->getRoadSegment()->getLaneEdgePolyline(0);
	for (std::vector<Point2D>::const_iterator it=poly.begin(); it!=poly.end(); it++) {
				//Temporarily disabled: Seth
	        	//float distance_measured = getDistance(position,it);
				//float other_distance = getDistance(position,it);
				float distance_measured = 0;
				float other_distance = 0;

	        	if (distance_measured > other_distance) {
	        		//Temporarily disabled: Seth
	               //distance_measured = getDistance(position,it);

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


float sim_mob::BusStop::getSumDistance()
{
	float SumofDistances = 0;

	//Added:
	sim_mob::Point2D position(0,0);

	//NOTE: I changed a lot of the following to get it to compile; please double-check your
	//      algorithm. ~Seth
	int x1 = position.getX();
	int y1 = position.getY();
	int x2 = position.getX();
	int y2 = position.getY();
	getNearestPolyline(position);
	float m = (y2-y1)/(x2-x1);
	float n = -1/m;
	float Y = position.getY();
	float X = position.getX();
	int x_base = (m*x1-n*X+Y-y1)/(m-n);
	int y_base = (X-x1+m*Y-n*y1)/(m-n);

	//NOTE: Disabled for now; this is kind of buggy. ~Seth
	/*sim_mob::vector<Point2D*>::const_iterator it=getLaneEdgePolyline(segment.getlanes.end()).begin()+1;
    sim_mob::Point2D first_PP = it;
	sim_mob::Point2D second_PP = it-1;
	for (int i = 0; i< count; i++) {
	        SumofDistances = SumofDistances + sqrt((first_PP.getX()-second_PP.getX())^2 + (first_PP.getY()-second_PP.getY())^2);
	        it++;
	}*/
	//SumofDistances = SumofDistances + sqrt((x_base-first_PP.getX())^2 + (y_base-first_PP.getY())^2);

	return SumofDistances;
}







/*
  d) Set this Bus Stop as an obstacle at that distance (we use
distance, not percentages).
*/


//RoadItem* item = /* "get next obstacle" */;

// Yet to implement. ~ Saurabh
//Lane* laneOfBusStop = dynamic_cast<BusStop*>(item)->lane_location;



//}
//}
