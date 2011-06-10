
#pragma once


#include "BufferedDataManager.hpp"


namespace sim_mob
{


/**
 * \brief Templatized wrapper for buffered objects.
 *
 * A Buffered data-type can handle multiple readers and a single writer without
 * locking. The "flip" method is used to update the current value after calling "set".
 *
 * The spoken semantics of this template are sensible; for example:
 *   Buffered<int>
 * ...is a "Buffered int".
 *
 * Based on the file "testing/data/data.hpp"
 *
 * NOTE: I removed most virtual functions; trying to make this class as simple as possible.
 *       Re-enable features as you need them.
 *  ~Seth
 */
template <typename T>
class Buffered : public BufferedBase
{
public:
	/**
	 * Create a new Buffered data type.
	 * @param mgr The data manager which will call "flip". Can be NULL.
	 * @param value The initial value. You can also set an initial value using "force"
	 */
	Buffered (BufferedDataManager* mgr, const T& value = T());

	virtual Buffered& operator=(const Buffered& rhs);


	/**
	 * Retrieve the current value. Get the current value of the data type. This can
	 * also be thought of as being one flip "behind" the actual value.
	 */
    const T& get() const;

	/**
	 * Set the next value. Set the next value of the data type. This value will
	 * only take effect when "flip" is called.
	 */
    void set (const T& value);

	/**
	 * Force a new value into effect. Set the current and next value without a call to flip().
	 * This is usually only needed when loading values from a config file.
	 */
    void force(const T& value);


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
Buffered<T>::Buffered (BufferedDataManager* mgr, const T& value) :
    BufferedBase(mgr),
    is_dirty_ (false), current_ (value), next_ (value)
{

}

template <typename T>
Buffered<T>& Buffered<T>::operator=(const Buffered<T>& rhs)
{
	BufferedBase::operator =(rhs);
	this->is_dirty_ = rhs.is_dirty_;
	this->current_ = rhs.current_;
	this->next_ = rhs.next_;

	return *this;
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


template <typename T>
std::ostream & operator<< (std::ostream & stream, Buffered<T> const & data)
{
    stream << data.get();
    if (data.current_ != data.next_) {
    	stream <<"(" <<data.next_ <<")";
    }
    return stream;
}

}

