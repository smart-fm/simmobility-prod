#ifndef _integer_hpp_
#define _integer_hpp_

#include <Data.hpp>

namespace sim_mob
{

class Integer : public Data<int>
{
public:
    Integer (int v = 0)
      : Data<int> (v)
    {
    }
};

}

#endif
