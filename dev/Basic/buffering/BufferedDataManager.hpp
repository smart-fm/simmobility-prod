#pragma once

#include <vector>
#include <algorithm>

#include <boost/utility.hpp>

#include <iostream>

namespace sim_mob
{


class BufferedDataManager;


/**
 * Base class for all buffered data. The class Buffered extends this class and allows a template
 * parameter to be passed along.
 *
 * \note This used to be non-copyable, but we are allowing subclasses to access the constructor and
 * destructor, and we are defining an "equals" function which is relatively safe. So there's no
 * reason to prohibit copying.
 *
 * \par
 * ~Seth
 */
class BufferedBase
{
public:
	void migrate(sim_mob::BufferedDataManager* newMgr);

protected:
	BufferedBase(BufferedDataManager* mgr);
    virtual ~BufferedBase();
    virtual BufferedBase& operator=(const BufferedBase& rhs);

	virtual void flip() = 0;

    //Allow access to protected methods by BufferedDataManager.
    friend class BufferedDataManager;

private:
    BufferedDataManager* mgr;

};


/**
 * Manager for buffered data. Original source based on "data_mgr.hpp".
 */
class BufferedDataManager
{
public:
    //Data lifecycle management.
    void add (BufferedBase* datum);
    void rem (BufferedBase* datum);
    void flip();

private:
    std::vector<BufferedBase*> managedData;
};


}

