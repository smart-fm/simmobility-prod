#pragma once


#include <vector>
#include <string>


//Contains helper output functions.

namespace sim_mob {

//Print an array of integers with separators and auto-line breaks.
void PrintArray(const std::vector<int>& ids, const std::string& label, const std::string& brL, const std::string& brR, const std::string& comma, int lineIndent);


}
