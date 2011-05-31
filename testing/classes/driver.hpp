#pragma once

#include <mode.hpp>

namespace sim_mob
{
namespace short_term
{

class Vehicle;

class Driver : public Transport_mode
{
public:
    Driver(Person & person);
    virtual ~Driver();

    void stop_driving();

    virtual Person::mode_t mode() const { return Person::DRIVING; }

    virtual unsigned int vehicle_id() const;

protected:
    Vehicle* vehicle_;
};

} // namespace short_term
} // namespace sim_mob
