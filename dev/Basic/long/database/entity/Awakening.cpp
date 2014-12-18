/*
 * Awakening.cpp
 *
 *  Created on: 24 Nov, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */


#include "Awakening.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;


Awakening::Awakening(BigSerial id, float class1, float class2, float class3, float awaken_class1, float awaken_class2, float awaken_class3):
					 id(id), class1(class1), class2(class2), class3(class3), awaken_class1(awaken_class1), awaken_class2(awaken_class2), awaken_class3(awaken_class3){}

BigSerial Awakening::getId() const
{
	return id;
}

float Awakening::getClass1() const
{
	return class1;
}

float Awakening::getClass2() const
{
	return class2;
}

float Awakening::getClass3() const
{
	return class3;
}

void Awakening::setClass1(float one)
{
	class1 = one;
}

void Awakening::setClass2(float two)
{
	class2 = two;
}

void Awakening::setClass3(float three)
{
	class3 = three;
}

float Awakening::getAwakenClass1() const
{
	return awaken_class1;
}

float Awakening::getAwakenClass2() const
{
	return awaken_class2;
}

float Awakening::getAwakenClass3() const
{
	return awaken_class3;
}

void Awakening::setAwakenClass1(float one)
{
	awaken_class1 = one;
}

void Awakening::setAwakenClass2(float two)
{
	awaken_class2 = two;
}

void Awakening::setAwakenClass3(float three)
{
	awaken_class3 = three;
}


Awakening& Awakening::operator=(const Awakening& source)
{
	this->id = source.id;
	this->class1 = source.class1;
	this->class2 = source.class2;
	this->class3 = source.class3;
	this->awaken_class1 = source.awaken_class1;
	this->awaken_class2 = source.awaken_class2;
	this->awaken_class3 = source.awaken_class3;

	return *this;
}


namespace sim_mob
{
	namespace long_term
	{
		std::ostream& operator<<(std::ostream& strm, const Awakening& data)
		{
			return strm << "{" << "\"id \":\"" << data.id << "\","
						<< "\"class1 \":\"" 		<< data.class1 << "\","
						<< "\"class2 \":\"" 		<< data.class2 << "\","
						<< "\"class3 \":\"" 		<< data.class3 << "\","
						<< "\"awaken_class1 \":\"" 		<< data.awaken_class1 << "\","
						<< "\"awaken_class2 \":\"" 		<< data.awaken_class2 << "\","
						<< "\"awaken_class3 \":\"" 		<< data.awaken_class3 << "\"" << "}";
		}
	}
}




