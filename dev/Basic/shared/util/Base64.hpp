//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

namespace sim_mob
{

/**
 * Class providing static methods to encode/decode base-64 streams.
 *
 * \author Seth N. Hetu
 *
 * Based on the documentation/code here:
 *   http://en.wikipedia.org/wiki/Base64
 *   http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64
 *
 * Modified to not generate \r\n.
 */
class Base64 {
public:
	///Encode a stream of bytes (represented as a String) into a string in the base64 format.
	static std::string encode(const std::string& bytes);

	///Take a base64-encoded string and decode it into a stream of bytes (represented as a string).
	static std::string decode(const std::string& text);

private:
	static const std::string Alphabet;

	//TEMP: Inefficient. Replace with a map later.
	static size_t RevLookup(char letter);
};


}


