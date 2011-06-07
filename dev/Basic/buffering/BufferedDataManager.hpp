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
    friend class BufferedDataManager;

//protected:
    virtual ~BufferedBase() {
    	//NOTE:This line should go in every sub-class; I'm not putting
    	//     it here because we can't chain constructors, and I don't want
    	//     "add" and "rem" in different class specifications.
    	// ~Seth
    	//BufferedDataManager::GetInstance().rem(this);
    }
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



//TEST CLASS: Boolean. (Inheritance is behaving weirdly in our templated class)
class BufferedBool : public BufferedBase
{
public:
	BufferedBool (const bool& value = bool())
	  : is_dirty_ (false)
	  , current_ (value)
	  , next_ (value)
	{
		BufferedDataManager::GetInstance().add(this);
	}

	~BufferedBool()
	{
		BufferedDataManager::GetInstance().rem(this);
	}


    const bool& get() const
    {
    	return current_;
    }

    void set (const bool& value)
    {
	    if (next_ != value)
	    {
	        next_ = value;
	        is_dirty_ = true;
	    }
    }

    void force(const bool& value)
    {
    	next_ = current_ = value;
    }

protected:
    void flip()
    {
	    if (is_dirty_)
	    {
	        current_ = next_;
	        //notify();
	        is_dirty_ = false;
	    }
    }

private:
    bool is_dirty_;
    bool current_;
    bool next_;

friend class BufferedDataManager;
};



}


