//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * RRMSGFactory.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/message/base/RR_FactoryBase.hpp"

namespace sim_mob {
namespace roadrunner{

///Subclass for roadrunner-only.
class RR_Factory : public sim_mob::roadrunner::RR_FactoryBase<false, sim_mob::roadrunner::MSG_MULTICAST, sim_mob::roadrunner::MSG_UNICAST, sim_mob::roadrunner::ClientDoneMessage> {};

}} //End namespace sim_mob::roadrunner


