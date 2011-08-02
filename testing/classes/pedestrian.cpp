/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "pedestrian.hpp"
#include "person.hpp"

namespace sim_mob
{
namespace short_term
{

void Pedestrian::start_driving()
{
    me_.start_driving();
}

void Pedestrian::start_cycling()
{
    me_.start_cycling();
}

// This is probably incorrect.  Pedestrians take turn to board vehicles.
void Pedestrian::board_vehicle(unsigned int vehicle_id)
{
    me_.board_vehicle(vehicle_id);
}

} // namespace short_term
} // namespace sim_mob
