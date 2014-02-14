//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Log.hpp"

using namespace sim_mob;

using std::string;
using std::map;


//StaticLogManager
map<const std::ostream*, boost::shared_ptr<boost::mutex> > sim_mob::StaticLogManager::stream_locks;

//Log
//boost::shared_ptr<boost::mutex>   sim_mob::Log::log_mutex;
//std::ostream*   sim_mob::Log::log_handle = &std::cout;
//std::ofstream   sim_mob::Log::log_file;

//Warn
boost::shared_ptr<boost::mutex>   sim_mob::Warn::log_mutex;
std::ostream*   sim_mob::Warn::log_handle = &std::cout;
std::ofstream   sim_mob::Warn::log_file;

//Print
boost::shared_ptr<boost::mutex>   sim_mob::Print::log_mutex;
std::ostream*   sim_mob::Print::log_handle = &std::cout;
std::ofstream   sim_mob::Print::log_file;

//PassengerInfoPrint
boost::shared_ptr<boost::mutex>   sim_mob::PassengerInfoPrint::log_mutex;
std::ostream*   sim_mob::PassengerInfoPrint::log_handle = &std::cout;
std::ofstream   sim_mob::PassengerInfoPrint::log_file;

//HeadwayAtBusStopInfoPrint
boost::shared_ptr<boost::mutex>   sim_mob::HeadwayAtBusStopInfoPrint::log_mutex;
std::ostream*   sim_mob::HeadwayAtBusStopInfoPrint::log_handle = &std::cout;
std::ofstream   sim_mob::HeadwayAtBusStopInfoPrint::log_file;


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
// Warn implementation
//////////////////////////////////////////////////////////////

sim_mob::Warn::Warn()
{
	if (log_mutex) {
		local_lock = boost::mutex::scoped_lock(*log_mutex);
	}
}


sim_mob::Warn::~Warn()
{
	//Flush any pending output to stdout.
	if (log_handle) {
		(*log_handle) <<std::flush;
	}
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

sim_mob::Print::Print()
{
	if (log_mutex) {
		local_lock = boost::mutex::scoped_lock(*log_mutex);
	}
}


sim_mob::Print::~Print()
{
	//Flush any pending output to stdout.
	if (log_handle) {
		(*log_handle) <<std::flush;
	}
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


//////////////////////////////////////////////////////////////
// PassengerInfoPrint implementation
//////////////////////////////////////////////////////////////

sim_mob::PassengerInfoPrint::PassengerInfoPrint()
{
	if (log_mutex) {
		local_lock = boost::mutex::scoped_lock(*log_mutex);
	}
}


sim_mob::PassengerInfoPrint::~PassengerInfoPrint()
{
	//Flush any pending output to stdout.
	if (log_handle) {
		(*log_handle) <<std::flush;
	}
}

void sim_mob::PassengerInfoPrint::Init(const string& path)
{
	log_handle = OpenStream(path, log_file);
	log_mutex = RegisterStream(log_handle);
}

void sim_mob::PassengerInfoPrint::Ignore()
{
	log_handle = nullptr;
	log_mutex.reset();
}

bool sim_mob::PassengerInfoPrint::IsEnabled()
{
	return log_handle;
}

//////////////////////////////////////////////////////////////
// HeadwayAtBusStopInfoPrint implementation
//////////////////////////////////////////////////////////////

sim_mob::HeadwayAtBusStopInfoPrint::HeadwayAtBusStopInfoPrint()
{
	if (log_mutex) {
		local_lock = boost::mutex::scoped_lock(*log_mutex);
	}
}

sim_mob::HeadwayAtBusStopInfoPrint::~HeadwayAtBusStopInfoPrint()
{
	//Flush any pending output to stdout.
	if (log_handle) {
		(*log_handle) <<std::flush;
	}
}

void sim_mob::HeadwayAtBusStopInfoPrint::Init(const string& path)
{
	log_handle = OpenStream(path, log_file);
	log_mutex = RegisterStream(log_handle);
}

void sim_mob::HeadwayAtBusStopInfoPrint::Ignore()
{
	log_handle = nullptr;
	log_mutex.reset();
}

bool sim_mob::HeadwayAtBusStopInfoPrint::IsEnabled()
{
	return log_handle;
}
