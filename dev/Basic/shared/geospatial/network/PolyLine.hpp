//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include "Point.hpp"

namespace sim_mob
{

/**
 * A poly-line is a simply a sequence of points. This class defines the structure of the poly-line
 * \author Neeraj D
 * \author Harish L
 */
class PolyLine
{
protected:

	/**Unique identifier for the Poly-line*/
	int polyLineId;

	/**Defines the length of the poly-line*/
	double length;

	/**Defines the points in the poly-line*/
	std::vector<PolyPoint> points;

public:
	PolyLine();

	virtual ~PolyLine();

	void setPolyLineId(int polyLineId);
	int getPolyLineId() const;

	void setLength(double length);
	double getLength() const;

	const std::vector<PolyPoint>& getPoints() const;

	const PolyPoint& getFirstPoint() const;
	const PolyPoint& getLastPoint() const;

	/**
	 * Extends the poly-line by adding a point to the end
	 * @param point - the poly-point to be added
	 */
	void addPoint(PolyPoint point);
};
}
