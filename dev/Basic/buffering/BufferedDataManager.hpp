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
class BufferedBase : private boost::noncopyable
{
protected:
	BufferedBase(BufferedDataManager& mgr);
    virtual ~BufferedBase();

	virtual void flip() = 0;

    //Allow access to protected methods by BufferedDataManager.
    friend class BufferedDataManager;

private:
    BufferedDataManager& mgr;

};



class BufferedDataManager
{
public:
	//No longer singleton...
	//BufferedDataManager();
	//~BufferedDataManager();

    //Data lifecycle management.
    void add (BufferedBase* datum);
    void rem (BufferedBase* datum);
    void flip();

//private:
    std::vector<BufferedBase*> managedData;
};


}


