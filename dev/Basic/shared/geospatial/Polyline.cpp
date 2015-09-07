//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include <geospatial/Polyline.hpp>

namespace sim_mob {

Polyline::Polyline()
:type(POLYLINE_TYPE_POLYPOINT),id(-1),length(0.0)
{	
}

Polyline::Polyline(const Polyline& src) :
	id(src.id), type(src.type), length(src.length), scenario(src.scenario)
{
}

Polyline::~Polyline() 
{
}

void Polyline::setScenario(std::string scenario)
{
	this->scenario = scenario;
}

std::string Polyline::getScenario() const
{
	return scenario;
}

void Polyline::setType(int type)
{
	this->type = type;
}

int Polyline::getType() const
{
	return type;
}

void Polyline::setLength(double length)
{
	this->length = length;
}

double Polyline::getLength() const
{
	return length;
}

void Polyline::setId(int id)
{
	this->id = id;
}

int Polyline::getId() const
{
	return id;
}

} /* namespace sim_mob */
