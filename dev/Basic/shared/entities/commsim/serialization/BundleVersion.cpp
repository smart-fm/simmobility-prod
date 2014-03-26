//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BundleVersion.hpp"

#include <stdexcept>
#include <sstream>
#include <iomanip>

std::string sim_mob::BundleParser::make_bundle_header(const BundleHeader& header)
{
	if (NEW_BUNDLES) {
		return make_bundle_header_v1(header);
	} else {
		return make_bundle_header_v0(header);
	}
}


sim_mob::BundleHeader sim_mob::BundleParser::read_bundle_header(const std::string& header)
{
	if (NEW_BUNDLES) {
		return read_bundle_header_v1(header);
	} else {
		return read_bundle_header_v0(header);
	}
}


std::string sim_mob::BundleParser::make_bundle_header_v1(const BundleHeader& header)
{
	//TODO: We'll need a more streamlined approach for this eventually.
	unsigned char res[8];
	res[0] = 1;
	res[1] = (unsigned char)header.messageCount;
	res[2] = (unsigned char)header.sendIdLen;
	res[3] = (unsigned char)header.destIdLen;

	res[4] = (unsigned char)((header.remLen>>24)&0xFF);
	res[5] = (unsigned char)((header.remLen>>16)&0xFF);
	res[6] = (unsigned char)((header.remLen>>8)&0xFF);
	res[7] = (unsigned char)((header.remLen)&0xFF);
	return std::string(reinterpret_cast<char*>(res), 8);
}


sim_mob::BundleHeader sim_mob::BundleParser::read_bundle_header_v1(const std::string& header)
{
	//Failsafe
	if (((unsigned char)header[0]) != 1) { throw std::runtime_error("Invalid header version."); }

	sim_mob::BundleHeader res;
	res.messageCount = (unsigned char)header[1];
	res.sendIdLen = (unsigned char)header[2];
	res.destIdLen = (unsigned char)header[3];

	//This might be overly verbose; I'm used to byte stream manipulation in Java.
	res.remLen = (((unsigned char)header[4])<<24) | (((unsigned char)header[5])<<16) | (((unsigned char)header[6])<<8) | ((unsigned char)header[7]);

	return res;
}




std::string sim_mob::BundleParser::make_bundle_header_v0(const BundleHeader& header)
{
	std::ostringstream header_stream;
	header_stream << std::setw(header_length) << std::hex << header.remLen;
	if (!header_stream || header_stream.str().size() != header_length) {
		return "";
	}
	return header_stream.str();
}


sim_mob::BundleHeader sim_mob::BundleParser::read_bundle_header_v0(const std::string& header)
{
	std::istringstream is(header);
	BundleHeader res;
	if (!(is >> std::hex >> res.remLen)) {
		res.remLen = 0;
	}
	return res;
}


