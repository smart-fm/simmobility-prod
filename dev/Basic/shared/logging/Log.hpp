//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "util/LangHelpers.hpp"


namespace sim_mob {


/**
 * Used to record an event that should be logged. This generally includes simulation output
 * and other saved messages (including some profiling information).
 *
 * \author Seth N. Hetu
 *
 * The general usage pattern for this class is as follows. We use the subclass "Warn"
 *   as an example, since StaticLogManager() cannot be used directly.
 *
 *   \code
 *   //Do this once for each subclass of StaticLogManager, at the top of Main:
 *   if ( someConfigCheck() ) { //Should logging be enabled?
 *     Warn::Init("warnings.log")
 *   } else {
 *     Warn::Ignore()
 *   }
 *
 *   //Now use it:
 *   Warn() <<"Testing: " <<x <<"," <<y <<std::endl;
 *   Warn() <<"Using a newline instead of endl is fine too.\n";
 *   \endcode
 *
 * If Init() or Ignore() have not been called on a particular subclass, a default, non-synchronized stream
 * will be used (usually std::cout). If Ignore() has been called, then the system will silently ignore all
 * output, and no locking will take place. Some processing time is still wasted, so we recommend using the
 * macros (e.g., WarnLine("Test:" <<x <<"\n");) in areas where performance is crucial. Static functions
 * such as Warn::IsEnabled() can be used to check if logging on a particular subclass has been Ignored.
 *
 * Once Init() has been called with reasonable input (either a file name or "<stdout>" or "<stderr>" to
 * redirect to the standard streams), then the StaticLogManager subclasses function as follows. First, a mutex is
 * seized during construction of the temporary StaticLogManager object. Calls to operator<< are redirected to
 * the stream's operator of the same name. Finally, when the StaticLogManager temporary is destructed, the output
 * buffer is flushed and the mutex is released.
 *
 * It is functionally possible to create a StaticLogManager object that does not lock, but this is considered useless,
 * since the user would have to know that locking is not required to use that function, and mutexes which
 * are only seized by one entity incur almost no overhead. If overhead is a proble, it is likely that one
 * actually intended to disable output entirely anyway.
 *
 * StaticLogManagers which access the same stream are locked with the same mutex. As a result, it is not currently
 * possible to change the log location at runtime (calling Init() multiple times is risky, although it
 * is not explicitly wrong).
 *
 * \note
 * The StaticLogManager class provides only basic (static) functionality; you should use objects of its subclass
 * to accomplish your logging needs.
 */
class StaticLogManager : private boost::noncopyable {
protected:
	///Used for reference; each stream has one associated mutex. The keys (ostream pointers) are not allocated;
	/// they are just pointers to existing statically-scoped objects (or cout/cerr).
	static std::map<const std::ostream*, boost::shared_ptr<boost::mutex> > stream_locks;

	///Helper function for subclasses: registers an ostream with stream_locks, skipping if it's already been
	///  registered. Returns the associated mutex regardless.
	static boost::shared_ptr<boost::mutex> RegisterStream(const std::ostream* str);

	///Helper function for subclasses: If the associated filename is not "<stdout>" or "<stderr>", then
	///  open the ofstream object passed by reference and return a pointer to it. Else, return
	///  a pointer to std::cout or std::cerr. On error (if the file can't be created), silently
	///  return a pointer to cout.
	static std::ostream* OpenStream(const std::string& path, std::ofstream& file);

public:
	//Type of cout.
	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

	//Type of std::endl and some other manipulators.
	typedef CoutType& (*StandardEndLine)(CoutType&);
};



/**
 * Logging functionality for Warnings. See the StaticLogManager class for general details about how logging works.
 * A "Warn" item includes non-fatal items that may threaten the validity of the simulation (but will not cause it to crash).
 *
 * To be used like so:
 *
 *   \code
 *   Warn() <<"There was a warning: " <<str <<std::endl;
 *   \endcode
 *
 * ...or, if performance is critical:
 *
 *   \code
 *   WarnOut("There was a warning: " <<str <<std::endl);
 *   \endcode
 *
 * Don't forget to call Warn::Init() or Warn::Ignore() at the beginning of main().
 */
class Warn : private StaticLogManager {
public:
	///Construct a new Warn() object. It is best to use this object immediately, by chaining to calls of operator<<.
	Warn();

