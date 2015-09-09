//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DynamicVector.hpp"

using namespace simmobility_network;

DynamicVector::DynamicVector(double fromX, double fromY, double toX, double toY) : pos(fromX, fromY), mag(toX - fromX, toY - fromY)
{
	isZero = toX == fromX && toY == fromY; //In this case, there's not much we can do.
}

DynamicVector::DynamicVector(const Point& from, const Point& to)
: pos(from.getX(), from.getY()), mag(to.getX() - from.getX(), to.getY() - from.getY())
{
	isZero = to.getX() == from.getX() && to.getY() == from.getY(); //In this case, there's not much we can do.
}

DynamicVector::DynamicVector(const DynamicVector& copyFrom)
: pos(copyFrom.pos.getX(), copyFrom.pos.getY()), mag(copyFrom.mag.getX(), copyFrom.mag.getY()), isZero(copyFrom.isZero)
{
}

double DynamicVector::getX() const
{
	return pos.getX();
}

double DynamicVector::getY() const
{
	return pos.getY();
}

double DynamicVector::getEndX() const
{
	return pos.getX() + (isZero ? 0 : mag.getX());
}

double DynamicVector::getEndY() const
{
	return pos.getY() + (isZero ? 0 : mag.getY());
}

double DynamicVector::getMagnitude() const
{
	return isZero ? 0 : sqrt(mag.getX() * mag.getX() + mag.getY() * mag.getY());
}

double DynamicVector::getAngle() const
{
	if (mag.getX() == 0 && mag.getY() == 0)
	{
		//This only happens if the vector is specifically initialise with zero size.
		//throw std::runtime_error("Impossible to retrieve a vector's angle if it has never had a size.");
		return 0;
	}

	//Bound to 0...2*PI
	double calcAngle = atan2(mag.getY(), mag.getX());
	return (calcAngle < 0 ? calcAngle + 2 * M_PI : calcAngle);
}

DynamicVector& DynamicVector::translateVect(double dX, double dY)
{
	pos.setX(pos.getX() + dX);
	pos.setY(pos.getY() + dY);
	
	return *this;
}

DynamicVector& DynamicVector::translateVect()
{
	return translateVect((isZero ? 0 : mag.getX()), (isZero ? 0 : mag.getY()));
}

DynamicVector& DynamicVector::scaleVectTo(double val)
{
	//Will this vector have no size after this operation?
	isZero = (val == 0);

	//If non-zero, we can scale it.
	if (!isZero)
	{
		if (mag.getX() == 0 && mag.getY() == 0)
		{
			//This only happens if the vector is specifically initialise with zero size.
			//throw std::runtime_error("Impossible to scale a vector that has never had a size.");
			return *this;
		}

		//Note: The old way (converting to a unit vector then scaling) is very likely
		//      to introduce accuracy errors, since 1 "unit" is a very small number of centimetre.
		//      That is why this function factors in the unit vector in the same step.
		double factor = val / getMagnitude(); //Dividing first is usually slightly more accurate
		
		mag.setX(factor * mag.getX());
		mag.setY(factor * mag.getY());
	}
	return *this;
}

DynamicVector& DynamicVector::flipMirror()
{
	//Note: Mirroring a vector while isZero is true is fine; the change will be visible later once the vector scales to a non-zero size.
	mag.setX(-mag.getX());
	mag.setY(-mag.getY());

	return *this;
}

DynamicVector& DynamicVector::flipNormal(bool clockwise)
{
	//Note: Flipping a vector while isZero is true is fine; the change will be visible later once the vector scales to a non-zero size.
	int sign = clockwise ? 1 : -1;
	double newX = mag.getY() * sign;
	double newY = -mag.getX() * sign;

	mag.setX(newX);
	mag.setY(newY);

	return *this;
}

DynamicVector& DynamicVector::flipRight()
{
	return flipNormal(true);
}

DynamicVector& DynamicVector::flipLeft()
{
	return flipNormal(false);
}