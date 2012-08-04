/*
 * ReactionTimeDistributions.hpp
 * \author Li Zhemin
 *
 * TODO: These need to be done in a more generic, customizable way. ~Seth
 */

#include <boost/random.hpp>
#include <boost/math/distributions.hpp>


typedef boost::minstd_rand GenType;
typedef boost::normal_distribution<> NormalDis;
typedef boost::lognormal_distribution<> LognormalDis;
typedef boost::variate_generator<GenType&, NormalDis> RNG_Normal;
typedef boost::variate_generator<GenType&, LognormalDis> RNG_Lognormal;

namespace sim_mob {

class ReactionTimeDistributions {

private:
	static GenType gt1, gt2;
	static NormalDis* normal_dis1;
	static NormalDis* normal_dis2;
	static LognormalDis* lognormal_dis1;
	static LognormalDis* lognormal_dis2;
	static RNG_Normal* rng_normal1;
	static RNG_Normal* rng_normal2;
	static RNG_Lognormal* rng_lognormal1;
	static RNG_Lognormal* rng_lognormal2;
	static ReactionTimeDistributions instance_;

public:
	static size_t distributionType1;
	static size_t distributionType2;
	static size_t mean1;
	static size_t mean2;
	static size_t standardDev1;
	static size_t standardDev2;
	static ReactionTimeDistributions& instance()
	{
		return instance_;
	}

	virtual ~ReactionTimeDistributions();
	void setupDistribution1();
	void setupDistribution2();
	void setupNormalDisforRact1();
	void setupNormalDisforRact2();
	void setupLognormalDisforRact1();
	void setupLognormalDisforRact2();

	size_t static reactionTime1();
	size_t static reactionTime2();


};

} /* namespace sim_mob */
