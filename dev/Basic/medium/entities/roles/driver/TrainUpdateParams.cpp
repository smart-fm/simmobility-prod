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

TrainUpdateParams::TrainUpdateParams() {
	// TODO Auto-generated constructor stub

}

TrainUpdateParams::~TrainUpdateParams() {
	// TODO Auto-generated destructor stub
}

void TrainUpdateParams::reset(timeslice now)
{
	UpdateParams::reset(now);

	secondsInTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	elapsedSeconds = 0.0;
}
} /* namespace sim_mob */
