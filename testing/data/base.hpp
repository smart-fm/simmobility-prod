/* Copyright Singapore-MIT Alliance for Research and Technology */

#ifndef _base_hpp_
#define _base_hpp_

#include <boost/utility.hpp>

namespace sim_mob
{

class Observer;

class Base : private boost::noncopyable
{
public:
    virtual ~Base()
    {
    }

    virtual void add (Observer * observer) = 0;

protected:
    friend class DataManager;
    virtual void flip() = 0;
};

}

#endif
