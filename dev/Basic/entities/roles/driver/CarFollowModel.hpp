/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

namespace sim_mob {

/*
 * CF_Model.hpp
 *
 *  Created on: 2011-8-15
 *      Author: wangxy & Li Zhemin
 */

//Abstract class which describes car following.
class CarFollowModel {
public:
	virtual double makeAcceleratingDecision(sim_mob::UpdateParams& p) = 0;
	virtual double breakToTargetSpeed(UpdateParams& p) = 0;
	virtual double accOfEmergencyDecelerating(UpdateParams& p) = 0;
	virtual double accOfCarFollowing(UpdateParams& p) = 0;
	virtual double accOfFreeFlowing(UpdateParams& p) = 0;
	virtual double accOfMixOfCFandFF(UpdateParams& p) = 0; 	//mix of car following and free flowing
};

//MITSIM version of car following model
class MITSIM_CF_Model : public CarFollowModel {
	virtual double makeAcceleratingDecision(sim_mob::UpdateParams& p);
	virtual double breakToTargetSpeed(UpdateParams& p);
	virtual double accOfEmergencyDecelerating(UpdateParams& p);
	virtual double accOfCarFollowing(UpdateParams& p);
	virtual double accOfFreeFlowing(UpdateParams& p);
	virtual double accOfMixOfCFandFF(UpdateParams& p); 	//mix of car following and free flowing
};


}
