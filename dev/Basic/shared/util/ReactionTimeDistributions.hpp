/*
 * ReactionTimeDistributions.hpp
 *
 * \author Li Zhemin
 * \author Seth N. Hetu
 */

#pragma once

#include <boost/random.hpp>
#include <boost/math/distributions.hpp>
#include <boost/random/uniform_int_distribution.hpp>


namespace sim_mob {

///Simple base class for reaction time distributions
class ReactionTimeDist {
public:
	virtual double getReactionTime() = 0;
};


///Pre-specialized sub-classes for reaction time distributions
class UniformReactionTimeDist : public ReactionTimeDist {
private:
	boost::mt19937 gen;
	boost::random::uniform_int_distribution<> dist;

public:
	UniformReactionTimeDist(double min, double max)
	 : gen(), dist(min, max)
	{
	}

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
	{
	}

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
	{
	}

	virtual double getReactionTime() { return varGen(); }
};


}
