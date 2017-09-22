/*
 * Awakening.cpp
 *
 *  Created on: 24 Nov, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */


#include "Awakening.hpp"

#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace sim_mob;
using namespace sim_mob::long_term;


Awakening::Awakening(BigSerial id, float class1, float class2, float class3, float awakenClass1, float awakenClass2, float awakenClass3):
					 id(id), class1(class1), class2(class2), class3(class3), awakenClass1(awakenClass1), awakenClass2(awakenClass2), awakenClass3(awakenClass3){}

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
	return awakenClass1;
}

float Awakening::getAwakenClass2() const
{
	return awakenClass2;
}

float Awakening::getAwakenClass3() const
{
	return awakenClass3;
}

void Awakening::setAwakenClass1(float one)
{
	awakenClass1 = one;
}

void Awakening::setAwakenClass2(float two)
{
	awakenClass2 = two;
}

void Awakening::setAwakenClass3(float three)
{
	awakenClass3 = three;
}


Awakening& Awakening::operator=(const Awakening& source)
{
	this->id = source.id;
	this->class1 = source.class1;
	this->class2 = source.class2;
	this->class3 = source.class3;
	this->awakenClass1 = source.awakenClass1;
	this->awakenClass2 = source.awakenClass2;
	this->awakenClass3 = source.awakenClass3;

	return *this;
}

template<class Archive>
void  Awakening::serialize(Archive & ar,const unsigned int version)
{

	ar & id;
	ar & class1;
	ar & class2 ;
	ar & class3;
	ar & awakenClass1;
	ar & awakenClass2;
	ar & awakenClass3;
}

void  Awakening::saveData(std::vector<Awakening*> &awakenings)
{
	// make an archive
	std::ofstream ofs(filename);
	boost::archive::binary_oarchive oa(ofs);
	oa & awakenings;

}

std::vector<Awakening*>  Awakening::loadSerializedData()
{
	std::vector<Awakening*> awakenings;
	// Restore from saved data and print to verify contents
	std::vector<Awakening*> restored_info;
	{
		// Create and input archive
		std::ifstream ifs( filename );
		boost::archive::binary_iarchive ar( ifs );

		// Load the data
		ar & restored_info;
	}

	std::vector<Awakening*>::const_iterator it = restored_info.begin();
	for (; it != restored_info.end(); ++it)
	{
		Awakening *aw = *it;
		awakenings.push_back(aw);
	}

	return awakenings;


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
						<< "\"awakenClass1 \":\"" 		<< data.awakenClass1 << "\","
						<< "\"awakenClass2 \":\"" 		<< data.awakenClass2 << "\","
						<< "\"awakenClass3 \":\"" 		<< data.awakenClass3 << "\"" << "}";
		}
	}
}




