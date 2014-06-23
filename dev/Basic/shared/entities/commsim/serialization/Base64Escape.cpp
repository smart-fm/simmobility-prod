//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Base64Escape.hpp"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <map>

namespace {
const std::string Alphabet =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::map<char, unsigned int> RevLookup;
} //End un-named namespace


void sim_mob::Base64Escape::Init()
{
	RevLookup.clear();
	unsigned int i = 0;
	for (std::string::const_iterator it=Alphabet.begin(); it!=Alphabet.end(); it++) {
		RevLookup[*it] = i++;
	}
}


std::string sim_mob::Base64Escape::Encode(std::vector<std::string> data, char endline)
{
	if (RevLookup.empty()) {
		throw std::runtime_error("Can't Base64Escape decode; the lookup was not initialized.");
	}

	std::stringstream res;
	std::vector<unsigned int> inputBuff;
	unsigned int outputBuff[] = {0,0,0,0};
	int numEq = 0;
	for (size_t i=0; i<data.size(); i++) {
		//Make sure it ends with endline, unless it's the last line.
		if (endline!='\0' && data[i][data[i].size()-1] != endline) {
			if (i<data.size()-1) {
				data[i] += endline;
			}
		}

		//Now, add each character to our encoding stream.
		for (std::string::const_iterator it=data[i].begin(); it!=data[i].end(); it++) {
			inputBuff.push_back(*it);

			//Is this the VERY LAST character? If so, add equals signs if we don't line up entirely.
			if (i==data.size()-1 && (it+1)==data[i].end()) {
				numEq = 3 - inputBuff.size();
				for (int i=0; i<numEq; i++) {
					inputBuff.push_back('\0'); //All zeroes.
				}
			}

			//Do we have enough to generate an output quartet?
			if (inputBuff.size()==3) {
				outputBuff[0] = (inputBuff[0]>>2)&0x3F;
				outputBuff[1] = ((inputBuff[0]&0x3)<<4) | ((inputBuff[1]>>4)&0xF);
				outputBuff[2] = ((inputBuff[1]&0xF)<<2) | ((inputBuff[2]>>6)&0x3);
				outputBuff[3] = inputBuff[2]&0x3F;
				inputBuff.clear();

				//Now append.
				for (int j=0; j<4-numEq; j++) {
					if (outputBuff[j]>=Alphabet.size()) {
						throw std::runtime_error("Base64 internal error; exceeded size of alphabet.");
					}
					res <<Alphabet[outputBuff[j]];
				}

				//And the equals signs
				for (int j=0; j<numEq; j++) {
					res <<'=';
				}
			}
		}
	}

	return res.str();
}


void sim_mob::Base64Escape::Decode(std::vector<std::string>& res, const std::string& data, char split)
{
	if (RevLookup.empty()) {
		throw std::runtime_error("Can't Base64Escape decode; the lookup was not initialized.");
	}

	res.clear();
	bool esc = false; //Are we on an escape character?
	int eq = 0; //Number of equals signs encountered.
	std::stringstream curr;
	std::vector<unsigned int> inputBuff;
	char outputBuff[] = {0,0,0};
	for (std::string::const_iterator it=data.begin(); it!=data.end(); it++) {
		char c = *it;
		if (esc) {
			//Only a few letters are allowed.
			esc = false;
			if (c=='1') {
				c = ';';
			} else if (c=='2') {
				c = ':';
			} else if (c=='2') {
				c = '\n';
			} else if (c!='.') {
				throw std::runtime_error("Unknown base64 escape letter.");
			}
		} else if (c=='.') {
			esc = true;
			continue;
		}

		//Count the number of equals signs (necessary for decoding)
		if (c=='=') {
			eq++;
		}

		//Buffer c until we have 4 bytes.
		inputBuff.push_back(c);
		if (inputBuff.size()==4) {
			//Replace with their value.
			for (size_t i=0; i<inputBuff.size(); i++) {
				if (inputBuff[i]=='=') {
					inputBuff[i] = 0;
				} else {
					std::map<char, unsigned int>::const_iterator it=RevLookup.find(inputBuff[i]);
					if (it==RevLookup.end()) {
						throw std::runtime_error("Invalid base64 character.");
					}
					inputBuff[i] = it->second;
				}
			}

			//Now combine.
			outputBuff[0] = (inputBuff[0]<<2)  | (inputBuff[1]>>4);
			outputBuff[1] = ((inputBuff[1]&0xF)<<4) | (inputBuff[2]>>2);
			outputBuff[2] = ((inputBuff[2]&0x3)<<6) | (inputBuff[3]&0x3F);
			inputBuff.clear();

			//Finally, append
			for (size_t i=0; i<(3-eq); i++) {
				if (outputBuff[i]==split && split!='\0') {
					res.push_back(curr.str());
					curr.str("");
				} else {
					curr <<outputBuff[i];
				}
			}
		}
	}

	//Add the last line; it can be empty.
	res.push_back(curr.str());
}

