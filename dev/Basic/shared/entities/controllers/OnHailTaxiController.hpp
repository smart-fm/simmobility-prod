/*
 * OnHailTaxiController.hpp
 *
 *  Created on: May 22, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef OnHailTaxiController_HPP_
#define OnHailTaxiController_HPP_

#include <vector>

#include "entities/Agent.hpp"
#include "OnCallController.hpp"

namespace sim_mob
{

class OnHailTaxiController : public MobilityServiceController
{
public:
	OnHailTaxiController(const MutexStrategy &mtxStrat, unsigned id, std::string tripSupportMode_, unsigned maxAggregatedRequests_,bool studyAreaEnabledController_,unsigned int toleratedExtraTime_,
                         unsigned int maxWaitingTime_,bool parkingEnabled) :
			MobilityServiceController(mtxStrat, MobilityServiceControllerType::SERVICE_CONTROLLER_ON_HAIL, id, tripSupportMode_,maxAggregatedRequests_,studyAreaEnabledController_,toleratedExtraTime_,maxWaitingTime_,parkingEnabled)
	{
	}

	~OnHailTaxiController()
	{};
};
}
#endif /* OnHailTaxiController_HPP_ */
