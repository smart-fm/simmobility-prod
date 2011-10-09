#pragma once


#include <vector>
#include <string>

#include "DynamicVector.hpp"


//Helper functions for geometric calculations

namespace sim_mob {
class Point2D;

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
double dist(const sim_mob::aimsun::Node* n1, const sim_mob::aimsun::Node* n2);
double dist(const sim_mob::Point2D* p1, const sim_mob::Point2D* p2);


//Check if an intersection point is actually on a line segment
bool lineContains(double ax, double ay, double bx, double by, double cx, double cy);
bool lineContains(const sim_mob::aimsun::Crossing* p1, const sim_mob::aimsun::Crossing* p2, double xPos, double yPos);
bool lineContains(const sim_mob::aimsun::Section* sec, double xPos, double yPos);


//Check if a point is left or right of a vector.
bool PointIsLeftOfVector(double ax, double ay, double bx, double by, double cx, double cy);
bool PointIsLeftOfVector(const sim_mob::aimsun::Node* vecStart, const sim_mob::aimsun::Node* vecEnd, const sim_mob::aimsun::Lane* point);
bool PointIsLeftOfVector(const sim_mob::DynamicVector& vec, const sim_mob::aimsun::Lane* point);


//Geometric line-line intersection formula.
//(x1,y1)->(x2, y2) and (x3,y3)->(x4,y4) are the two lines in question.
sim_mob::Point2D LineLineIntersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
sim_mob::Point2D LineLineIntersect(const sim_mob::aimsun::Crossing* const p1, const sim_mob::aimsun::Crossing* p2, const sim_mob::aimsun::Section* sec);
sim_mob::Point2D LineLineIntersect(const sim_mob::DynamicVector& v1, const sim_mob::DynamicVector& v2);
sim_mob::Point2D LineLineIntersect(const sim_mob::Point2D& p1, const sim_mob::Point2D& p2, const sim_mob::Point2D& p3, const sim_mob::Point2D& p4);




}
