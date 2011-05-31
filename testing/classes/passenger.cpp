#include "passenger.hpp"

namespace sim_mob
{
namespace short_term
{

Passenger::Passenger(Person & person, unsigned int vehicle_id)
  : Transport_mode(person)
  , vehicle_id_(vehicle_id)
{
}

void Passenger::stop_riding()
{
    me_.stop_riding();
}

Passenger::~Passenger()
{
}

} // namespace short_term
} // namespace sim_mob
