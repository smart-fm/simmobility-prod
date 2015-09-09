//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

/**
 * \file GeomHelpers.hpp
 * Contains functions which are useful when calculating geometric information, such as distances or line intersections.
 *
 * \author Seth N. Hetu
 * \author Matthew Bremer Bruchon
 * \author Xu Yan
 * \author LIM Fung Chai
 */

#include <vector>
#include <string>
#include <utility>

#include "DynamicVector.hpp"

namespace sim_mob
{
  class Point;
  class RoadSegment;
  class Link;

  /**
   * Distance between two points, (x1,y1) and (x2,y2)
   *
   * Various alternative forms of this function exist, all of which reduce to this form.
   */
  double dist(Point pt1, Point pt2);

  //More distance functions; these use "get_distarg()" to dispatch down to the primary dist() function.
  double dist(double x1, double y1, double x2, double y2);

  template <class T> double dist(double x1, double y1, const T& point2);

  template <class T> double dist(const T& point1, double x2, double y2);

  template <class T, class Y> double dist(const T& point1, const Y& point2);

  ///Helper: Convert a Lane/Crossing/Point into a DPoint for use in the dist() function.
  ///See specializations at the bottom of this file.
  template <class T> Point get_distarg(const T& item);
  
  ///////////////////////////////////////////////////////////////////////////
  // Template specializations for get_distarg()
  ///////////////////////////////////////////////////////////////////////////
  template <> Point get_distarg(const Point& item);

  /**
   * Determine the normal intersection point between a line and a point outside that line.
   * Currently, this will throw an exception if that intersection point is outside the line.
   */
  Point normal_intersect(const Point& pt, const DynamicVector& line);

  /**
   * Check if an intersection point is actually on a line segment. The line from (ax,ay) to (bx,by) is
   * checked to see if (cx,cy) is a valid point.
   *
   * Various alternative forms of this function exist, all of which reduce to this form.
   */
  bool lineContains(double ax, double ay, double bx, double by, double cx, double cy);

  /**
   * Check if a point is left or right of a vector. The vector is defined with a starting point of (ax,ay)
   *   and an ending point of (bx,by). The point (cx,cy) is tested to see if it is left of the vector.
   *
   * \return true if the point is left of the vector.
   *
   * Various alternative forms of this function exist, all of which reduce to this form.
   */
  bool PointIsLeftOfVector(double ax, double ay, double bx, double by, double cx, double cy);
  bool PointIsLeftOfVector(const DynamicVector& vec, double x, double y);

  /**
   * Check if two lines intersect. The two lines in question are (x1,y1)->(x2, y2) and (x3,y3)->(x4,y4).
   *
   * \note
   * This function uses an arithmetic (NOT vector) computation. Hence, the size of the line segment is not taken into account; ANY two lines
   * will have an intersection unless they are parallel.
   *
   * \return the line intersection. If the lines are parallel, returns Point(MAX, MAX), where MAX is the maximum value of a double.
   *
   * Various alternative forms of this function exist, all of which reduce to this form.
   */
  Point LineLineIntersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
  Point LineLineIntersect(const DynamicVector& v1, const DynamicVector& v2);
  Point LineLineIntersect(const DynamicVector& v1, const Point& p3, const Point& p4);
  Point LineLineIntersect(const Point& p1, const Point& p2, const Point& p3, const Point& p4);

  /**
   * Shift a poly-line to the left or right by an amount.
   *
   * \param orig The original poly-line.
   * \param shiftAmt The distance in CM to shift the poly-line.
   * \param shiftLeft Whether to shift this poly-line left (true) or right (false). If a vector goes from (0,0) to (1,0), then the point (0,1) is defined as "left".
   *
   * \note
   * It is assumed that all shifted poly-lines are aligned on their start and end points, and all interior points
   * are found by approximating an angle of intersection.
   *
   * \note
   * This function currently does not operate as intended, due to some strangeness discovered with poly-lines.
   */
  std::vector<Point> ShiftPolyline(const std::vector<Point>& orig, double shiftAmt, bool shiftLeft=true);

  /**
   * Calculates the projection of a point onto a line defined by two other points.
   *
   * \param pToProject The point whose projection should be found.
   * \param pA One point defining the line to be projected upon.
   * \param shiftLeft The other point defining the line to be projected upon.
   *
   * \return The projection of the point onto the line
   *
   * \note
   * This is a projection onto a line, a line segment.  The returned point isn't necessarily between pA and pA.
   */
  Point ProjectOntoLine(const Point& pToProject, const Point& pA, const Point& pB);

  /**
   * Return the point that is perpendicular (with magnitude <magnitude>) to the vector that begins at <origin> and
   * passes through <direction>. This point is left of the vector if <magnitude> is positive.
   *
   * \param origin The start point of the vector.
   * \param direction A point the vector passes through.
   * \param magnitude Desired distance from the vector (left of the vector if positive)
   *
   * \return The calculated side point.
   */
  Point getSidePoint(const Point& origin, const Point& direction, double magnitude);

  //add by xuyan
  //Calculate the middle point, given the start/end point and the offset.
  //used to get the boundary box
  Point getMiddlePoint(const Point* start_point, const Point* end_point, double offset);

  //add by xuyan
  //Check whether one point is in the polygon, whose size is N
  //Note: the polygon is closed, which means the first node is the same with the last node
  bool PointInsidePolygon(const Point* polygon, int N, const Point p);

  /**
   * Calculates the projection of a point onto a line defined by two other points.
   *
   * \param pToProject The point whose projection should be found.
   * \param pA One point defining the line to be projected upon.
   * \param shiftLeft The other point defining the line to be projected upon.
   *
   * \return The projection of the point onto the line
   *
   * \note
   * This is a projection onto a line, a line segment.  The returned point isn't necessarily between pA and pA.
   */
  Point ProjectOntoLine(const Point& pToProject, const Point& pA, const Point& pB);

  /**
   * Return the point that is perpendicular (with magnitude <magnitude>) to the vector that begins at <origin> and
   * passes through <direction>. This point is left of the vector if <magnitude> is positive.
   *
   * \param origin The start point of the vector.
   * \param direction A point the vector passes through.
   * \param magnitude Desired distance from the vector (left of the vector if positive)
   *
   * \return The calculated side point.
   */
  Point getSidePoint(const Point& origin, const Point& direction, double magnitude);

  //TODO: This should eventually go into its own "Parser" class
  Point parse_point(const std::string& str);

  //Takes "xxxx,yyyy" or "(xxxx,yyyy)" and returns the x's and y's in a pair.
  //Ignores spaces (but not tabs/newlines)
  //TODO: This is stronger than parse_point, but we can't merge them yet (different return types).
  std::pair<uint32_t, uint32_t> parse_point_pair(const std::string& src);

}

