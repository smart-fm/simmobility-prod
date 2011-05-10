#ifndef _position_hpp_
#define _position_hpp_

#include <Data.hpp>
#include <iosfwd>

namespace mit_sim
{

struct Position_pod
{
    int x_;
    int y_;

    Position_pod (int x, int y)
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
    Position (int x = 0, int y = 0)
      : Data<Position_pod> (Position_pod (x, y))
    {
    }

    int x() const { return get().x_; }
    int y() const { return get().y_; }

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
