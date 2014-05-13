//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

namespace sim_mob {


/**
 * Handles decoding our Opaque message's base64escape format, and then re-encoding it for transmission to clients.
 */
class Base64Escape {
public:
	/**
	 * Call this once, in a thread-safe location.
	 */
	static void Init();

	/**
	 * Decode our input string into an array of output lines.
	 * If split is not 0, the output will be split into a new line at every occurrence of split.
	 */
	static void Decode(std::vector<std::string>& res, const std::string& data, char split);

	/**
	 * Encode an array of input lines into a single output string.
	 * If endline is not 0, each line will be checked to make sure it ends with that character (it may be appended).
	 */
	static std::string Encode(std::vector<std::string> data, char endline);
};

}
