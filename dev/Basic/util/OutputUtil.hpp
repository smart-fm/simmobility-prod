#pragma once

/**
 * \file OutputUtil.hpp
 * Contains functions which are helpful for formatting output on stdout.
 */


#include <vector>
#include <string>


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


}
