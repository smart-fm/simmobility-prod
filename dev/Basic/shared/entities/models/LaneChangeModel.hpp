/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "geospatial/Lane.hpp"

namespace sim_mob {

//Forward declaration
class DriverUpdateParams;


/*
 * \file LaneChangeModel.hpp
 *
 * \author Wang Xinyuan
 * \author Li Zhemin
 * \author Seth N. Hetu
 */
class LaneChangeModel {
public:
	//Allow propagation of delete
	virtual ~LaneChangeModel() {}

	///to execute the lane changing, meanwhile, check if crash will happen and avoid it
	///Return new lateral velocity, or <0 to keep the velocity at its previous value.
	virtual double executeLaneChanging(sim_mob::DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir) = 0;
};


/**
 *
 * Simple version of the lane changing model
 * The purpose of this model is to demonstrate a very simple (yet reasonably accurate) model
 * which generates somewhat plausible visuals. This model should NOT be considered valid, but
 * it can be used for demonstrations and for learning how to write your own *Model subclasses.
 *
 * \author Seth N. Hetu
 */
class SimpleLaneChangeModel : public LaneChangeModel {
public:
	virtual double executeLaneChanging(sim_mob::DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir);
};

class MITSIM_LC_Model : public LaneChangeModel {
public:
	virtual double executeLaneChanging(sim_mob::DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir);

	///Use Kazi LC Gap Model to calculate the critical gap
	///\param type 0=leading 1=lag + 2=mandatory (mask) //TODO: ARGHHHHHHH magic numbers....
	///\param dis from critical pos
	///\param spd spd of the follower
	///\param dv spd difference from the leader));
	virtual double lcCriticalGap(sim_mob::DriverUpdateParams& p, int type,	double dis, double spd, double dv);
	int loadMLCKaziNosingModel(DriverUpdateParams &g);
	virtual sim_mob::LaneSide gapAcceptance(sim_mob::DriverUpdateParams& p, int type);
	virtual double calcSideLaneUtility(sim_mob::DriverUpdateParams& p, bool isLeft);  ///<return utility of adjacent gap
	virtual sim_mob::LANE_CHANGE_SIDE makeDiscretionaryLaneChangingDecision(sim_mob::DriverUpdateParams& p);  ///<DLC model, vehicles freely decide which lane to move. Returns 1 for Right, -1 for Left, and 0 for neither.
	virtual double checkIfMandatory(DriverUpdateParams& p);  ///<check if MLC is needed, return probability to MLC
	virtual sim_mob::LANE_CHANGE_SIDE makeMandatoryLaneChangingDecision(sim_mob::DriverUpdateParams& p); ///<MLC model, vehicles must change lane, Returns 1 for Right, -1 for Left.

	virtual sim_mob::LANE_CHANGE_SIDE executeNGSIMModel(sim_mob::DriverUpdateParams& p);
	virtual bool ifCourtesyMerging(DriverUpdateParams& p);
	virtual bool ifForcedMerging(DriverUpdateParams& p);
	virtual sim_mob::LANE_CHANGE_SIDE makeCourtesyMerging(sim_mob::DriverUpdateParams& p);
	virtual sim_mob::LANE_CHANGE_SIDE makeForcedMerging(sim_mob::DriverUpdateParams& p);
	virtual sim_mob::TARGET_GAP chooseTargetGap(sim_mob::DriverUpdateParams& p);

};


}
