/*
 * BoundarySegment.cpp
 *
 *  Created on: 03-Dec-2011
 *      Author: xuyan
 */

#include "BoundarySegment.hpp"

#include "geospatial/RoadSegment.hpp"
#include "util/GeomHelpers.hpp"
#include "util/OutputUtil.hpp"

using namespace sim_mob;


//Use anonymous namespaces for private helper functions.
namespace {
void outputLine(Point2D& start_p, Point2D& end_p, std::string color)
{
	static int line_id = 100;

#ifndef SIMMOB_DISABLE_OUTPUT
	LogOut("(" << "\"CutLine\"," << "0," << line_id++ << "," << "{\"startPointX\":\"" << start_p.getX() << "\","
			<< "\"startPointY\":\"" << start_p.getY() << "\"," << "\"endPointX\":\"" << end_p.getX() << "\","
			<< "\"endPointY\":\"" << end_p.getY() << "\"," << "\"color\":\"" << color << "\"" << "})" << std::endl);
#endif
}
} //End anonymous namespace


void sim_mob::BoundarySegment::buildBoundaryBox(double boundary_length, double boundary_width)
{
	double down_offset = cutLineOffset + boundary_length / 100;
	double up_offset = cutLineOffset - boundary_length / 100;

	Point2D down_point = getMiddlePoint2D(&boundarySegment->getStart()->location,
			&boundarySegment->getEnd()->location, down_offset);
	Point2D up_point = getMiddlePoint2D(&boundarySegment->getStart()->location,
			&boundarySegment->getEnd()->location, up_offset);

	double length = dist(down_point, up_point);
	double ratio = boundary_width * 1.0 / length;

	int x_dis = (int) ((down_point.getY() - up_point.getY()) * ratio);
	int y_dis = (int) ((down_point.getX() - up_point.getX()) * ratio);

	Point2D firstPoint(down_point.getX() + x_dis, down_point.getY() - y_dis);
	Point2D secondPoint(down_point.getX() - x_dis, down_point.getY() + y_dis);
	Point2D thirdPoint(up_point.getX() - x_dis, up_point.getY() + y_dis);
	Point2D forthPoint(up_point.getX() + x_dis, up_point.getY() - y_dis);

	bounary_box.push_back(firstPoint);
	bounary_box.push_back(secondPoint);
	bounary_box.push_back(thirdPoint);
	bounary_box.push_back(forthPoint);

	//cut line

	int middle_1_x = (firstPoint.getX() + forthPoint.getX()) / 2;
	int middle_1_y = (firstPoint.getY() + forthPoint.getY()) / 2;
	int middle_2_x = (secondPoint.getX() + thirdPoint.getX()) / 2;
	int middle_2_y = (secondPoint.getY() + thirdPoint.getY()) / 2;

	Point2D* middlePoint_1 = new Point2D(middle_1_x, middle_1_y);
	Point2D* middlePoint_2 = new Point2D(middle_2_x, middle_2_y);

	cut_line_start = middlePoint_1;
	cut_line_to = middlePoint_2;
}


void sim_mob::BoundarySegment::output()
{
	if (bounary_box.size() < 3) {
#ifndef SIMMOB_DISABLE_OUTPUT
		LogOut("Error: Boundary box has < 3 points");
#endif
		return;
	}

	outputLine(*cut_line_start, *cut_line_to, "red");

	std::vector<Point2D>::iterator itr = bounary_box.begin();

	for (unsigned int i = 0; i < bounary_box.size() - 1; i++) {
		outputLine(*(itr + i), *(itr + i + 1), "blue");
	}

	outputLine(*(bounary_box.end() - 1), *(bounary_box.begin()), "blue");
}


