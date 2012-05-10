/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "FileOutputHelper.hpp"

namespace sim_mob_partitioning {

void FileOutputHelper::closeFile()
{
	outputFile.flush();
	outputFile.close();
}

void FileOutputHelper::openFile(std::string fileURL)
{
	outputFile.open(fileURL.c_str());
}

void FileOutputHelper::output_to_file(std::string one_line)
{
	outputFile << one_line << "\n";
}

}
