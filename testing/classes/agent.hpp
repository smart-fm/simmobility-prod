/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <entity.hpp>

namespace sim_mob
{
namespace short_term
{

//! \brief Base class for all objects that has reasoning and decision-making capabilities.
//
//! Derived classes must implement the Entity::perform() method.
class agent : public entity
{
public:
    Agent() {}
    virtual ~Agent() {}
};

} // namespace short_term
} // namespace sim_mob
