/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "OutputUtil.hpp"

#include <sstream>
#include <iostream>

using std::vector;
using std::string;

boost::mutex sim_mob::Logger::global_mutex;
std::ostream* sim_mob::Logger::log_file_or_cout;
std::ofstream sim_mob::Logger::file_output;





