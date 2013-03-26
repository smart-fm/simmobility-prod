/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/**
 * \file Log.hpp
 *
 * Contains a more flexible logging framework than util/OutputUtil.hpp and is intended to
 * eventually replace it.
 *
 * \note
 * The functionality here is still untested; once it's stable we will remove OutputUtil and
 * replace it with this.
 */


//This is a minimal header file, so please keep includes to a minimum.
#include "conf/settings/DisableOutput.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>


namespace sim_mob {


/**
 * Used to record an event that should be logged. This generally includes simulation output
 * and other saved messages (including some profiling information).
 *
 * This class is intended to be used as a verb; e.g.,
 * Log() <<"Testing: " <<x <<"," <<y;
 *
 * During construction, a mutex is seized. During destruction, the mutex is released and the
 * output buffer is flushed (so a trailing "\n" will have the same effect as std::endl).
 *
 * Log and its subclasses must each have their static "Init()" methods called at some point before
 * logging the first point of data. (TODO: We might provide default "stdout" logging behavior in
 * the event that logging hasn't been initialized yet.) Init() must not be called from multiple threads
 * at the same time.
 *
 * Logs which access the same stream are locked with the same mutex. As a result, it is not currently
 * possible to change the log location at runtime (calling Init() multiple times is an error).
 *
 * When the simulation closes, make sure to call Log::Done() to clean up mutexes (log files should close
 *  automatically). Failure to call this function will merely leak memory.
 *
 * Note: We'd make the constructor private, but there's really nothing *wrong with using the base class...
 *       it's just useless.
 */
class Log : private boost::noncopyable {

public:
	static void Done();

protected:
	///Used for reference; each stream has one associated mutex.
	static std::map<const std::ostream*, boost::mutex*> stream_locks;

	///Helper function for subclasses: registers an ostream with stream_locks, skipping if it's already been
	///  registered. Returns the associated mutex regardless.
	static boost::mutex* RegisterStream(const std::ostream* str);

	///Helper function for subclasses: If the associated filename is not "<stdout>" or "<stderr>", then
	///  open the ofstream object passed by reference and return a pointer to it. Else, return
	///  a pointer to std::cout or std::cerr. On error (if the file can't be created), silently
	///  return a pointer to cout.
	static std::ostream* OpenStream(const std::string& path, std::ofstream& file);
};


///Sample Log subclass for handling warnings
class Warn : private Log {
public:
	Warn();

	~Warn();

	///Log a given item; this simply forwards the call to << of the given logger.
	///TODO: Returning by reference *should* keep this object alive until its work is done. Check this.
	template <typename T>
	Warn& operator<< (const T& val);

	//Multiple calls to Init() *might* work, and the system should default to cout
	// if Init() has not been called. Either way, you should plan to call Init() once.
	static void Init(bool enabled, const std::string& path);

private:
	static boost::mutex* log_mutex;
	static std::ostream* default_log_location;
	static std::ofstream log_file; //Will delete/close itself after main() automatically.

	boost::mutex::scoped_lock* local_lock;

	///Where to send logging events. May point to std::cout, std::cerr,
	/// or a file stream located in a subclass.
	std::ostream*  log_handle;
};




} //End sim_mob namespace




//Templates:

template <typename T>
sim_mob::Warn& sim_mob::Warn::operator<< (const T& val) {
	if (log_handle) {
		(*log_handle) <<val;
	}
	return *this;
}



//TODO: Useful macros (like "LogOut") go here:


