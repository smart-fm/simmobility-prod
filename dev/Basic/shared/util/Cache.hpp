#include <cassert>
#include <list>
#include <map>

namespace sim_mob
{
/// Base template class for all cache implementations
template <typename KEY, typename VAL> class Cache;

/// Template class for Least Recently Used Caching Policy(LRU)
template <typename KEY, typename VAL> class LRU_Cache : public Cache<KEY,VAL>{};

///	Specialization of LRU template class for the VAL type to accept shared_ptr only
template <typename K, typename VAL>
class LRU_Cache<K, boost::shared_ptr<VAL> >
{
public:
	typedef K KeyType;
	typedef boost::shared_ptr<VAL> ValueType;

	// Key access history, most recent at back
	typedef std::list<KeyType> keyTrackerType;

	// Key to value and key history iterator
	typedef std::map<KeyType, std::pair<ValueType, typename keyTrackerType::iterator> > KeyToValueType;

	/// Constuctor specifies the cached function and the maximum number of records to be stored
	LRU_Cache(size_t c) : capacity(c)
	{
		assert(capacity!=0);
	}

	/// Obtain value of the cached function for k
	bool find(const KeyType& key, ValueType & value)
	{
		boost::shared_lock<boost::shared_mutex> lock(mutex_);

		// Attempt to find existing record
		const typename KeyToValueType::iterator it = keyToValue.find(key);

		if (it==keyToValue.end())
		{
			return false;
		}
		else
		{
			// Update access record by moving accessed key to back of list
			keyTracker.splice(keyTracker.end(),	keyTracker,	(*it).second.second);

			// Return the retrieved value
			value = (*it).second.first;
			return true;
		}
	}

	/// Record a fresh key-value pair in the cache
	void insert(const KeyType& k,const ValueType& v)
	{
		boost::unique_lock<boost::shared_mutex> lock(mutex_);
		// Method is only called on cache misses
		assert(keyToValue.find(k)==keyToValue.end());

		// Make space if necessary
		if (keyToValue.size()==capacity)
		{
			evict();
		}

		// Record k as most-recently-used key
		typename keyTrackerType::iterator it = keyTracker.insert(keyTracker.end(),k);

		// Create the key-value entry,
		// linked to the usage record.
		keyToValue.insert( std::make_pair(k, std::make_pair(v,it)) );
		// No need to check return,
		// given previous assert.
	}

	int size()
	{
		return keyToValue.size();
	}

private:
	/// Purge the least-recently-used element in the cache
	void evict()
	{
		// Assert method is never called when cache is empty
		assert(!keyTracker.empty());

		// Identify least recently used key
		const typename KeyToValueType::iterator it = keyToValue.find(keyTracker.front());
		assert(it!=keyToValue.end());
		// Erase both elements to completely purge record
		keyToValue.erase(it);
		keyTracker.pop_front();
	}

	/// Maximum number of key-value pairs to be retained
	const size_t capacity;

	/// Key access history
	keyTrackerType keyTracker;

	/// Key-to-value lookup
	KeyToValueType keyToValue;

	///	protector
	boost::shared_mutex mutex_;
};
}//namespace
