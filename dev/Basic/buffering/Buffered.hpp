/*
 * A templatized data type, copied from "testing/data/data.hpp".
 * Renamed, since "data" is likely going to come up elsewhere.
 * The following also makes sense linguistically:
 *    Buffered<int>
 * NOTE: The "Base" class is included here in BufferedDataManager, since its only purpose is to allow
 *       storage of Buffered templatized types.
 */

#pragma once


#include "BufferedDataManager.hpp"


namespace sim_mob
{




/**
 * NOTE: I removed most virtual functions; trying to make this class as simple as possible.
 *       Re-enable features as you need them.
 *  ~Seth
 */
template <typename T>
class Buffered : public BufferedBase
{
public:
	Buffered (const T& value = T());
	~Buffered();

    const T& get() const;
    void set (const T& value);

    //We need this for out-of-loop setting (e.g., reading from the config file)
    //  The other option is to read all properties in "tick 0" then flip once.
    void force(const T& value);

    //void add (Observer * observer);

protected:
    void flip();

    //NOTE: I left notify() out (the implementation is left commented out a few dozen lines below)
    //      because at the moment I'm not sure if we should have Buffered types acting as listeners;
    //      it seems like this might slow down data that one doesn't want to be notified of.
    // ~Seth
    //void notify();

private:
    bool is_dirty_;
    T current_;
    T next_;

    //std::set<Observer*> observers_;
};


template <typename T>
Buffered<T>::Buffered (const T& value)
  : is_dirty_ (false)
  , current_ (value)
  , next_ (value)
{
	BufferedDataManager::GetInstance().add(this);
}


template <typename T>
Buffered<T>::~Buffered ()
{
	BufferedDataManager::GetInstance().rem(this);
}


template <typename T>
const T& Buffered<T>::get() const
{
    return current_;
}


template <typename T>
void Buffered<T>::set (const T& value)
{
    if (next_ != value)
    {
        next_ = value;
        is_dirty_ = true;
    }
}


template <typename T>
void Buffered<T>::force (const T& value)
{
	next_ = current_ = value;
}


/*template <typename T>
void Buffered<T>::add (Observer* observer)
{
    observers_.insert (observer);
}*/


template <typename T>
void Buffered<T>::flip()
{
    if (is_dirty_)
    {
        current_ = next_;
        //notify();
        is_dirty_ = false;
    }
}

/*template <typename T>
void Buffered<T>::notify()
{
    for (std::set<Observer*>::iterator iter = observers_.begin(); iter != observers_.end(); ++iter)
    {
        Observer* observer = *iter;
        observer->notify (this);
    }
}*/


//Should this also output the "to be set" value?
// ~Seth
template <typename T>
std::ostream & operator<< (std::ostream & stream, Buffered<T> const & data)
{
    stream << data.get();
    return stream;
}

}

