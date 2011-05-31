#pragma once

#include <iosfwd>
#include <data/position.hpp>

namespace sim_mob
{
namespace short_term
{

class Transport_mode;

class Person : public Agent
{
public:
    //! \brief Person's constructor.
    //
    //! All person must have an id that is the primary key into the Person database table.
    explicit Person(unsigned int id);
    virtual ~Person();

    //! \brief Return this person's id.
    unsigned int id() const { return id_; }

    //! \brief Return this Person's location.
    //
    //! The returned value is double-buffered.  The get() method returns the x-y co-ordinates.
    Position const & location() const { return location_; }

    //! The various mobility style of moving around.
    enum mode_t
    {
        WALKING, //!< The Pedestrian transport-mode will do the walking.
        DRIVING, //!< The Driver transport-mode will do the driving.
        RIDING,  //!< The Passenger transport-mode will do the riding.
        CYCLING  //!< The Cyclist transport-mode will do the cycling.
    };

    //! \brief Switch to driving mode.
    void start_driving();
    //! \brief Stop driving and switch back to walking mode.
    void stop_driving();
    //! \brief Switch to cycling mode.
    void start_cycling();
    //! \brief Stop cycling and switch back to walking mode.
    void stop_cycling();
    //! \brief Switch to riding mode by becoming a passenger on a vehicle (bus, taxi, or private car).
    //
    //! \param vehicle_id is the primary key into the Vehicle database table.
    void board_vehicle(unsigned int vehicle_id);
    //! \brief Stop riding and switch back to walking mode.
    //
    //! Must be riding on vehicle with id specified by \param vehicle_id.
    void alight_vehicle(unsigned int vehicle_id);

    //! \brief Return the current mobility style.
    mode_t current_mode() const;

    //! \brief Return the vehicle-id if driving or riding; 0 otherwise.
    unsigned int vehicle_id() const;

    virtual void perform(frame_t frame_number);

private:
    friend class Transport_mode;

    unsigned int id_; //!< The primary key in the Person database table.
    Transport mode * mode_; //!< The current mobility method this agent is using to get around.
    Transport mode * next_mode_; //!< If not null, the agent will be switching to this mode in the next frame.
    Position location_; //!< The double-buffered position of this agent.
};

std::ostream & operator<< (std::ostream & stream, Person::mode_t mode)
{
    if (WALKING == mode) stream << "walking";
    else if (DRIVING == mode) stream << "driving";
    else if (CYCLING == mode) stream << "cycling";
    else if (RIDING == mode) stream << "riding";
    else stream << "moving around in the Great Network In The Sky";
    return stream;
}

} // namespace short_term
} // namespace sim_mob
