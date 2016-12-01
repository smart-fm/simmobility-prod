//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Driver.hpp"

#include <cmath>
#include <ostream>
#include <algorithm>
#include "DriverFacets.hpp"
#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/AuraManager.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"

#include "buffering/BufferedDataManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/Point.hpp"

#include "logging/Log.hpp"
#include "util/DebugFlags.hpp"

#include "partitions/PartitionManager.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#include "entities/Person_MT.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

//Helper functions
namespace {
//TODO:I think lane index should be a data member in the lane class
size_t getLaneIndex(const Lane* l) {
	if (l) {
		const RoadSegment* r = l->getParentSegment();
		for (size_t i = 0; i < r->getLanes().size(); i++) {
			if (r->getLanes().at(i) == l) {
				return i;
			}
		}
	}
	return -1; //NOTE: This might not do what you expect! ~Seth
}
} //end of anonymous namespace

//Initialize
sim_mob::medium::Driver::Driver(Person_MT* parent,
		sim_mob::medium::DriverBehavior* behavior,
		sim_mob::medium::DriverMovement* movement,
		std::string roleName, Role<Person_MT>::Type roleType) :
	sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName, roleType),
	currLane(nullptr)
{}

sim_mob::medium::Driver::~Driver() {}

vector<BufferedBase*> sim_mob::medium::Driver::getSubscriptionParams() {
	return vector<BufferedBase*>();
}

void sim_mob::medium::Driver::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

Role<Person_MT>* sim_mob::medium::Driver::clone(Person_MT* parent) const
{
	DriverBehavior* behavior = new DriverBehavior();
	DriverMovement* movement = new DriverMovement();
	Driver* driver = new Driver(parent, behavior, movement, "Driver_");
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);
	return driver;
}

void sim_mob::medium::DriverUpdateParams::reset(timeslice now)
{
	UpdateParams::reset(now);

	secondsInTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	elapsedSeconds = 0.0;
}

void sim_mob::medium::Driver::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type)
	{
	case MSG_INSERT_INCIDENT:
	{
		Movement()->handleMessage(type, message);
		break;
	}
	}
}

const sim_mob::medium::DriverMode & sim_mob::medium::Driver::getDriveMode() const
{
	return Driver::driverMode;
}

void sim_mob::medium::Driver::setDriveMode(const DriverMode &driveMode)
{
	sim_mob::medium::Driver::driverMode = driveMode;
}
