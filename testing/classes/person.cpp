/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "person.hpp"
#include "pedestrian.hpp"
#include "driver.hpp"
#include "cyclist.hpp"
#include "passenger.hpp"

namespace short_term
{

Person::Person(unsigned int id)
  : Agent()
  , id_(id)
  , mode_(0)
  , next_mode_(0)
{
    mode_ = new Pedestrian(*this);

    mit_sim::Position_pod position;
    //position = Database::lookup_person_initial_position(id);
    location_.set(position);

    add(location_);
}

/* virtual */ Person::~Person()
{
    delete mode_;
    delete next_mode_;
}

/* virtual */ void Person::perform(frame_t frame_number)
{
    if (next_mode_)
    {
        delete mode_;
        mode_ = next_mode_;
    }

    mode_->perform(frame_number);
}

void Person::start_driving()
{
    next_mode_ = new Driver(*this);
}

void Person::stop_driving()
{
    next_mode_ = new Pedestrian(*this);
}

void Person::start_cycling()
{
    next_mode_ = new Cyclist(*this);
}

void Person::stop_cycling()
{
    next_mode_ = new Pedestrian(*this);
}

// This is probably incorrect.  Pedestrians take turn to board vehicles.
void Person::board_vehicle(unsigned int vehicle_id)
{
    next_mode_ = new Passenger(*this, vehicle_id);
}

// This is probably incorrect.  Passengers take turn to alight vehicles.
void Person::alight_vehicle(unsigned int /* vehicle_id */)
{
    next_mode_ = new Pedestrian(*this);
}

Person::mode_t Person::current_mode() const
{
    return mode_->mode();
}

unsigned int Person::vehicle_id() const
{
    return mode_->vehicle_id();
}

}
