//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AMODController.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: Max
 */

#include "AMODController.hpp"
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include "entities/Person.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/Link.hpp"
#include <utility>
#include <stdexcept>

namespace sim_mob {
namespace AMOD
{
AMODController* AMODController::pInstance = nullptr;
//boost::asio::io_service AMODController::ioService;

sim_mob::AMOD::AMODController::~AMODController() {
	// TODO Auto-generated destructor stub
}
bool AMODController::instanceExists()
{
	return pInstance;
}
AMODController* AMODController::instance()
{
	if (!pInstance) {
		pInstance = new AMODController();
	}

	return pInstance;
}

bool AMODController::frame_init(timeslice now)
{
	return true;
}

Entity::UpdateStatus AMODController::frame_tick(timeslice now)
{
	//TODO

}

void AMODController::frame_output(timeslice now)
{

}




} /* namespace AMOD */
} /* namespace sim_mob */
