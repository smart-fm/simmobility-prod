//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DynamicVector.hpp"

using namespace sim_mob;

sim_mob::DynamicVector::DynamicVector(double fromX, double fromY, double toX, double toY)
	: pos(fromX, fromY), mag(toX-fromX, toY-fromY)
{
	isZero = toX==fromX && toY==fromY; //In this case, there's not much we can do.
}

sim_mob::DynamicVector::DynamicVector(const Point2D& from, const Point2D& to)
	: pos(from.getX(), from.getY()), mag(to.getX()-from.getX(), to.getY()-from.getY())
{
	isZero = to.getX()==from.getX() && to.getY()==from.getY(); //In this case, there's not much we can do.
}

sim_mob::DynamicVector::DynamicVector(const DynamicVector& copyFrom)
	: pos(copyFrom.pos.x, copyFrom.pos.y), mag(copyFrom.mag.x, copyFrom.mag.y), isZero(copyFrom.isZero)
{
}

double sim_mob::DynamicVector::getX() const
{
	return pos.x;
}

double sim_mob::DynamicVector::getY() const
{
	return pos.y;
}

double sim_mob::DynamicVector::getEndX() const
{
	return pos.x + (isZero?0:mag.x);
}

double sim_mob::DynamicVector::getEndY() const
{
	return pos.y + (isZero?0:mag.y);
}

double sim_mob::DynamicVector::getMagnitude() const
{
	return isZero?0:sqrt(mag.x*mag.x + mag.y*mag.y);
}

double sim_mob::DynamicVector::getAngle() const
{
	if (mag.x == 0 && mag.y == 0) {
	//This only happens if the vector is specifically initialize with zero size.
		throw std::runtime_error("Impossible to retrieve a vector's angle if it has never had a size.");
	}

	//Bound to 0...2*PI
	double calcAngle = atan2(mag.y, mag.x);
	return (calcAngle < 0 ? calcAngle + 2 * M_PI : calcAngle);

}

DynamicVector& sim_mob::DynamicVector::translateVect(double dX, double dY)
{
	pos.x += dX; pos.y += dY; return *this;
}

DynamicVector& sim_mob::DynamicVector::translateVect()
{
	return translateVect((isZero?0:mag.x), (isZero?0:mag.y));
}


DynamicVector& sim_mob::DynamicVector::scaleVectTo(double val)
{
	//Will this vector have no size after this operation?
	isZero = (val==0);

	//If non-zero, we can scale it.
	if (!isZero) {
		if(mag.x==0 && mag.y==0) {
			//This only happens if the vector is specifically initialize with zero size.
			throw std::runtime_error("Impossible to scale a vector that has never had a size.");
		}

		//Note: The old way (converting to a unit vector then scaling) is very likely
		//      to introduce accuracy errors, since 1 "unit" is a very small number of centimeters.
		//      That is why this function factors in the unit vector in the same step.
		double factor = val/getMagnitude(); //Dividing first is usually slightly more accurate
		mag.x = factor*mag.x;
		mag.y = factor*mag.y;
	}
	return *this;
}

DynamicVector& sim_mob::DynamicVector::flipMirror()
{
	//Note: Mirroring a vector while isZero is true is fine; the change will be visible later once the vector scales to a non-zero size.
	mag.x=-mag.x;
	mag.y=-mag.y;
	return *this;
}

DynamicVector& sim_mob::DynamicVector::flipNormal(bool clockwise)
{
	//Note: Flipping a vector while isZero is true is fine; the change will be visible later once the vector scales to a non-zero size.
	int sign = clockwise ? 1 : -1;
	double newX = mag.y*sign;
	double newY = -mag.x*sign;
	mag.x = newX;
	mag.y = newY;
	return *this;
}

DynamicVector& sim_mob::DynamicVector::flipRight()
{
	return flipNormal(true);
}

DynamicVector& sim_mob::DynamicVector::flipLeft()
{
	return flipNormal(false);
}



