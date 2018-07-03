//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

/**
 * \file OutputUtil.hpp
 *
 * Contains functions which are helpful for printing various items.
 *
 * \note
 * If you are looking for generalized logging (e.g., Print(),Warn()), look at "logging/Log.hpp"
 */


#include "conf/settings/DisableOutput.h"


#include <vector>
#include <string>

#include <boost/thread.hpp>

#include <iostream>
#include <fstream>

//TEMP: Chain to our new "Log" class.
//#include "logging/Log.hpp"

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


} //End sim_mob namespace



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

