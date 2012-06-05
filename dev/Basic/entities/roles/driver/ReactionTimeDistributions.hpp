/*
 * ReactionTimeDistributions.hpp
 *
 *  Created on: Jun 1, 2012
 *      Author: lzm
 */

#include <boost/random.hpp>
#include <boost/math/distributions.hpp>


typedef boost::minstd_rand GenType;
typedef boost::normal_distribution<> NormalDis;
typedef boost::variate_generator<GenType&, NormalDis> RNG;

namespace sim_mob {

class ReactionTimeDistributions {

private:
	static GenType gt;
	static NormalDis* dis;
	static RNG* rng_noraml;
	static ReactionTimeDistributions instance_;
	ReactionTimeDistributions()
	{
		dis = new NormalDis(1000, 200); // μ = 30, δ^2 = 4
		rng_noraml =  new RNG(gt, *dis);
	}

public:
	static ReactionTimeDistributions& instance()
	{
		return instance_;
	}


	virtual ~ReactionTimeDistributions();


	size_t static normal();


};

} /* namespace sim_mob */
