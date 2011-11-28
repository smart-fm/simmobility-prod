/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <mode.hpp>

namespace sim_mob
{
namespace short_term
{

class Passenger : public Transport_mode
{
public:
    Passenger(Person & person, unsigned int vehicle_id);
    virtual ~Passenger()

    void stop_riding();

    virtual Person::mode_t mode() { return Person::RIDING; }

    virtual unsigned int vehicle_id() const { return vehicle_id_; }

protected:
    unsigned int vehicle_id_;
    Position const * driver_position_;
};

} // namespace short_term
} // namespace sim_mob
