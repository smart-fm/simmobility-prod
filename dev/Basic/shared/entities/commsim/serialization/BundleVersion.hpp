//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

namespace sim_mob {

///Whether to use the old bundle version or the new one. 
///NOTE: There should ONLY be one bundle format (v1). This flag exists during the transitional phase from v0 to v1,
///      but you should expect that it will be deleted soon after v1 is shown to be stable. At that point, we might
///      have slightly different message formats, but the bundle format should NOT change. 
const bool NEW_BUNDLES = true;

///Whether to prefer binary messages in v1 bundles (only applies to serialization).
///NOTE: Unlike NEW_BUNDLES, this flag will remain relevant after we switch to v1.
///      There will eventually be more fine-grained control over individual message serialization
///      (e.g., a way of "registering" serialization protocols), but for now we simply turn it "on" or "off".
const bool PREFER_BINARY_MESSAGES = false;

///The size of a fixed length header.
///Fortunately, both v0 and v1 headers are 8 bytes.
const unsigned int header_length = 8;


///A bundle header. Some values not available in v0.
struct BundleHeader {
	int sendIdLen; ///<Length of the "senderID" field. (v1 only)
	int destIdLen; ///<Length of the "destinationID" field. (v1 only)
	int messageCount; ///<Number of messages. (v1 only)
	unsigned int remLen; ///<Length of the remaining headers+data. (v0, v1)
	BundleHeader() : sendIdLen(0), destIdLen(0), messageCount(0), remLen(0) {}
};

///A varying-length header. Only used for v1 (follows the BundleHeader).
struct VaryHeader {
	std::string sendId; ///<SenderID for all messages in this bundle.
	std::string destId; ///<Destination ID for all messages in this bundle.
	std::vector<unsigned int> msgLengths; ///<Length of each message in this bundle, in order.
};


///Transitional class for reading bundle headers based on NEW_BUNDLES's value.
class BundleParser {
public:
	///Create a bundle header (v0 or v1, depending on whether NEW_BUNDLES has been set).
	///For v0, "header" only needs to contain remLen.
	///For v1, "header" should be valid. remLen should be set to the length of the remaining variable-length header and message data.
	static std::string make_bundle_header(const BundleHeader& header);

	///Read a bundle header (v0 or v1, depending on whether NEW_BUNDLES has been set).
	static BundleHeader read_bundle_header(const std::string& header);

private:
	///Create a "version 0" (old-style) header, consisting of the data's length as an 8-byte hex (text) string.
	static std::string make_bundle_header_v0(const BundleHeader& heade);

	///Read a "version 0" (old-style) header, returning the header struct (which only contains a valid length).
	static BundleHeader read_bundle_header_v0(const std::string& header);

	///Create a "version 1" (new-style) header.
	static std::string make_bundle_header_v1(const BundleHeader& heade);

	///Read a "version 1" (new-style) header.
	static BundleHeader read_bundle_header_v1(const std::string& header);
};



}

