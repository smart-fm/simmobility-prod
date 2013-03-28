/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/**
 * \file OutputUtil.hpp
 *
 * Contains functions which are helpful for synchronized output to cout and log files.
 */


#include "conf/settings/DisableOutput.h"



#include <vector>
#include <string>

#include <boost/thread.hpp>

#include <iostream>
#include <fstream>

namespace sim_mob {

/**
 * Print an array of basic types with separators and automatic line-breaks.
 * This function buffers output to the stream so that it is all sent at once.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 *
 * \param arr Integer values we are printing.
 * \param out Output stream to receive the output. Defaults to cout.
 * \param label Label for the entire output string.
 * \param brL Left bracket character.
 * \param brL Right bracket character.
 * \param comma Character to be used as a comma separator between items.
 * \param lineIndent Number of spaces to be used on each new line.
 */
template <typename T>
void PrintArray(const std::vector<T>& arr, std::ostream& out=std::cout, const std::string& label="", const std::string& brL="[", const std::string& brR="]", const std::string& comma=",", int lineIndent=2);

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

} //End sim_mob namespace


#ifdef SIMMOB_DISABLE_OUTPUT

//Simply destroy this text; no logging; no locking
#define LogOutNotSync( strm )      DO_NOTHING
#define LogOut( strm )  DO_NOTHING
#define SyncCout( strm )  DO_NOTHING


#else

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
 *
 * \note
 * If SIMMOB_DISABLE_OUTPUT is defined, this macro will discard its arguments. Thus, it is safe to
 * call this function without #ifdef guards and let cmake handle whether or not to display output.
 * In some cases, it is still wise to check SIMMOB_DISABLE_OUTPUT; for example, if you are building up
 * an output std::stringstream. However, in this case you should call ConfigParams::GetInstance().OutputEnabled().
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
 *
 * \note
 * If SIMMOB_DISABLE_OUTPUT is defined, this macro will discard its arguments. Thus, it is safe to
 * call this function without #ifdef guards and let cmake handle whether or not to display output.
 * In some cases, it is still wise to check SIMMOB_DISABLE_OUTPUT; for example, if you are building up
 * an output std::stringstream. However, in this case you should call ConfigParams::GetInstance().OutputEnabled().
 */
 
#define LogOut( strm ) \
    do \
    { \
        boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex); \
        sim_mob::Logger::log_file() << strm; \
    } \
    while (0)

/**
 * Write a message to cout, thread-safe.
 *
 * Usage:
 *   \code
 *   if (count)
 *       SyncCout("The total cost of " << count << " apples is " << count * unit_price);
 *   else
 *       SyncCout("Why don't you buy something?");
 *   \endcode
 *
 * \note
 * Like the other logging functions, this does nothing if SIMMOB_DISABLE_OUTPUT is defined.
 * Consider using a regular "cout" for simple debugging messages that are not performance-related
 * (as SIMMOB_DISABLE_OUTPUT exists for performance purposes).
 */

#define SyncCout( strm ) \
    do \
    { \
        boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex); \
        std::cout << strm; \
    } \
    while (0)


#endif



////////////////////////////////////////////////
// Template implementation
////////////////////////////////////////////////


template <typename T>
void sim_mob::PrintArray(const std::vector<T>& ids, std::ostream& out, const std::string& label, const std::string& brL, const std::string& brR, const std::string& comma, int lineIndent)
{
	//Easy
	if (ids.empty()) {
		return;
	}

	//Buffer in a stringstream
	std::stringstream buff;
	int lastSize = 0;
	buff <<label <<brL;
	for (size_t i=0; i<ids.size(); i++) {
		//Output the number
		buff <<ids[i];

		//Output a comma, or the closing brace.
		if (i<ids.size()-1) {
			buff <<comma;

			//Avoid getting anyway near default terminal limits
			if (buff.str().size()-lastSize>75) {
				buff <<"\n" <<std::string(lineIndent, ' ');
				lastSize += (buff.str().size()-lastSize)-1;
			}
		} else {
			buff <<brR <<"\n";
		}
	}
	out <<buff.str();
}

