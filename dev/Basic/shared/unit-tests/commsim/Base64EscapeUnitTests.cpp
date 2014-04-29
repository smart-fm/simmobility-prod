//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Base64EscapeUnitTests.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include "entities/commsim/serialization/Base64Escape.hpp"

using namespace sim_mob;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::Base64EscapeUnitTests);


namespace {
struct DecodeTest {
	std::string encoded;
	std::vector<std::string> decoded;

	//The "decoded" string should have newlines separating each line.
	DecodeTest(const std::string& encoded, const std::string& decoded) : encoded(encoded) {
		boost::split(this->decoded, decoded, boost::is_any_of("\n"));
	}
};

void make_tests(std::vector<DecodeTest>& tests) {
	//Wikipedia example.
	tests.push_back(DecodeTest(
		"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
		"IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
		"dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
		"dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
		"ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=",
		"Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the "
		"mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short "
		"vehemence of any carnal pleasure."
	));

	//More: examples of actual messages, if possible.

	//More: test escaping (even though we don't really use it much).

	//More: test newlines, line splitting, the empty line at the end.
}


void fail(const std::string& msg, const std::string& enc) {
	std::string res = msg + enc;
	CPPUNIT_FAIL(res.c_str());
}

void fail(const std::string& msg, const std::string& enc, size_t id) {
	std::stringstream res;
	res <<msg <<"(" <<id <<") " <<enc;
	CPPUNIT_FAIL(res.str().c_str());
}


} //End un-named namespace.


void unit_tests::Base64EscapeUnitTests::test_Base64Escape_normal_strings()
{
	//Init
	Base64Escape::Init();

	//Create our set of test strings.
	std::vector<DecodeTest> tests;
	make_tests(tests);

	//Now actually run these tests
	std::vector<std::string> res;
	for (std::vector<DecodeTest>::const_iterator it=tests.begin(); it!=tests.end(); it++) {
		Base64Escape::Decode(res, it->encoded, '\n');
		if (res.size() != it->decoded.size()) {
			fail("Decoded result set size mismatch: ", it->encoded);
		}

		//Check each string.
		for (size_t i=0; i<it->decoded.size(); i++) {
			if (it->decoded[i] != res[i]) {
				fail("Decoding doesn't match: ", it->encoded+"=>"+res[i]+":"+it->decoded[i], i);
			}
		}
	}
}





