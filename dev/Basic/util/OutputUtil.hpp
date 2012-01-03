/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/**
 * \file OutputUtil.hpp
 * Contains functions which are helpful for formatting output on stdout.
 */

#include "GenConfig.h"



#include <vector>
#include <string>

#ifndef SIMMOB_DISABLE_OUTPUT
#include <boost/thread.hpp>
#endif

#include <iostream>
#include <fstream>


namespace sim_mob {

/**
 * Print an array of integers with separators and automatic line-breaks.
 * \param ids Integer values we are printing.
 * \param label Label for the entire output string.
 * \param brL Left bracket character.
 * \param brL Right bracket character.
 * \param comma Character to be used as a comma separator between items.
 * \param lineIndent Number of spaces to be used on each new line.
 */
void PrintArray(const std::vector<int>& ids, const std::string& label="", const std::string& brL="[", const std::string& brR="]", const std::string& comma=",", int lineIndent=2);

#ifndef SIMMOB_DISABLE_OUTPUT
class Logger
{
public:
	static boost::mutex global_mutex;
	static bool log_init(const std::string& path) {
		if (!path.empty()) {
			file_output.open(path.c_str());
			if (file_output.good()) {
				log_file_or_cout = &file_output;
				return true;
			}
		}

		log_file_or_cout = &std::cout;
		return false;
	}
	static void log_done() {
		if (file_output.is_open()) {
			file_output.close();
		}
	}
	static std::ostream& log_file() { return *log_file_or_cout; }

private:
	static std::ostream* log_file_or_cout;
	static std::ofstream file_output;
};
#endif

} //End sim_mob namespace


#ifndef SIMMOB_DISABLE_OUTPUT

/**
 * Write a message to the log file without any thread synchronization.
 *
 * Usage:
 *   \code
 *   if (count)
 *       LogOutNotSync("The total cost of " << count << " apples is " << count * unit_price);
 *   else
 *       LogOutNotSync("Why don't you buy something?");
 *   \endcode
 */
#define LogOutNotSync( strm ) \
    do \
    { \
        sim_mob::Logger::log_file() << strm; \
    } \
    while (0)

/**
 * Write a message to the log file, thread-safe.
 *
 * Usage:
 *   \code
 *   if (count)
 *       LogOut("The total cost of " << count << " apples is " << count * unit_price);
 *   else
 *       LogOut("Why don't you buy something?");
 *   \endcode
 */
#define LogOut( strm ) \
    do \
    { \
        boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex); \
        LogOutNotSync(strm); \
    } \
    while (0)


#endif
