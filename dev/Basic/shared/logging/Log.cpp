/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Log.hpp"

#include "util/LangHelpers.hpp"


using namespace sim_mob;

using std::string;
using std::map;


//Log
map<const std::ostream*, boost::mutex*> sim_mob::Log::stream_locks;

//Warn
boost::mutex*   sim_mob::Warn::log_mutex = nullptr;
std::ostream*   sim_mob::Warn::log_handle = &std::cout;
std::ofstream   sim_mob::Warn::log_file;


void sim_mob::Log::Done()
{
	for (map<const std::ostream*, boost::mutex*>::iterator it=stream_locks.begin(); it!=stream_locks.end(); it++) {
		delete it->second;
	}
	stream_locks.clear();
}

boost::mutex* sim_mob::Log::RegisterStream(const std::ostream* str)
{
	if (stream_locks.count(str)==0) {
		stream_locks[str] = new boost::mutex();
	}
	return stream_locks[str];
}


std::ostream* sim_mob::Log::OpenStream(const string& path, std::ofstream& file)
{
	if (path=="<stdout>") { return &std::cout; }
	if (path=="<stderr>") { return &std::cerr; }
	file.open(path.c_str());
	if (file.fail()) {
		return &file;
	} else {
		return &std::cout;
	}
}

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
	log_mutex = nullptr;
}

bool sim_mob::Warn::IsEnabled()
{
	return log_handle;
}
