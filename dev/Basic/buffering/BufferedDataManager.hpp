/*
 * Manager for buffered data, copied from "testing/data/data_mgr.hpp".
 */

#pragma once

#include <vector>
#include <algorithm>

#include <boost/utility.hpp>

#include <iostream>

namespace sim_mob
{


class BufferedDataManager;


/**
 * Avoid circular dependencies.
 */
//NOTE: This used to be non-copyable, but we are allowing subclasses to access the constructor/destructor
//      and we are defining an "equals" function which is relatively safe. So there's no reason to make it
//      uncopyable....
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