	///Destroy a Warn() object. Logging is mutually exclusive on a given output stream until the Warn object
	/// has been destroyed.
	~Warn();

	///Log a given item; this simply forwards the call to operator<< of the given logger.
	///NOTE: I am assuming that return-by-reference keeps the object alive until all chained
	///      operator<<'s are done. Should check the standard on this. ~Seth
	template <typename T>
	Warn& operator<< (const T& val);

    ///Hack to get manipulators (std::endl) to work.
	///NOTE: I have *no* idea if this is extremely stupid or not. ~Seth
	Warn& operator<<(StandardEndLine manip) {
		if (log_handle) {
			manip(*log_handle);
		}
		return *this;
	}

	///Ininitialize this StaticLogManager subclass. If "path" is "<stdout>" or "<stderr>", then bind to
	///std::cout or std::cerr. Else, attempt to open the file pointed to by "path".
	///On failure, bind to std::cout.
	///
	///\note
	///Multiple calls to Init() *might* work, and the system should default to cout
	/// if Init() has not been called. Either way, you should plan to call Init() once.
	static void Init(const std::string& path);

	///Disable logging for this StaticLogManager subclass.
	static void Ignore();

	///Is this StaticLogManager subclass enabled for writing? If not, calls to operator<< will be ignored.
	static bool IsEnabled();

private:
	///A pointer to the mutex (managed in StaticLogManager::stream_locks) used for locking the output stream.
	static boost::shared_ptr<boost::mutex> log_mutex;

	///Where to send logging events. May point to std::cout, std::cerr,
	/// or a file stream located in a subclass.
	static std::ostream* log_handle;

	///The actual file used for logging. If stdout or stderr are used for logging, this
	/// file will be un-opened. After main() terminates, this file will be destroyed and
	/// closed automatically.
	static std::ofstream log_file;

	///A scoped lock on the log_mutex. May be null, in which case output is not locked.
	boost::mutex::scoped_lock local_lock;
};


/**
 * Logging functionality for Printed output. See the StaticLogManager class for general details about how logging works.
 * A "Print" item includes console (cout) output, and is used to inform the user of the state of the simulation.
 * This should (almost always) point to cout.
 *
 * To be used like so:
 *
 *   \code
 *   Print() <<"Simulation is " <<x <<"% done.\n";
 *   \endcode
 *
 * ...or, if performance is critical:
 *
 *   \code
 *   PrintOut("Simulation is " <<x <<"% done.\n");
 *   \endcode
 *
 * Don't forget to call Print::Init() or Print::Ignore() at the beginning of main().
 */
class Print : private StaticLogManager {
public:
	///Construct a new Print() object. It is best to use this object immediately, by chaining to calls of operator<<.
	Print();

	///Destroy a Print() object. Logging is mutually exclusive on a given output stream until the Warn object
	/// has been destroyed.
	~Print();

	///Log a given item; this simply forwards the call to operator<< of the given logger.
	///NOTE: I am assuming that return-by-reference keeps the object alive until all chained
	///      operator<<'s are done. Should check the standard on this. ~Seth
	template <typename T>
	Print& operator<< (const T& val);

    ///Hack to get manipulators (std::endl) to work.
	///NOTE: I have *no* idea if this is extremely stupid or not. ~Seth
	Print& operator<<(StandardEndLine manip) {
		if (log_handle) {
			manip(*log_handle);
		}
		return *this;
	}

	///Ininitialize this StaticLogManager subclass. If "path" is "<stdout>" or "<stderr>", then bind to
	///std::cout or std::cerr. Else, attempt to open the file pointed to by "path".
	///On failure, bind to std::cout.
	///
	///\note
	///Multiple calls to Init() *might* work, and the system should default to cout
	/// if Init() has not been called. Either way, you should plan to call Init() once.
	static void Init(const std::string& path);

