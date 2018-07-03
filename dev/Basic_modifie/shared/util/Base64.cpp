//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Base64.hpp"

#include <sstream>
#include <stdint.h>  //NOTE: <cstdint> is only part of C++11, strangely.
#include <stdexcept>

using namespace sim_mob;



//Inefficient lookup. Can use a map later.
size_t sim_mob::Base64::RevLookup(char letter) {
	for (size_t i=0; i<Alphabet.length(); i++) {
		if (Alphabet.at(i)==letter) {
			return i;
		}
	}
	throw std::runtime_error("Invalid character in base64 string.");
}



const std::string sim_mob::Base64::Alphabet("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");


std::string sim_mob::Base64::encode(const std::string& bytes)
{
	//Short-circuit
	if (bytes.empty()) {
		return "";
	}

	//Calculate padding
	size_t padding = (3-bytes.length()%3) % 3;

	//Establish padding in the source string.
	std::string src = bytes + std::string(padding, '\0');

	//Read through and encode each three 8-bit triplet.
	std::stringstream res;
	for (size_t i=0; i<src.length(); i+=3) {
		//Compute the combined 24-bit representation of this triplet.
		uint32_t triplet = (src.at(i)<<16) | (src.at(i+1)<<8) | (src.at(i+2));

		//Now split that into three components and append it.
		res <<Alphabet.at((triplet>>18) & 0x3F);
		res <<Alphabet.at((triplet>>12) & 0x3F);
		res <<Alphabet.at((triplet>>6)  & 0x3F);
		res <<Alphabet.at(triplet & 0x3F);
	}

	//Now, we just need to return the final string, substituting
	// for padding bytes where appropriate.
	return res.str().substr(0, res.str().length()-padding) + std::string(padding, '=');
}

std::string sim_mob::Base64::decode(const std::string& text)
{
	//Calculate the padding.
	std::string postfix = "";
	for (std::string::const_reverse_iterator it=text.rbegin(); it!=text.rend(); it++) {
		if ((*it) == '=') {
			postfix += "A"; //'A' will revert to '\0'
		}
	}

	//Undo the padding.
	std::string source = text.substr(0, text.length()-postfix.length()) + postfix;

	//Now revert, iterating in tetra-bytes this time.
	std::stringstream res;
	for (int i=0; i<source.length(); i+=4) {
		//Compute the tetra-integer.
		uint32_t tetra = (RevLookup(source.at(i))<<18) | (RevLookup(source.at(i+1))<<12)
						| (RevLookup(source.at(i+2))<<6) | RevLookup(source.at(i+3));

		//Split this integer back into its original three bytes.
		res <<static_cast<char>((tetra>>16)&0xFF);
		res <<static_cast<char>((tetra>>8)&0xFF);
		res <<static_cast<char>(tetra&0xFF);
	}

	//Finally, remove the padding.
	return res.str().substr(0, res.str().length() - postfix.length());
}









