#include "driver.hpp"
#include "vehicle.hpp"  // TODO

namespace sim_mob
{
namespace short_term
{

Driver::Driver(Person & person)
  : Transport_mode(person)
{
    //unsigned int vehicle_id = Database::
    vehicle_ = new Vehicle(vehicle_id);
}

Driver::~Driver()
{
    delete vehicle_;
}

void Driver::stop_driving()
{
    //Network::singleton().place_vehicle(vehicle_id(), me_.location().get());
    me_.stop_driving();
}

/* virtual */ unsigned int vehicle_id() const
{
    return vehicle_->id();
}

} // namespace short_term
} // namespace sim_mob
