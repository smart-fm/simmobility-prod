/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <algorithm>
#include <cassert>

#include <boost/utility.hpp>

namespace sim_mob
{


class BufferedDataManager;


/**
 * Base class for all buffered data.
 *
 * \author LIM Fung Chai
 * \author Seth N. Hetu
 *
 * It is recommended to use the templatized sub-class Buffered
 * for actual data. This base class exists to allow the BufferedDataManager to store a vector of
 * non-templatized pointers and perform "flip" operations on them.
 *
 * A BufferedBase must be associate with a BufferedDataManager; otherwise, its current value will never
 * be updated.
 *
 * \note
 * This class is non-copyable; it is not clear semantically what happens when a Buffered data
 * item is copied. In particular, who should now manage this class? If a copy is needed, consider
 * implementing a "clone" function in BufferedDataManager, since this will allow exact control
 * over data management (and BufferedDataManager is already a friend class).
 *
 * \par
 * ~Seth
 *
 */
class BufferedBase : private boost::noncopyable
{
protected:
	BufferedBase() : refCount(0) {}
    virtual ~BufferedBase() {
    	assert(refCount==0);   //Error if refCount's not zero.
    }

    /**
     * Update the current data value with the old value. Makes no guarantees about
     * what happens to the old value; e.g., calling flip() twice without a set() in
     * between has undefined behavior.
     */
	virtual void flip() = 0;

    //Allow access to protected methods by BufferedDataManager.
    friend class BufferedDataManager;

private:
    ///Count of BufferedDataManagers accessing this Buffered type.
    ///Helps catch harder-to-debug errors further down the line.
    unsigned int refCount;
};


/**
 * Manager for buffered data. Maintains a list of which buffered types it is managing, and
 * updates their current values each time flip() is called. Calling flip() multiple times
 * in a row (without calling each datum's "set()" method in between) has undefined behavior.
 *
 * \todo
 * It seems sensible to have beginManaging() add the datum to a static array of raw "data", using
 * the "size" of the buffered type. The next_ and current_ values can then be represented by two
 * large static arrays, which can be flipped with a single pointer swap. Care should be taken to
 * update the arrays when they grow too big. In addition, internal segmentation will develop
 * when an item is removed through a call to stopManaging(). So, it's not a trivial feature, but
 * then again there's plenty of existing research (for dealing with general memory allocation).
 *
 * \par
 * ~Seth
 */
class BufferedDataManager
{
public:
	virtual ~BufferedDataManager();

	///Become responsible for a buffered data item.
    void beginManaging(BufferedBase* datum);

    ///Stop tracking a buffered data item.
    void stopManaging(BufferedBase* datum);

    //For multiple items
    void beginManaging(std::vector<sim_mob::BufferedBase*> data);
    void stopManaging(std::vector<sim_mob::BufferedBase*> data);

    ///Flip (update the current value of) all buffered data items under your control.
    void flip();


protected:
    std::vector<BufferedBase*> managedData;
};


}

