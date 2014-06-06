//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_map.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


namespace sim_mob
{

class CSV_Reader
{
private:
	const bool headerLineIncludedInFile;
	std::ifstream inputFileStream;
	std::vector<std::string> headerList;
	const char delimiter;

public:
	CSV_Reader(const std::string& fileName, char delim, bool headerLineIncluded);
	virtual ~CSV_Reader();

	void getNextRow(boost::unordered_map<std::string,std::string> rowOut);


};

} // end namespace sim_mob
