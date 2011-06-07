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

/**
 * Avoid circular dependencies.
 */
class BufferedBase : private boost::noncopyable
{
protected:
	virtual void flip() = 0;

//protected:
    virtual ~BufferedBase() {
    	//NOTE:This line should go in every sub-class; I'm not putting
    	//     it here because we can't chain constructors, and I don't want
    	//     "add" and "rem" in different class specifications.
    	// ~Seth
    	//BufferedDataManager::GetInstance().rem(this);
    }

    //Allow access to protected methods by BufferedDataManager.
    friend class BufferedDataManager;
};





class BufferedDataManager
{
public:
	/**
	 * The original singleton design pattern recommends naming this "GetInstance()"
	 */
    static BufferedDataManager& GetInstance();

    //Data lifecycle management.
    void add (BufferedBase* datum);
    void rem (BufferedBase* datum);
    void flip();

private:
    //Ensure our class cannot be instantiated directly.
    BufferedDataManager() {}

    static BufferedDataManager instance_;

    std::vector<BufferedBase*> managedData;
};


}


