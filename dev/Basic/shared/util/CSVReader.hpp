//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_map.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace sim_mob
{

/**
 * A simple CSV reader class.
 *
 * \author Harish Loganathan
 */
class CSV_Reader
{
private:
	/**flag to indicate whether the CSV file read by this reader includes a header line*/
	const bool headerLineIncludedInFile;
	/**input file stream for the csv being read*/
	std::ifstream inputFileStream;
	/**container for headers if header line is included in csv*/
	std::vector<std::string> headerList;
	/**escaped list separator specifying the delimiter, quote and escape characters*/
	boost::escaped_list_separator<char> escapedListSeparator;

public:
	/**
	 * constructor
	 * @param fileName string containing the full path of the input csv file
	 * @param headerLineIncluded boolean to specify whether header line is included in the csv
	 * @param delimChar Specifies the character used to separate fields in csv. Defaults to ','
	 * @param escapeChar Specifies the character to use for escape sequences. Defaults to the C style '\' (backslash).
	 * 			An example of when you might want to override the default value is when you have many fields
	 * 			which are Windows style filenames. Instead of escaping out each \ in the path, you can change the escape to something else.
	 * @param quoteChar Specifies the character used for the quote in csv. Defaults to '"'
	 */
	CSV_Reader(const std::string& fileName, bool headerLineIncluded, char delimChar=',', char escapeChar='\\', char quoteChar='\"');

	virtual ~CSV_Reader();

	const std::vector<std::string>& getHeaderList() const
	{
		return headerList;
	}

	/**
	 * Gets the next row of the csv file if it exists into a vector of strings
	 * After this function is executed, the caller must check if vector is empty.
	 * The vector will not be populated if there are no more rows in the file.
	 * @param rowOut the vector of strings to be populated with the fields of the row.
	 *
	 */
	void getNextRow(std::vector<std::string>& rowOut);

	/**
	 * Gets the next row of the csv file if it exists into an unordered map.
	 * This overload works only if headerLineIncludedInFile is true.
	 * A row is considered valid only if the number of fields in the row is equal to the number of fields in the header line.
	 * If the current row is invalid, this function skips the row and reads the next row if skipInvalidRows is set to true; an error is thrown otherwise.
	 * After this function is executed, the caller must check if unordered_map is empty.
	 * The map will not be populated if there are no more valid rows to be read in the file.
	 * @param rowOut unordered_map to be populated with the row's values. When this function is executed, for each <k,v> element in the map - v is the value
	 * 			of a field in the row that was read from the csv and k is the value of the field in the header line corresponding to that field.
	 * @param skipInvalidRows flag to indicate whether the algorithm can skip invalid rows.
	 */
	void getNextRow(boost::unordered_map<std::string,std::string>& rowOut, bool skipInvalidRows = true);
};

} // end namespace sim_mob
