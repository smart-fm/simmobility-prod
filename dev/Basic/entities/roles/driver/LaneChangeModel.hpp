/* Copyright Singapore-MIT Alliance for Research and Technology */


#pragma once


#include "UpdateParams.hpp"


namespace sim_mob {


/*
 * LC_Model.cpp
 *
 *  Created on: 2011-8-15
 *      Author: mavswinwxy & Li Zhemin
 */


class LaneChangeModel {
public:
	///to execute the lane changing, meanwhile, check if crash will happen and avoid it
	///Return new lateral velocity, or <0 to keep the velocity at its previous value.
	virtual double executeLaneChanging(sim_mob::UpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir) = 0;
};

class MITSIM_LC_Model : public LaneChangeModel {
public:
	virtual double executeLaneChanging(sim_mob::UpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir);

	///Use Kazi LC Gap Model to calculate the critical gap
	///\param type 0=leading 1=lag + 2=mandatory (mask) //TODO: ARGHHHHHHH magic numbers....
	///\param dis from critical pos
	///\param spd spd of the follower
	///\param dv spd difference from the leader));
	virtual double lcCriticalGap(sim_mob::UpdateParams& p, int type,	double dis_, double spd_, double dv_);

	virtual sim_mob::LaneSide gapAcceptance(sim_mob::UpdateParams& p, int type);
	virtual double calcSideLaneUtility(sim_mob::UpdateParams& p, bool isLeft);  ///<return utility of adjacent gap
	virtual sim_mob::LANE_CHANGE_SIDE makeDiscretionaryLaneChangingDecision(sim_mob::UpdateParams& p);
	virtual double checkIfMandatory(UpdateParams& p);  ///<check if MLC is needed, return probability to MLC
	virtual sim_mob::LANE_CHANGE_SIDE makeMandatoryLaneChangingDecision(sim_mob::UpdateParams& p);
};


}
