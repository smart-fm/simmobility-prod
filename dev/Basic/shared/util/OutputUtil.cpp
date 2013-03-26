/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "OutputUtil.hpp"

#include <sstream>
#include <iostream>

using std::vector;
using std::string;

boost::mutex sim_mob::Logger::global_mutex;
std::ostream* sim_mob::Logger::log_file_or_cout;
std::ostream* sim_mob::Logger::log_file_or_cout1;
std::ofstream sim_mob::Logger::file_output;
std::ofstream sim_mob::Logger::file_output1;





