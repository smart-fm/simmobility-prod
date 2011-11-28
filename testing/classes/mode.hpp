/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <boost/utility.hpp>
#include <entity.hpp>
#include <person.hpp>

namespace sim_mob
{
namespace short_term
{

class Transport_mode : private boost::noncopyable
{
public:
    Transport_mode(Person & person)
      : me_(person)
    {
    }

    virtual ~Transport_mode()
    {
    }

    virtual Person::mode_t mode() const = 0;

    virtual unsigned int vehicle_id() const { return 0; }

    virtual void perform(frame_t frame_number) = 0;

protected:
    Person & me_;
};

} // namespace short_term
} // namespace sim_mob
