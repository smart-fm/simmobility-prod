#pragma once

#include <vector>
#include <boost/utility.hpp>
#include "../data/base.hpp"

namespace sim_mob
{
namespace short_term
{

//! \brief The type to represent frame numbers.
//
//! Frame numbers are 32 bits unsigned ints.  If the smallest time-step is 0.1 second, then this type
//! can represent 429496729.6 seconds or 119304.64711111112 hours, or 4971.0269629629629 days or
//! 13.61030271318301 years.
typedef uint32_t frame_t;

//! \brief Base class to represent tasks that must be performed in every frame (time-step).
//
//! Entity objects may used the double-buffered data types for any mutable properties that are exposed to
//! other (Entity) objects.  The Entity object is then responsible to flip its double-buffered data.  The
//! constructor of derived classes can call add() for each double-buffered data so that flip() will update
//! them.
class Entity : private boost::noncopyable
{
public:
    Entity() {}
    virtual ~Entity() {}

    //! \brief The code that will run in every frame.
    //
    //! Derived classes must implement perform().  The Worker thread class will invoke the perform() method
    //! in the simulation main loop where a strictly monotonic incresing sequence is passed as the \param
    //! frame_number parameter.  Inside the perform() method, the \param frame_number parameter can be
    //! converted into wall-clock time via the Configuration singleton object.
    virtual void perform(frame_t frame_number) = 0;

protected:
    //! \brief Manage the double-buffered \param data.
    //
    //! The constructor of classes derived from Entity should call add() for each of its double-buffered
    //! data.
    void add (Base & data)
    {
        datum_.push_back (&data);
    }

    friend class Worker;
    //! \brief Update all double-buffered data managed by this entity.
    //
    //! The Worker object which is managing this Entity will invoke its flip() at the appropriate time.
    void flip()
    {
        for (size_t i = 0; i < datum_.size(); ++i)
        {
            datum_[i]->flip();
        }
    }

    std::vector<Base*> datum_; //!< The list of double-buffered data that this entity is managing.
};

} // namespace short_term
} // namespace sim_mob
