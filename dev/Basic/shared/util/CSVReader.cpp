//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "CSVReader.hpp"

#include <boost/algorithm/string/trim.hpp>

using namespace sim_mob;
using namespace boost;
using std::string;

namespace
{
	typedef tokenizer< escaped_list_separator<char> > Tokenizer;
}

sim_mob::CSV_Reader::CSV_Reader(const std::string& fileName, bool headerLineIncluded, char delimChar, char escapeChar, char quoteChar)
: headerLineIncludedInFile(headerLineIncluded), escapedListSeparator(escapeChar, delimChar, quoteChar)
{
	if(fileName.empty())
	{
		throw std::runtime_error("CSV_Reader cannot be constructed with empty file name");
	}

	//open the file
	inputFileStream.open(fileName.c_str());
	if (!inputFileStream.is_open())
	{
		throw std::runtime_error("CSV file cannot be opened");
	}

	//load header line if it is included in file
	if(headerLineIncludedInFile)
	{
		string headerLine = string();
		if(getline(inputFileStream,headerLine))
		{
			Tokenizer tknzr(headerLine, escapedListSeparator);
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
	inputFileStream.close();
}

void sim_mob::CSV_Reader::getNextRow(boost::unordered_map<std::string, std::string>& rowOut, bool skipInvalidRows)
{
	if(!headerLineIncludedInFile)
	{
		throw std::runtime_error("Cannot build map out of row. Header line was not included in csv.");
	}
	string row = string();
	bool invalidRow = true;
	while (invalidRow && getline(inputFileStream,row))
	{
		Tokenizer tknzr(row, escapedListSeparator);
		size_t itNo = 0;
		invalidRow = false;
		for(Tokenizer::iterator tknIt=tknzr.begin(); tknIt!=tknzr.end(); tknIt++)
		{
			string item = *tknIt;
			trim(item);
			if(itNo < headerList.size())
			{
				rowOut[headerList[itNo]] = item;
			}
			itNo++;
		}
		if(itNo != headerList.size()) { invalidRow = true; }
		if(!invalidRow)
		{
			// One row was read successfully.
			break;
		}
		else
		{
			//row was not having the correct number of items
			rowOut.clear();
			if(!skipInvalidRows)
			{
				std::stringstream errStream;
				errStream << "getNextRow(): row has " << itNo << " fields. " << headerList.size() << " fields were expected.\n" << row << "\n";
				throw std::runtime_error(errStream.str());
			}
		}
	}
}

void sim_mob::CSV_Reader::getNextRow(std::vector<std::string>& rowOut)
{
	string row = string();
	if (getline(inputFileStream,row))
	{
		Tokenizer tknzr(row, escapedListSeparator);
		for(Tokenizer::iterator tknIt=tknzr.begin(); tknIt!=tknzr.end(); tknIt++)
		{
			string item = *tknIt;
			trim(item);
			rowOut.push_back(item);
		}
	}
}
