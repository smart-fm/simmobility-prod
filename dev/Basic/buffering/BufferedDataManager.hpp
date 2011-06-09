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
	virtual void flip() = 0;

	BufferedBase(BufferedDataManager& mgr) : mgr(mgr) {

	}

    virtual ~BufferedBase() {
    	//NOTE:This line should go in every sub-class; I'm not putting
    	//     it here because we can't chain constructors, and I don't want
    	//     "add" and "rem" in different class specifications.
    	// ~Seth
    	//BufferedDataManager::GetInstance().rem(this);
    }

    BufferedDataManager& mgr;

    //Allow access to protected methods by BufferedDataManager.
    friend class BufferedDataManager;
};





class BufferedDataManager
{
public:
	//No longer singleton...
	BufferedDataManager();
	~BufferedDataManager();

    //Data lifecycle management.
    void add (BufferedBase* datum);
    void rem (BufferedBase* datum);
    void flip();

//private:
    std::vector<BufferedBase*> managedData;
};


}