	///Disable logging for this StaticLogManager subclass.
	static void Ignore();

	///Is this StaticLogManager subclass enabled for writing? If not, calls to operator<< will be ignored.
	static bool IsEnabled();

private:
	///A pointer to the mutex (managed in StaticLogManager::stream_locks) used for locking the output stream.
	static boost::shared_ptr<boost::mutex> log_mutex;

	///Where to send logging events. May point to std::cout, std::cerr,
	/// or a file stream located in a subclass.
	static std::ostream* log_handle;

	///The actual file used for logging. If stdout or stderr are used for logging, this
	/// file will be un-opened. After main() terminates, this file will be destroyed and
	/// closed automatically.
	static std::ofstream log_file;

	///A scoped lock on the log_mutex. May be null, in which case output is not locked.
	boost::mutex::scoped_lock local_lock;
};

class PassengerInfoPrint : private StaticLogManager {
public:
	///Construct a new Warn() object. It is best to use this object immediately, by chaining to calls of operator<<.
	PassengerInfoPrint();

	///Destroy a Warn() object. Logging is mutually exclusive on a given output stream until the Warn object
	/// has been destroyed.
	~PassengerInfoPrint();

	///Log a given item; this simply forwards the call to operator<< of the given logger.
	///NOTE: I am assuming that return-by-reference keeps the object alive until all chained
	///      operator<<'s are done. Should check the standard on this. ~Seth
	template <typename T>
	PassengerInfoPrint& operator<< (const T& val);

    ///Hack to get manipulators (std::endl) to work.
	///NOTE: I have *no* idea if this is extremely stupid or not. ~Seth
	PassengerInfoPrint& operator<<(StandardEndLine manip) {
		if (log_handle) {
			manip(*log_handle);
		}
		return *this;
	}

	///Ininitialize this StaticLogManager subclass. If "path" is "<stdout>" or "<stderr>", then bind to
	///std::cout or std::cerr. Else, attempt to open the file pointed to by "path".
	///On failure, bind to std::cout.
	///
	///\note
	///Multiple calls to Init() *might* work, and the system should default to cout
	/// if Init() has not been called. Either way, you should plan to call Init() once.
	static void Init(const std::string& path);

	///Disable logging for this StaticLogManager subclass.
	static void Ignore();

	///Is this StaticLogManager subclass enabled for writing? If not, calls to operator<< will be ignored.
	static bool IsEnabled();

private:
	///A pointer to the mutex (managed in StaticLogManager::stream_locks) used for locking the output stream.
	static boost::shared_ptr<boost::mutex> log_mutex;

	///Where to send logging events. May point to std::cout, std::cerr,
	/// or a file stream located in a subclass.
	static std::ostream* log_handle;

	///The actual file used for logging. If stdout or stderr are used for logging, this
	/// file will be un-opened. After main() terminates, this file will be destroyed and
	/// closed automatically.
	static std::ofstream log_file;

	///A scoped lock on the log_mutex. May be null, in which case output is not locked.
	boost::mutex::scoped_lock local_lock;
};

class HeadwayAtBusStopInfoPrint : private StaticLogManager {
public:
	///Construct a new Warn() object. It is best to use this object immediately, by chaining to calls of operator<<.
	HeadwayAtBusStopInfoPrint();

	///Destroy a Warn() object. Logging is mutually exclusive on a given output stream until the Warn object
	/// has been destroyed.
	~HeadwayAtBusStopInfoPrint();

	///Log a given item; this simply forwards the call to operator<< of the given logger.
	///NOTE: I am assuming that return-by-reference keeps the object alive until all chained
	///      operator<<'s are done. Should check the standard on this. ~Seth
	template <typename T>
	HeadwayAtBusStopInfoPrint& operator<< (const T& val);

    ///Hack to get manipulators (std::endl) to work.
	///NOTE: I have *no* idea if this is extremely stupid or not. ~Seth
	HeadwayAtBusStopInfoPrint& operator<<(StandardEndLine manip) {
		if (log_handle) {
			manip(*log_handle);
		}
		return *this;
	}

