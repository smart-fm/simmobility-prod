//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ControllerLog.hpp"

using namespace sim_mob;

using std::string;
using std::map;


//ControllerLog
boost::shared_ptr<boost::mutex>   sim_mob::ControllerLog::log_mutex;
std::ostream*   sim_mob::ControllerLog::log_handle = &std::cout;
std::ofstream   sim_mob::ControllerLog::log_file;


//////////////////////////////////////////////////////////////
// ControllerLog implementation
//////////////////////////////////////////////////////////////

sim_mob::ControllerLog::ControllerLog()
{
	if (log_mutex) {
		local_lock = boost::mutex::scoped_lock(*log_mutex);
	}
}


sim_mob::ControllerLog::~ControllerLog()
{
	//Flush any pending output to stdout.
	if (log_handle) {
		(*log_handle) <<std::flush;
	}
}

std::ostream* sim_mob::ControllerLog::CreateStream(const string& path, std::ofstream& file)
{
	if (path=="<stdout>") { return &std::cout; }
	if (path=="<stderr>") { return &std::cerr; }
	file.open(path.c_str(), std::ofstream::trunc);
	if (file.fail()) { return &std::cout; }
	return &file;
}

void sim_mob::ControllerLog::Init(const string& path)
{
	log_handle = CreateStream(path, log_file);
	log_mutex = RegisterStream(log_handle);
}

void sim_mob::ControllerLog::Ignore()
{
	log_handle = nullptr;
	log_mutex.reset();
}

bool sim_mob::ControllerLog::IsEnabled()
{
	return log_handle;
}

