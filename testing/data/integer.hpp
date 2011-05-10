#ifndef _integer_hpp_
#define _integer_hpp_

#include <Data.hpp>

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

}

#endif
