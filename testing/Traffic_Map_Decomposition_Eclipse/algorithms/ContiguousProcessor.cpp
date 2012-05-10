/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "ContiguousProcessor.hpp"

using namespace std;
using namespace sim_mob_partitioning;

void ContiguousProcessor::do_contiguous(std::string node_file, std::string section_file, std::string partition_file)
{
	/**
	 * Generate the command for contiguous processing Algorithm
	 */
	string command_contiguous = "java -jar resources/ContiguousProcessing.jar";
	command_contiguous += " " + node_file;
	command_contiguous += " " + section_file;
	command_contiguous += " " + partition_file;

	/**
	 * Execute commands (Java Codes)
	 */
	if (system(command_contiguous.c_str()) != 0)
		throw "command FlowAndInforToHMETIS failed";
}
