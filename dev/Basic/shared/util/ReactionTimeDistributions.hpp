//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ReactionTimeDistributions.hpp
 *
 * \author Li Zhemin
 * \author Seth N. Hetu
 */

#pragma once

#include <boost/random.hpp>
#include <boost/math/distributions.hpp>
#include <boost/version.hpp>

//Boost::Random is slightly messy re: uniform_int_distribution >1.47.0
//For now we just workaround via a macro and another define (which is undone after this file).
#if BOOST_VERSION >= 104700
#include <boost/random/uniform_int_distribution.hpp>
#define  boost_uniform_int  boost::random::uniform_int_distribution
#else
#include <boost/random/uniform_int.hpp>
#define  boost_uniform_int  boost::uniform_int
#endif

namespace sim_mob {

///Simple base class for reaction time distributions
class ReactionTimeDist {
public:
	virtual ~ReactionTimeDist() {}
	virtual double getReactionTime() = 0;
};


///Pre-specialized sub-classes for reaction time distributions
class UniformReactionTimeDist : public ReactionTimeDist {
private:
	boost::mt19937 gen;
	boost_uniform_int<> dist;

public:
	UniformReactionTimeDist(double min, double max)
	 : gen(), dist(min, max)
	{}
	virtual ~UniformReactionTimeDist() {}

	virtual double getReactionTime() { return dist(gen); }
};


///Pre-specialized sub-classes for reaction time distributions
class NormalReactionTimeDist : public ReactionTimeDist {
private:
	boost::mt19937 gen;
	boost::normal_distribution<double> dist;
	boost::variate_generator<boost::mt19937, boost::normal_distribution<double> > varGen;

public:
	NormalReactionTimeDist(double mean, double stdev)
	 : gen(), dist(mean, stdev), varGen(gen, dist)
	{}
	virtual ~NormalReactionTimeDist(){}

	virtual double getReactionTime() { return varGen(); }
};

///Pre-specialized sub-classes for reaction time distributions
class LognormalReactionTimeDist : public ReactionTimeDist {
private:
	boost::mt19937 gen;
	boost::lognormal_distribution<double> dist;
	boost::variate_generator<boost::mt19937, boost::lognormal_distribution<double> > varGen;

public:
	LognormalReactionTimeDist(double mean, double stdev)
	 : gen(), dist(mean, stdev), varGen(gen, dist)
	{}
	virtual ~LognormalReactionTimeDist(){}

	virtual double getReactionTime() { return varGen(); }
};


}

//Remove our macro; we don't need it after this.
#undef  boost_uniform_int

