/*
 * FileOutputHelper.hpp
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#pragma once

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

namespace partitioning {
	class FileOutputHelper
	{
	public:
		void openFile(std::string fileURL);
		void closeFile();
		void output_to_file(std::string one_line);

	private:
		ofstream outputFile;
	};
}
