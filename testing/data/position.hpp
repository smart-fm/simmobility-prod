#ifndef _position_hpp_
#define _position_hpp_

#include <Data.hpp>
#include <iosfwd>

namespace sim_mob
{

//! \brief The type to represent lengths and co-ordinates.
//
//! 32 bits unsigned ints are used to represent lengths.  For a resolution of 1 cm, then this type can
//! represent 42949672.96 meters or 42949.67296 km.  
typedef uint32_t length_t;

struct Position_pod
{
    length_t x_;
    length_t y_;

    Position_pod (length_t x, length_t y)
      : x_ (x)
      , y_ (y)
    {
    }

    bool operator== (const Position_pod& rhs) const
    {
        return (x_ == rhs.x_) && (y_ == rhs.y_);
    }
    bool operator!= (const Position_pod& rhs) const
    {
        return ! operator== (rhs);
    }
};

class Position : public Data<Position_pod>
{
public:
    Position (length_t x = 0, length_t y = 0)
      : Data<Position_pod> (Position_pod (x, y))
    {
    }

    length_t x() const { return get().x_; }
    length_t y() const { return get().y_; }

    void increment_x (int delta_x)
    {
        set (Position_pod (x() + delta_x, y()));
    }
    void increment_y (int delta_y)
    {
        set (Position_pod (x(), y() + delta_y));
    }
};

inline
std::ostream & operator<< (std::ostream & stream, Position const & pos)
{
    stream << "(" << pos.x() << ", " << pos.y() << ")";
    return stream;
}

}

#endif
