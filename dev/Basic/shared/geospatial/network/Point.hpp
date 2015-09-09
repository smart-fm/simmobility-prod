//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace simmobility_network
{
class Point
{
protected:
	/**X coordinate*/
	double x;

	/**Y coordinate*/
	double y;

	/**Z coordinate*/
	double z;

public:
	Point();
	Point(double x, double y, double z=0);
	virtual ~Point();

	double getX() const;
	void setX(double x);

	double getY() const;
	void setY(double y);

	double getZ() const;
	void setZ(double z);
};

class PolyPoint : public Point
{
private:
	/**Indicates the poly-line to which the point belongs*/
	unsigned int polyLineId;

	/**Indicates the position of the point in a line*/
	unsigned int sequenceNumber;

public:
	PolyPoint();
	PolyPoint(unsigned int id, unsigned int seqNum, double x, double y, double z);
	virtual ~PolyPoint();

	unsigned int getPolyLineId() const;
	void setPolyLineId(unsigned int polyLineId);

	unsigned int getSequenceNumber() const;
	void setSequenceNumber(unsigned int sequenceNumber);
};
}
