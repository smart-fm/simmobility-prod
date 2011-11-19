/* Copyright Singapore-MIT Alliance for Research and Technology */


#pragma once

namespace sim_mob {


//Forward declaration
class UpdateParams;
class LaneSide;


/*
 * LC_Model.cpp
 *
 *  Created on: 2011-8-15
 *      Author: mavswinwxy & Li Zhemin
 */


class LaneChangeModel {
	virtual double lcCriticalGap(UpdateParams& p, int type,	double dis_, double spd_, double dv_) = 0;
	virtual sim_mob::LaneSide gapAcceptance(UpdateParams& p, int type) = 0;
	virtual double calcSideLaneUtility(UpdateParams& p, bool isLeft) = 0;
	virtual LANE_CHANGE_SIDE makeDiscretionaryLaneChangingDecision(UpdateParams& p) = 0;
	virtual double checkIfMandatory(double totalLinkDist) = 0;
	virtual LANE_CHANGE_SIDE makeMandatoryLaneChangingDecision(UpdateParams& p) = 0;
	virtual void excuteLaneChanging(UpdateParams& p, double totalLinkDistance) = 0;
};

class MITSIM_LC_Model : public LaneChangeModel {
	virtual double lcCriticalGap(UpdateParams& p, int type,	double dis_, double spd_, double dv_);
	virtual sim_mob::LaneSide gapAcceptance(UpdateParams& p, int type);
	virtual double calcSideLaneUtility(UpdateParams& p, bool isLeft);
	virtual LANE_CHANGE_SIDE makeDiscretionaryLaneChangingDecision(UpdateParams& p);
	virtual double checkIfMandatory(double totalLinkDist);
	virtual LANE_CHANGE_SIDE makeMandatoryLaneChangingDecision(UpdateParams& p);
	virtual void excuteLaneChanging(UpdateParams& p, double totalLinkDistance);
};


}
