/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../all_includes.hpp"

using namespace std;

namespace sim_mob_partitioning {
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
