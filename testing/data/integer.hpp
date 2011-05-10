#ifndef _integer_hpp_
#define _integer_hpp_

#include <Data.hpp>
#include <iosfwd>

namespace mit_sim
{

class Integer : public Data<int>
{
public:
    Integer (int v = 0)
      : Data<int> (v)
    {
    }
};

std::ostream & operator<< (std::ostream & stream, Integer const & integer)
{
    stream << integer.get();
    return stream;
}

}

#endif
