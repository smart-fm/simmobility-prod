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

#include "util/LangHelpers.hpp"


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
 */
class Log : private boost::noncopyable {
protected:
	///Where to send logging events. May point to std::cout, std::cerr,
	/// or a file stream located in a subclass.
	std::ostream*  log_handle;

protected:
	///NOTE: For now we force logging semantics into the subclasses to avoid the need for virtual destructors.
	Log() : log_handle(nullptr) {}

public:
	///Log a given item; this simply forwards the call to << of the given logger.
	///TODO: Returning by reference *should* keep this object alive until its work is done. Check this.
	template <typename T>
	Log& operator<< (const T& val) {
		if (log_handle) {
			(*log_handle) <<val;
		}
		return *this;
	}

protected:
	///Used for reference; each stream has one associated mutex.
	static std::map<const std::ostream*, boost::mutex*> stream_locks;

	///Helper function for subclasses: registers an ostream with stream_locks, skipping if it's already been
	///  registered. Returns the associated mutex regardless.
	static boost::mutex* RegisterStream(const std::ostream* str) {
		if (stream_locks.count(str)==0) {
			stream_locks[str] = new boost::mutex();
		}
		return stream_locks[str];
	}

	///Helper function for subclasses: If the associated filename is not "<stdout>" or "<stderr>", then
	///  open the ofstream object passed by reference and return a pointer to it. Else, return
	///  a pointer to std::cout or std::cerr. On error (if the file can't be created), silently
	///  return a pointer to cout.
	static std::ostream* OpenStream(const std::string& path, std::ofstream& file) {
		if (path=="<stdout>") { return &std::cout; }
		if (path=="<stderr>") { return &std::cerr; }
		file.open(path.c_str());
		if (file.fail()) {
			return &file;
		} else {
			return &std::cout;
		}
	}
};


///Sample Log subclass for handling warnings
class Warn : public Log {
public:
	Warn() : Log(), local_lock(nullptr) {
		if (enabled) {
			local_lock = new boost::mutex::scoped_lock(*log_mutex);
			log_handle = default_log_location;
		}
	}

	~Warn() {
		//Deleting will free the lock (if it exists in the first place).
		safe_delete_item(local_lock);
	}

	static void Init(bool isEnabled, const std::string& path) {
		enabled = isEnabled;
		if (enabled) {
			default_log_location = OpenStream(path, log_file);
			log_mutex = RegisterStream(default_log_location);
		}
	}

private:
	static boost::mutex* log_mutex;
	static std::ostream* default_log_location;
	static std::ofstream log_file; //Will delete/close itself after main() automatically.
	static bool enabled; //Is logging enabled for this Log subclass?

	boost::mutex::scoped_lock* local_lock;
};




} //End sim_mob namespace




//TODO: Useful macros (like "LogOut") go here:


