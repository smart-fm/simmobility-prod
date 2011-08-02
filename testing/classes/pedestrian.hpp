/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <mode.hpp>

namespace sim_mob
{
namespace short_term
{

class Pedestrian : public Transport_mode
{
public:
    Pedestrian(Person & person)
      : Transport_mode(person)
    {
    }

    virtual ~Pedestrian()
    {
    }

    void start_driving();
    void start_cycling();
    void board_vehicle(unsigned int vehicle_id);

    virtual Person::mode_t mode() const { return Person::WALKING; }

    virtual void perform(frame_t frame_number);
};

} // namespace short_term
} // namespace sim_mob
