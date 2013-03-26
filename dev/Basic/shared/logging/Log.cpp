/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Log.hpp"

//Log
std::map<const std::ostream*, boost::mutex*> sim_mob::Log::stream_locks;

//Warn
boost::mutex*   sim_mob::Warn::log_mutex;
std::ostream*   sim_mob::Warn::default_log_location;
std::ofstream   sim_mob::Warn::log_file;
bool            sim_mob::Warn::enabled;





