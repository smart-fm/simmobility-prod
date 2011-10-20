#include "OutputUtil.hpp"

#include <sstream>
#include <iostream>

using std::vector;
using std::string;


boost::mutex sim_mob::Logger::global_mutex;
std::ostream* sim_mob::Logger::log_file_or_cout;
std::ofstream sim_mob::Logger::file_output;


void sim_mob::PrintArray(const vector<int>& ids, const string& label, const string& brL, const string& brR, const string& comma, int lineIndent)
{
	//Easy
	if (ids.empty()) {
		return;
	}

	//Buffer in a stringstream
	std::stringstream out;
	int lastSize = 0;
	out <<label <<brL;
	for (size_t i=0; i<ids.size(); i++) {
		//Output the number
		out <<ids[i];

		//Output a comma, or the closing brace.
		if (i<ids.size()-1) {
			out <<comma;

			//Avoid getting anyway near default terminal limits
			if (out.str().size()-lastSize>75) {
				out <<"\n" <<string(lineIndent, ' ');
				lastSize += (out.str().size()-lastSize)-1;
			}
		} else {
			out <<brR <<"\n";
		}
	}
	std::cout <<out.str();
}


