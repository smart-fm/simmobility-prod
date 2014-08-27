#pragma once
#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/function.hpp>
#include <cassert>

namespace sim_mob
{
/// Base template class for all cache implementations
template <typename KEY, typename VAL>  class Cache;

/// Template class for Least Recently Used Caching Policy(LRU)
template <typename KEY, typename VAL>  class LRU_Cache :public Cache<KEY,VAL>{};

/************************************************
 ****************** L R U ***********************
 ************************************************/
///	Specialization of LRU template class for
//the VAL type to accept shared_ptr only
template <typename K, typename VAL>
class LRU_Cache<K, boost::shared_ptr<VAL> >
{
//public:
	typedef boost::shared_ptr<VAL> V;
protected:
	boost::bimap<K,V> container;
	size_t capacity;
public:
	LRU_Cache(size_t capacity)
	:capacity(capacity)
	{
		BOOST_ASSERT_MSG(capacity != 0, "Cache Size with " << capacity << " Size Specified");
	}

	virtual ~LRU_Cache(){}

	bool find(const K& key, V& output){
		typename boost::bimap<K,V>::left_iterator it = container.find(key);
		if(it == container.end()){
			return false;
		}
		else
		{
			//copy to output
			output = *it;
			//move to end to update LRU
			container.right.relocate(container.right.end(),container,container.project_right(it));
			return true;
		}
	}

	void add(const K& key, V& value)
	{
		//evict
		if(container.size() == capacity)
		{
			evict();
		}
		container.right.insert(std::make_pair(key,value));
	}

	virtual void evict()
	{
		if(!container.empty() && container.size() == capacity)
		{
			container.right.erase(container.right.begin());
		}
	}
};
/************************************************
 *************** Extended L R U *****************
 ************************************************/
/// In order to make the life for code readers simple, the extra/incremental features are defined in a separate class.
/// There are 2 extensions to the basic LRU
///	1. Invoke the primary data generation/retrieval routine when there is a cache miss. To do this, instead of overriding find(),
///	a new method "V get(K&)" is added
///	2. The new eviction method removes a portion of LRU cache entries rather than just 1 entry. Default ratio is 10% .

///	Note: in order to used this class with eviction method behaving like that of its base class, do this: ratio= 100/capacity


template<typename K, typename V, size_t RATIO = 10>
class Extended_LRU_Cache : public LRU_Cache<K, boost::shared_ptr<V> >
{
	boost::function<V(K&)> generator;
public:
	Extended_LRU_Cache(size_t capacity,boost::function<V(K&)> generator = 0):LRU_Cache<K,V>(capacity), generator(generator)
	{
	}

	V get(K& key)
	{
		typename boost::bimap<K,V>::left_iterator it = this->container.find(key);
		if(it == this->container.end()){
			V value = generator(key);
			return value;
		}
		else
		{
			//copy to output
			V value = *it;
			//move to end to update LRU
			this->container.right.relocate(this->container.right.end(),this->container,this->container.project_right(it));
			return value;
		}
	}

	void evict()
	{
		if(this->container.size() >= this->capacity)
		{
			//how much to delete
			short ratio = this->capacity*RATIO/100;
			typename boost::bimap<K,V>::reverse_right_iterator it = this->container.begin();
			for(; it != this->container.rend() && (--ratio > 0) ; this->container.right.erase(it++));
		}
	}

};

}//namespace
