#ifndef _data_hpp
#define _data_hpp

#include <trace.hpp>
#include <boost/utility.hpp>
#include <iosfwd>

namespace mit_sim
{

template <typename T>
class Data : private boost::noncopyable
{
public:
    Data (const T& value = T())
      : is_dirty_ (false)
      , current_ (value)
      , next_ (current_)
    {
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

    void flip()
    {
        if (is_dirty_)
        {
            current_ = next_;
            notify();
            is_dirty_ = false;
        }
    }

protected:
    void notify()
    {
        traceln ("I've have changed");
    }

    bool is_dirty_;
    T current_;
    T next_;
};

template <typename T>
std::ostream & operator<< (std::ostream & stream, Data<T> const & data)
{
    stream << data.get();
    return stream;
}

}

#endif
