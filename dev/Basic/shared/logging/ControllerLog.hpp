#pragma once

#include "logging/Log.hpp"
#include "metrics/Frame.hpp" // For using operator << on timeslice
#include "message/MobilityServiceControllerMessage.hpp"

namespace sim_mob {

class ControllerLog : private StaticLogManager {	
public:
	///Construct a new ControllerLog() object. It is best to use this object immediately, by chaining to calls of operator<<.
	ControllerLog();

	///Destroy a ControllerLog() object. Logging is mutually exclusive on a given output stream until the Warn object
	/// has been destroyed.
	~ControllerLog();

	///Log a given item; this simply forwards the call to operator<< of the given logger.
	///NOTE: I am assuming that return-by-reference keeps the object alive until all chained
	///      operator<<'s are done. Should check the standard on this. ~Seth
	template <typename T>
	ControllerLog& operator<< (const T& val);

    ///Hack to get manipulators (std::endl) to work.
	///NOTE: I have *no* idea if this is extremely stupid or not. ~Seth
	ControllerLog& operator<<(StandardEndLine manip) {
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
	static std::ostream* CreateStream(const std::string& path, std::ofstream& file);

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
}


//////////////////////////////////////////////////////////////
// Macros for each StaticLogManager subclass.
//////////////////////////////////////////////////////////////

#ifdef SIMMOB_DISABLE_OUTPUT

//Simply destroy this text; no logging; no locking
#define ControllerLogOut( strm )  DO_NOTHING

#else

#define ControllerLogOut( strm ) \
    do \
    { \
        sim_mob::ControllerLog() << strm; \
    } \
    while (0)

#endif //SIMMOB_DISABLE_OUTPUT

template <typename T>
sim_mob::ControllerLog& sim_mob::ControllerLog::operator<< (const T& val)
{
	if (log_handle) {
		(*log_handle) <<val;
	}
	return *this;
}


