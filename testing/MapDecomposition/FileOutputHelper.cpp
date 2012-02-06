/*
 * FileOutputHelper.cpp
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#include "FileOutputHelper.hpp"

namespace partitioning {

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
