/*
 * BoundarySegment.cpp
 *
 *  Created on: 03-Dec-2011
 *      Author: xuyan
 */

#include "BoundarySegment.hpp"

#include "geospatial/Lane.hpp"

#include "geospatial/RoadSegment.hpp"
#include "util/GeomHelpers.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;


//Use anonymous namespaces for private helper functions.
namespace {
void outputLine(Point2D& start_p, Point2D& end_p, std::string color)
{
	static int line_id = 100;
std::cout.operator <<(std::endl);
	PrintOut("(" << "\"CutLine\"," << "0," << line_id++ << "," << "{\"startPointX\":\"" << start_p.getX() << "\","
			<< "\"startPointY\":\"" << start_p.getY() << "\"," << "\"endPointX\":\"" << end_p.getX() << "\","
			<< "\"endPointY\":\"" << end_p.getY() << "\"," << "\"color\":\"" << color << "\"" << "})" <<"\n");
}
} //End anonymous namespace


void sim_mob::BoundarySegment::buildBoundaryBox(double boundary_length, double boundary_width)
{
	double down_offset = cutLineOffset + boundary_length / 100;
	double up_offset = cutLineOffset - boundary_length / 100;

	//change to use the start and end node in the polyline.
	int lane_count = boundarySegment->getLanes().size();
	int middle_lane = lane_count / 2;
	sim_mob::Lane* the_lane = boundarySegment->getLanes()[middle_lane];
	const std::vector<sim_mob::Point2D>& the_lines = the_lane->getPolyline();
	const Point2D& start_node = the_lines[0];
	const Point2D& end_node = the_lines[the_lines.size() - 1];

	Point2D down_point = sim_mob::getMiddlePoint2D(&start_node, &end_node, down_offset);
	Point2D up_point = sim_mob::getMiddlePoint2D(&start_node, &end_node, up_offset);

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

	Point2D middlePoint_1(middle_1_x, middle_1_y);
	Point2D middlePoint_2(middle_2_x, middle_2_y);

	int width_of_section = 3 * 100 * lane_count;
	double length_points = sim_mob::dist(middlePoint_1, middlePoint_2);
	double dis_short = length_points / 2 - width_of_section / 2;
	double dis_long = length_points / 2 + width_of_section / 2;

	int short_x = middle_1_x + (middle_2_x - middle_1_x) / length_points * dis_short;
	int short_y = middle_1_y + (middle_2_y- middle_1_y) / length_points * dis_short;
	int long_x = middle_1_x + (middle_2_x - middle_1_x) / length_points * dis_long;
	int long_y = middle_1_y + (middle_2_y- middle_1_y) / length_points * dis_long;

	cut_line_start = new Point2D(short_x, short_y);
	cut_line_to = new Point2D(long_x, long_y);
}


void sim_mob::BoundarySegment::output()
{
	if (bounary_box.size() < 3) {
		WarnOut("Error: Boundary box has < 3 points");
		return;
	}

	outputLine(*cut_line_start, *cut_line_to, "red");

	std::vector<Point2D>::iterator itr = bounary_box.begin();

	for (unsigned int i = 0; i < bounary_box.size() - 1; i++) {
		outputLine(*(itr + i), *(itr + i + 1), "blue");
	}

	outputLine(*(bounary_box.end() - 1), *(bounary_box.begin()), "blue");
}


