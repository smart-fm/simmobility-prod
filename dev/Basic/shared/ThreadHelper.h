/* 
 * File:   ThreadHelper.h
 * Author: gandola
 *
 * Created on April 5, 2013, 3:15 PM
 */

#include <boost/thread.hpp>

using namespace boost;

#define SharedReadLock(var_mutex) shared_lock<shared_mutex> __r_locker__(var_mutex)

#define SharedWriteLock(var_mutex) \
        upgrade_lock<shared_mutex> __u_locker__(var_mutex); \
        upgrade_to_unique_lock<shared_mutex> __uu_locker__()
