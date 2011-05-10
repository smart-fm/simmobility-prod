#ifndef _base_hpp_
#define _base_hpp_

#include <boost/utility.hpp>

namespace mit_sim
{

class Base : private boost::noncopyable
{
protected:
    friend class DataManager;
    virtual void flip() = 0;
};

}

#endif
