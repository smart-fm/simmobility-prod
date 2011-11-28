/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <mode.hpp>

namespace sim_mob
{
namespace short_term
{

class Cyclist : public Transport_mode
{
public:
    Cyclist(Person & person)
      : Transport_mode(person)
    {
    }

    virtual ~Cyclist()
    {
    }

    void stop_cycling();

    virtual Person::mode_t mode() { return Person::CYCLING; }

    virtual void perform(frame_t frame_number);
};

} // namespace short_term
} // namespace sim_mob
