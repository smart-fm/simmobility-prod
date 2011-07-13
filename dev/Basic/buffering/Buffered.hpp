
#pragma once


#include "BufferedDataManager.hpp"


namespace sim_mob
{


/**
 * Templatized wrapper for buffered objects.
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
 * \note I removed most virtual functions; trying to make this class as simple as possible.
 *       Re-enable features as you need them.
 *  \par
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
	Buffered (/*BufferedDataManager* mgr, */const T& value = T());
	virtual ~Buffered();

	//virtual Buffered& operator=(const Buffered& rhs);


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
     * Evaluates as the current value.
     *
     * Used in an expression, the object will be evaluated as its current value.  Example:
     *     \code
     *     enum Color { GREEN, YELLOW, RED };
     *     Buffered<Color> color;
     *     ...
     *     switch (color)
     *     {
     *     case GREEN: ... break;
     *     case YELLOW: ... break;
     *     case RED: ... break;
     *     }
     *     \endcode
     *
     * Another example:
     *     \code
     *     Buffered<uint32_t> passenger_count;
     *     ...
     *     if (passenger_count == 10) ...
     *     std::cout << "There are " << passenger_count << " passengers on the bus\n";
     *     \endcode
     *
     *  Take care that this conversion is not called when \p T is big struct.
     */
    operator T() const
    {
        return current_;
    }

	/**
	 * Force a new value into effect. Set the current and next value without a call to flip().
	 * This is usually only needed when loading values from a config file.
	 */
    void force(const T& value);


protected:
    void flip();

    T current_;
    T next_;
};

}


template <typename T>
sim_mob::Buffered<T>::Buffered (/*BufferedDataManager* mgr, */const T& value) :
    BufferedBase(/*mgr*/),
    current_ (value), next_ (value)
{
}

template <typename T>
sim_mob::Buffered<T>::~Buffered ()
{
}

/*template <typename T>
sim_mob::Buffered<T>& sim_mob::Buffered<T>::operator=(const sim_mob::Buffered<T>& rhs)
{
	BufferedBase::operator =(rhs);
	this->current_ = rhs.current_;
	this->next_ = rhs.next_;

	return *this;
}*/


template <typename T>
const T& sim_mob::Buffered<T>::get() const
{
    return current_;
}


template <typename T>
void sim_mob::Buffered<T>::set (const T& value)
{
    next_ = value;
}


template <typename T>
void sim_mob::Buffered<T>::force (const T& value)
{
	next_ = current_ = value;
}




template <typename T>
void sim_mob::Buffered<T>::flip()
{
    current_ = next_;
}

template <typename T>
std::ostream & operator<< (std::ostream & stream, sim_mob::Buffered<T> const & data)
{
    stream << data.get();
    if (data.current_ != data.next_) {
    	stream <<"(" <<data.next_ <<")";
    }
    return stream;
}



