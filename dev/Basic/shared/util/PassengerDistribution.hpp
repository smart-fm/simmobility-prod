//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

 /* PassengerDistribution.hpp
  *
  * \author Meenu James
  */

#pragma once

#include <boost/random.hpp>
#include <boost/math/distributions.hpp>



namespace sim_mob {

///Simple base class for passenger time distributions
class PassengerDist {
public:
	virtual ~PassengerDist() {}
	virtual double getnopassengers() = 0;
};


///Pre-specialized sub-classes for passenger time distributions
class NormalPassengerDist : public PassengerDist {
private:
	boost::mt19937 gen;
	boost::normal_distribution<double> dist;
	boost::variate_generator<boost::mt19937, boost::normal_distribution<double> > varGen;

public:
	NormalPassengerDist(double mean, double stdev)
	 : gen(), dist(mean, stdev), varGen(gen, dist)
	{}
	virtual ~NormalPassengerDist() {}

	virtual double getnopassengers() { return varGen(); }
};

///Pre-specialized sub-classes for reaction time distributions
class LognormalPassengerDist : public PassengerDist {
private:
	boost::mt19937 gen;
	boost::lognormal_distribution<double> dist;
	boost::variate_generator<boost::mt19937, boost::lognormal_distribution<double> > varGen;

public:
	LognormalPassengerDist(double mean, double stdev)
	 : gen(), dist(mean, stdev), varGen(gen, dist)
	{}
	virtual ~LognormalPassengerDist() {}

	virtual double getnopassengers() { return varGen(); }
};
class UniformPassengerDist : public PassengerDist {
private:
	boost::mt19937 gen;
	boost::uniform_int<int> dist;
	boost::variate_generator<boost::mt19937, boost::uniform_int<int> > varGen;

public:
	UniformPassengerDist(int min, int max)
	 : gen(), dist(min ,max), varGen(gen, dist)
	{}
	virtual ~UniformPassengerDist() {}

	virtual double getnopassengers() { return varGen(); }
};


}
