#pragma once


#include <vector>
#include <string>


//Helper functions for geometric calculations

namespace sim_mob {

namespace aimsun {
//Forward declarations
class Node;
class Crossing;
class Lane;
class Section;
} //End aimsun namespace



///Simple distance formula. Various ways of calling it.
double dist(double x1, double y1, double x2, double y2);
double dist(const sim_mob::aimsun::Crossing* c1, const sim_mob::aimsun::Crossing* c2);
double dist(const sim_mob::aimsun::Lane* ln, const sim_mob::aimsun::Node* nd);
double dist(const sim_mob::aimsun::Lane* ln1, const sim_mob::aimsun::Lane* ln2);


//Check if an intersection point is actually on a line segment
bool lineContains(double ax, double ay, double bx, double by, double cx, double cy);
bool lineContains(const sim_mob::aimsun::Crossing* p1, const sim_mob::aimsun::Crossing* p2, double xPos, double yPos);
bool lineContains(const sim_mob::aimsun::Section* sec, double xPos, double yPos);




}
