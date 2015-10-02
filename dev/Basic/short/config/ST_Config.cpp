#include "ST_Config.hpp"

namespace sim_mob
{

VehicleType::VehicleType() : name(""), length(-1.0), width(-1.0), capacity(-1)
{}

bool VehicleType::operator==(const VehicleType &rhs) const
{
    return (this->name == rhs.name);
}

bool VehicleType::operator!=(const VehicleType &rhs) const
{
    return (this->name != rhs.name);
}

EntityTemplate::EntityTemplate() : startTimeMs(0), laneIndex(0),originNode(-1),
    destNode(-1),initSegId(-1),initDis(-1),initSpeed(0),angentId(-1), tripId(-1),
    vehicleId(-1), vehicleType("")
{}

ST_Config* ST_Config::instance = nullptr;

ST_Config& ST_Config::getInstance()
{
    if(!instance)
    {
	instance = new ST_Config();
    }
    return *instance;
}

void ST_Config::deleteIntance()
{
    safe_delete_item(instance);
}

}
