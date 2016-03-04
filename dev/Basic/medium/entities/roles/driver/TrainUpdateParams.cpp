/*
 * TrainUpdateParams.cpp
 *
 *  Created on: Feb 18, 2016
 *      Author: zhang huai peng
 */

#include <entities/roles/driver/TrainUpdateParams.hpp>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
namespace sim_mob {

TrainUpdateParams::TrainUpdateParams():currentSpeed(0),secondsInTick(1.0),elapsedSeconds(0),disToNextPlatform(0),currentSpeedLimit(0),currentAcelerate(0) {
	// TODO Auto-generated constructor stub
	secondsInTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
}

TrainUpdateParams::~TrainUpdateParams() {
	// TODO Auto-generated destructor stub
}

void TrainUpdateParams::reset(timeslice now)
{
	UpdateParams::reset(now);
}
} /* namespace sim_mob */
