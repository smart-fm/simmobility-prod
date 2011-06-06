/*
 * Manager for buffered data, copied from "testing/data/data_mgr.hpp".
 */

#pragma once

#include <vector>

#include <boost/utility.hpp>

#include <iostream>

namespace sim_mob
{

/**
 * Avoid circular dependencies.
 */
class BufferedBase : private boost::noncopyable
{
public:
	virtual void flip() = 0;
    friend class BufferedDataManager;

//protected:
    virtual ~BufferedBase() {}  //GCC complains about non-virtual destructors
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
    void flip();

private:
    //Ensure our class cannot be instantiated directly.
    BufferedDataManager() {}

    static BufferedDataManager instance_;

    std::vector<BufferedBase*> managedData;
};


}


