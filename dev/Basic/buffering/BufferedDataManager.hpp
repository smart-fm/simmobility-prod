#pragma once

#include <vector>
#include <algorithm>

#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include <iostream>

namespace sim_mob
{


class BufferedDataManager;


/**
 * Base class for all buffered data. It is recommended to use the templatized sub-class Buffered
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
 */
class BufferedBase : private boost::noncopyable
{
protected:
	BufferedBase() : refCount(0) {}
    virtual ~BufferedBase() {
    	assert(refCount==0);   //Error if refCount's not zero.
    }

    ///Update the current data value with the old value. Makes no guarantees about
    ///  what happens to the old value; e.g., calling flip() twice without a set() in
    ///  between has undefined behavior.
	virtual void flip() = 0;

    //Allow access to protected methods by BufferedDataManager.
    friend class BufferedDataManager;

private:
    ///Count of BufferedDataManagers accessing this Buffered type.
    ///Helps catch harder-to-debug errors further down the line.
    unsigned int refCount;

public:
	//TEMP
	static boost::mutex global_mutex;

};


/**
 * Manager for buffered data. Original source based on "data_mgr.hpp".
 */
class BufferedDataManager
{
public:
	~BufferedDataManager();          ///<Remove all items when this manager is deleted.

    void add (BufferedBase* datum);  ///<Become responsible for a buffered data item.
    void rem (BufferedBase* datum);  ///<Stop tracking a buffered data item.

    /**
     * Flip (update the current value of) all buffered data items under your control.
     */
    void flip();


protected:
    std::vector<BufferedBase*> managedData;
};


}

