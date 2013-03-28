/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Log.hpp"

#include "util/LangHelpers.hpp"


using namespace sim_mob;

using std::string;
using std::map;


//StaticLogManager
map<const std::ostream*, boost::shared_ptr<boost::mutex> > sim_mob::StaticLogManager::stream_locks;

//Log
boost::shared_ptr<boost::mutex>   sim_mob::Log::log_mutex;
std::ostream*   sim_mob::Log::log_handle = &std::cout;
std::ofstream   sim_mob::Log::log_file;

//Warn
boost::shared_ptr<boost::mutex>   sim_mob::Warn::log_mutex;
std::ostream*   sim_mob::Warn::log_handle = &std::cout;
std::ofstream   sim_mob::Warn::log_file;

//Print
boost::shared_ptr<boost::mutex>   sim_mob::Print::log_mutex;
std::ostream*   sim_mob::Print::log_handle = &std::cout;
std::ofstream   sim_mob::Print::log_file;


boost::shared_ptr<boost::mutex> sim_mob::StaticLogManager::RegisterStream(const std::ostream* str)
{
	if (stream_locks.count(str)==0) {
		stream_locks[str] = boost::shared_ptr<boost::mutex>(new boost::mutex());
	}
	return stream_locks[str];
}


std::ostream* sim_mob::StaticLogManager::OpenStream(const string& path, std::ofstream& file)
{
	if (path=="<stdout>") { return &std::cout; }
	if (path=="<stderr>") { return &std::cerr; }
	file.open(path.c_str());
	if (file.fail()) { return &std::cout; }
	return &file;
}


//////////////////////////////////////////////////////////////
// Log implementation
//////////////////////////////////////////////////////////////

sim_mob::Log::Log() : local_lock(nullptr)
{
	if (log_mutex) {
		local_lock = new boost::mutex::scoped_lock(*log_mutex);
	}
}


sim_mob::Log::~Log()
{
	//Flush any pending output to stdout.
	if (log_handle) {
		(*log_handle) <<std::flush;
	}

	//Deleting will free the lock (if it exists in the first place).
	safe_delete_item(local_lock);
}

void sim_mob::Log::Init(const string& path)
{
	log_handle = OpenStream(path, log_file);
	log_mutex = RegisterStream(log_handle);
}

void sim_mob::Log::Ignore()
{
	log_handle = nullptr;
	log_mutex.reset();
}

bool sim_mob::Log::IsEnabled()
{
	return log_handle;
}



//////////////////////////////////////////////////////////////
// Warn implementation
//////////////////////////////////////////////////////////////

sim_mob::Warn::Warn() : local_lock(nullptr)
{
	if (log_mutex) {
		local_lock = new boost::mutex::scoped_lock(*log_mutex);
	}
}


sim_mob::Warn::~Warn()
{
	//Flush any pending output to stdout.
	if (log_handle) {
		(*log_handle) <<std::flush;
	}

	//Deleting will free the lock (if it exists in the first place).
	safe_delete_item(local_lock);
}

void sim_mob::Warn::Init(const string& path)
{
	log_handle = OpenStream(path, log_file);
	log_mutex = RegisterStream(log_handle);
}

void sim_mob::Warn::Ignore()
{
	log_handle = nullptr;
	log_mutex.reset();
}

bool sim_mob::Warn::IsEnabled()
{
	return log_handle;
}


//////////////////////////////////////////////////////////////
// Print implementation
//////////////////////////////////////////////////////////////

sim_mob::Print::Print() : local_lock(nullptr)
{
	if (log_mutex) {
		local_lock = new boost::mutex::scoped_lock(*log_mutex);
	}
}


sim_mob::Print::~Print()
{
	//Flush any pending output to stdout.
	if (log_handle) {
		(*log_handle) <<std::flush;
	}

	//Deleting will free the lock (if it exists in the first place).
	safe_delete_item(local_lock);
}

void sim_mob::Print::Init(const string& path)
{
	log_handle = OpenStream(path, log_file);
	log_mutex = RegisterStream(log_handle);
}

void sim_mob::Print::Ignore()
{
	log_handle = nullptr;
	log_mutex.reset();
}

bool sim_mob::Print::IsEnabled()
{
	return log_handle;
}
