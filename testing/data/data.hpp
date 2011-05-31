#ifndef _data_hpp
#define _data_hpp

#include <trace.hpp>
#include <iosfwd>
#include <base.hpp>
#include <data_mgr.hpp>
#include <observer.hpp>
#include <set>

namespace sim_mob
{

template <typename T>
class Data : public Base
{
public:
    Data (const T& value = T())
      : is_dirty_ (false)
      , current_ (value)
      , next_ (current_)
    {
        DataManager::singleton().add (this);
    }

    T const & get() const
    {
        return current_;
    }

    void set (T const & value)
    {
        if (next_ != value)
        {
            next_ = value;
            is_dirty_ = true;
        }
    }

    /* virtual */ void add (Observer * observer)
    {
        observers_.insert (observer);
    }

protected:
    virtual void flip()
    {
        if (is_dirty_)
        {
            current_ = next_;
            notify();
            is_dirty_ = false;
        }
    }

    void notify()
    {
        for (std::set<Observer*>::iterator iter = observers_.begin(); iter != observers_.end(); ++iter)
        {
            Observer* observer = *iter;
            observer->notify (this);
        }
    }

    bool is_dirty_;
    T current_;
    T next_;
    std::set<Observer*> observers_;
};

template <typename T>
std::ostream & operator<< (std::ostream & stream, Data<T> const & data)
{
    stream << data.get();
    return stream;
}

}

#endif
