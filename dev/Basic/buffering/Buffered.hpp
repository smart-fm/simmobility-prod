
#pragma once


#include "BufferedDataManager.hpp"


namespace sim_mob
{


/**
 * Templatized wrapper for buffered objects.
 *
 * A Buffered datum handle multiple readers and a single writer without
 * locking. The "flip" method is used to update the current value after calling "set".
 *
 * The spoken semantics of this template are sensible; for example:
 *   Buffered<int>
 * ...is a "Buffered int".
 *
 *  \todo Currently, Buffered types don't work well with classes. For example, if
 *  we have a "Buffered<Point2D> pos", then calling "pos.get().xPos = 10" will
 *  not work. You need to do something like "Point2D newPos(10, 20); pos.set(newPos)", which
 *  is pretty arbitrary and counter-intuitive. One option is to make a BufferedPoint2D class
 *  which extends BufferedBase, and give it "setXPos()" and "setYPos()" methods.
 *
 *   \par
 *   However, I think there must be a better way to do this without sacraficing the Buffered class's
 *   nice template syntax. This will only really become an issue later, when we release the API, so
 *   I'd rather think about a good solution for a while, instead of creating tons of customized BufferedXYZ
 *   pseudo-wrappers.
 *
 *   \par
 *   ~Seth
 */
template <typename T>
class Buffered : public BufferedBase
{
public:
	/**
	 * Create a new Buffered data type.
	 *
	 * @param value The initial value. You can also set an initial value using "force".
	 */
	Buffered (const T& value = T()) : BufferedBase(), current_ (value), next_ (value) {}
	virtual ~Buffered() {}


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


    //
    // Note: I'm removing this; we don't gain much by checking if the value has
    //       changed, and the BufferedDataManager will flip all elements anyway.
    //       If we had Buffered<SomeBigClass>, and SomeBigClass was very expensive to
    //       copy, then it might be useful, but currently classes are troublesome as
    //       buffered types. So, I'm leaving this out until it's actually needed.
    // ~Seth
    //
    //bool is_dirty_;
};

}

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


//
//May be needed later for debugging. For now, just cout <<datum.get()
//
/*template <typename T>
std::ostream & operator<< (std::ostream & stream, sim_mob::Buffered<T> const & data)
{
    stream << data.get();
    if (data.current_ != data.next_) {
    	stream <<"(" <<data.next_ <<")";
    }
    return stream;
}*/



