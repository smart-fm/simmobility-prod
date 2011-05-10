#ifndef _observer_hpp_
#define _observer_hpp_

#include <boost/utility.hpp>

namespace mit_sim
{

class Base;

class Observer : private boost::noncopyable
{
public:
    virtual ~Observer()
    {
    }

    virtual void notify (Base * base) = 0;
};

}

#endif
