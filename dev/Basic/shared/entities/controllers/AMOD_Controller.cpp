//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AMOD_Controller.hpp"

using namespace sim_mob;

AMOD_Controller::AMOD_Controller(const MutexStrategy &mtx, unsigned int computationPeriod, unsigned int id,
                                 TT_EstimateType tt_estType) : OnCallController(mtx, computationPeriod,
                                                                                SERVICE_CONTROLLER_AMOD, id, tt_estType)
{}

void AMOD_Controller::computeSchedules()
{

}