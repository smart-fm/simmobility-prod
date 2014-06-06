//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "CSVReader.hpp"

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace sim_mob;
using namespace boost;
using std::string;

namespace
{
	typedef tokenizer< escaped_list_separator<char> > Tokenizer;
}

sim_mob::CSV_Reader::CSV_Reader(const std::string& fileName, char delim, bool headerLineIncluded)
: headerLineIncludedInFile(headerLineIncluded)
{
	if(fileName.empty())
	{
		throw std::runtime_error("CSV_Reader cannot be constructed with empty file name");
	}

	//open the file
	inputFileStream(fileName.c_str());
	if (!inputFileStream.is_open())
	{
		throw std::runtime_error("CSV file cannot be opened");
	}

	//check delimiter
	if(!isalnum(delim))
	{
		delimiter = delim;
	}
	else
	{
		throw std::runtime_error("Alphanumeric delimiters are not supported");
	}

	//load header line if it is included in file
	if(headerLineIncluded)
	{
		string headerLine = string();
		if(getline(inputFileStream,headerLine))
		{
			Tokenizer tknzr(headerLine);
			for(Tokenizer::iterator tknIt=tknzr.begin(); tknIt!=tknzr.end(); tknIt++)
			{
				string header = *tknIt;
				trim(header);
				headerList.push_back(header);
			}
		}
	}
}

sim_mob::CSV_Reader::~CSV_Reader()
{
}

void sim_mob::CSV_Reader::getNextRow(boost::unordered_map<std::string, std::string> rowOut)
{

}




