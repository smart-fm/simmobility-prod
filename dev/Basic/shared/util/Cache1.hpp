#include <cassert>
#include <list>
#include <map>

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
public:

  typedef K key_type;
  typedef boost::shared_ptr<VAL> value_type;

  // Key access history, most recent at back
  typedef std::list<key_type> key_tracker_type;

  // Key to value and key history iterator
  typedef std::map<
    key_type,
    std::pair<
      value_type,
      typename key_tracker_type::iterator
      >
  > key_to_value_type;

  // Constuctor specifies the cached function and
  // the maximum number of records to be stored
  LRU_Cache(

    size_t c
  )
    :capacity(c)
  {
    assert(capacity!=0);
  }

  // Obtain value of the cached function for k
  bool find(const key_type& key,value_type & value) {

    // Attempt to find existing record
    const typename key_to_value_type::iterator it
      =_key_to_value.find(key);

    if (it==_key_to_value.end()) {
      return false;
    } else {
      // Update access record by moving
      // accessed key to back of list
      _key_tracker.splice(
        _key_tracker.end(),
        _key_tracker,
        (*it).second.second
      );

      // Return the retrieved value
      value = (*it).second.first;
      return true;
    }
  }

  // Record a fresh key-value pair in the cache
  void insert(const key_type& k,const value_type& v) {

    // Method is only called on cache misses
    assert(_key_to_value.find(k)==_key_to_value.end());

    // Make space if necessary
    if (_key_to_value.size()==capacity)
      evict();

    // Record k as most-recently-used key
    typename key_tracker_type::iterator it
      =_key_tracker.insert(_key_tracker.end(),k);

    // Create the key-value entry,
    // linked to the usage record.
    _key_to_value.insert(
      std::make_pair(
        k,
        std::make_pair(v,it)
      )
    );
    // No need to check return,
    // given previous assert.
  }

private:
  // Purge the least-recently-used element in the cache
  void evict() {

    // Assert method is never called when cache is empty
    assert(!_key_tracker.empty());

    // Identify least recently used key
    const typename key_to_value_type::iterator it
      =_key_to_value.find(_key_tracker.front());
    assert(it!=_key_to_value.end());

    // Erase both elements to completely purge record
    _key_to_value.erase(it);
    _key_tracker.pop_front();
  }

  // Maximum number of key-value pairs to be retained
  const size_t capacity;

  // Key access history
  key_tracker_type _key_tracker;

  // Key-to-value lookup
  key_to_value_type _key_to_value;
};
}//namespace
