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
	virtual double lcCriticalGap(sim_mob::UpdateParams& p, int type,	double dis_, double spd_, double dv_) = 0;
	virtual sim_mob::LaneSide gapAcceptance(sim_mob::UpdateParams& p, int type) = 0;
	virtual double calcSideLaneUtility(sim_mob::UpdateParams& p, bool isLeft) = 0;
	virtual sim_mob::LANE_CHANGE_SIDE makeDiscretionaryLaneChangingDecision(sim_mob::UpdateParams& p) = 0;
	virtual double checkIfMandatory(UpdateParams& p) = 0;
	virtual sim_mob::LANE_CHANGE_SIDE makeMandatoryLaneChangingDecision(sim_mob::UpdateParams& p) = 0;
	virtual void executeLaneChanging(sim_mob::UpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir) = 0;
};

class MITSIM_LC_Model : public LaneChangeModel {
	virtual double lcCriticalGap(sim_mob::UpdateParams& p, int type,	double dis_, double spd_, double dv_);
	virtual sim_mob::LaneSide gapAcceptance(sim_mob::UpdateParams& p, int type);
	virtual double calcSideLaneUtility(sim_mob::UpdateParams& p, bool isLeft);
	virtual sim_mob::LANE_CHANGE_SIDE makeDiscretionaryLaneChangingDecision(sim_mob::UpdateParams& p);
	virtual double checkIfMandatory(UpdateParams& p);
	virtual sim_mob::LANE_CHANGE_SIDE makeMandatoryLaneChangingDecision(sim_mob::UpdateParams& p);
	virtual void executeLaneChanging(sim_mob::UpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir);
};


}
