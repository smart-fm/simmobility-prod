#ifndef _big_brother_hpp_
#define _big_brother_hpp_

#include <observer.hpp>
#include <map>

namespace sim_mob
{

class BigBrother : public Observer
{
public:
    static BigBrother & singleton()
    {
        if (0 == instance_)
        {
            instance_ = new BigBrother;
        }
        return *instance_;
    }

    void add (Base * data);

    /* virtual */ void notify (Base * data);

private:
    BigBrother()
    {
    }

    static BigBrother * instance_;

    std::map<Base*, size_t> datum_;
};

}

#endif
