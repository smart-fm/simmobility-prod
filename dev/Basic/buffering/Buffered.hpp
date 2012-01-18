/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once


#include "BufferedDataManager.hpp"


namespace sim_mob
{


/**
 * Templatized wrapper for buffered objects.
 *
 * \author LIM Fung Chai
 * \author Seth N. Hetu
 *
 * A Buffered datum handle multiple readers and a single writer without
 * locking. The "flip" method is used to update the current value after calling "set".
 *
 * The spoken semantics of this template are sensible; for example:
 *   Buffered<int>
 * ...is a "Buffered int".
 *
 * \note
 * Do not delete this class until we decide what to do with it. See Shared.hpp for more information.
 *
 *  \todo Currently, Buffered types don't work well with classes. For example, if
 *  we have a "Buffered<Point2D> pos", then calling "pos.get().xPos = 10" will
 *  not work. You need to do something like "Point2D newPos(10, 20); pos.set(newPos)", which
 *  is pretty arbitrary and counter-intuitive. One option is to make a BufferedPoint2D class
 *  which extends BufferedBase, and give it "setXPos()" and "setYPos()" methods.
 *
 *   \par
 *   However, I think there must be a better way to do this without sacrificing the Buffered class's
 *   nice template syntax. This will only really become an issue later, when we release the API, so
 *   I'd rather think about a good solution for a while, instead of creating tons of customized BufferedXYZ
 *   pseudo-wrappers.
 *
 *   \par
 *   ~Seth
 *
 *   \note
 *   Since get returns a constant ref, pos.get().xPos = 10 won't compile anyway. But we may still need a solution.
 */
template <typename T>
class Buffered : public BufferedBase
{
public:
	/**
	 * Create a new Buffered data type.
	 *
	 * \param value The initial value. You can also set an initial value using "force".
	 */
	explicit Buffered (const T& value = T()) : BufferedBase(), current_ (value), next_ (value) {}
	virtual ~Buffered() {}


	/**
	 * Retrieve the current value. Get the current value of the data type. This can
	 * also be thought of as being one flip "behind" the actual value.
	 */
    const T& get() const {
    	return current_;
    }

	/**
	 * Set the next value. Set the next value of the data type. This value will
	 * only take effect when "flip" is called.
	 */
    void set (const T& value) {
    	next_ = value;
    }


	/**
	 * Skips processing for this time tick. If an agent won't be updating a particular
	 * Buffered type during its time tick, it should call skip() on that type.
	 *
	 * \note
	 * This is intended for later, when we have pointers to arrays of data to update.
	 * But modelers should definitely respect the limitations of Buffere<> types now.
	 */
    void skip() {
    	this->set(this->get());
    }


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
    void force(const T& value) {
    	next_ = current_ = value;
    }


protected:
    void flip() {
    	current_ = next_;
    }

    T current_;
    T next_;

};

}