	///Ininitialize this StaticLogManager subclass. If "path" is "<stdout>" or "<stderr>", then bind to
	///std::cout or std::cerr. Else, attempt to open the file pointed to by "path".
	///On failure, bind to std::cout.
	///
	///\note
	///Multiple calls to Init() *might* work, and the system should default to cout
	/// if Init() has not been called. Either way, you should plan to call Init() once.
	static void Init(const std::string& path);

	///Disable logging for this StaticLogManager subclass.
	static void Ignore();

	///Is this StaticLogManager subclass enabled for writing? If not, calls to operator<< will be ignored.
	static bool IsEnabled();

private:
	///A pointer to the mutex (managed in StaticLogManager::stream_locks) used for locking the output stream.
	static boost::shared_ptr<boost::mutex> log_mutex;

	///Where to send logging events. May point to std::cout, std::cerr,
	/// or a file stream located in a subclass.
	static std::ostream* log_handle;

	///The actual file used for logging. If stdout or stderr are used for logging, this
	/// file will be un-opened. After main() terminates, this file will be destroyed and
	/// closed automatically.
	static std::ofstream log_file;

	///A scoped lock on the log_mutex. May be null, in which case output is not locked.
	boost::mutex::scoped_lock local_lock;
};

} //End sim_mob namespace



//////////////////////////////////////////////////////////////
// Macros for each StaticLogManager subclass.
//////////////////////////////////////////////////////////////

#ifdef SIMMOB_DISABLE_OUTPUT

//Simply destroy this text; no logging; no locking
#define WarnOut( strm )  DO_NOTHING
#define PrintOut( strm )  DO_NOTHING

//TODO: Describe in detail later.
#define LogOut( strm )  DO_NOTHING

#else



/**
 * Exactly the same as LogOut(), but for Warnings.
 * TODO: Describe better.
 */
#define WarnOut( strm ) \
    do \
    { \
        sim_mob::Warn() << strm; \
    } \
    while (0)

/**
 * Exactly the same as LogOut(), but for Print statements.
 */
#define PrintOut( strm ) \
    do \
    { \
        sim_mob::Print() << strm; \
    } \
    while (0)


/**
 * Write a message (statement_list) using "Log() <<statement_list"; Compiles to nothing if output is disabled.
 *
 * Usage:
 *   \code
 *   //This:
 *   LogOut("The total cost of " << count << " apples is " << count * unit_price);
 *
 *   //Is equivalent to this:
 *   Log() <<"The total cost of " << count << " apples is " << count * unit_price;
 *   \endcode
 *
 * \note
 * If SIMMOB_DISABLE_OUTPUT is defined, this macro will discard its arguments. Thus, it is safe to
 * call this function without #ifdef guards and let cmake handle whether or not to display output.
 * In some cases, it is still wise to check SIMMOB_DISABLE_OUTPUT; for example, if you are building up
 * an output std::stringstream. However, in this case you should call Log::IsEnabled().
 */
#define LogOut( strm ) \
    do \
    { \
        Log() << strm; \
    } \
    while (0)


#endif //SIMMOB_DISABLE_OUTPUT




//////////////////////////////////////////////////////////////
// Template function implementation.
//////////////////////////////////////////////////////////////


template <typename T>
sim_mob::Warn& sim_mob::Warn::operator<< (const T& val)
{
	if (log_handle) {
		(*log_handle) <<val;
	}
	return *this;
}

template <typename T>
sim_mob::Print& sim_mob::Print::operator<< (const T& val)
{
	if (log_handle) {
		(*log_handle) <<val;
	}
	return *this;
}

template <typename T>
sim_mob::PassengerInfoPrint& sim_mob::PassengerInfoPrint::operator<< (const T& val)
{
	if (log_handle) {
		(*log_handle) <<val;
	}
	return *this;
}

template <typename T>
sim_mob::HeadwayAtBusStopInfoPrint& sim_mob::HeadwayAtBusStopInfoPrint::operator<< (const T& val)
{
	if (log_handle) {
		(*log_handle) <<val;
	}
	return *this;
}
