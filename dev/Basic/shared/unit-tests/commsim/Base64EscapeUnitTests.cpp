//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Base64EscapeUnitTests.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
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

std::string esc(const std::string& src) {
	std::stringstream res;
	for (size_t i=0; i<src.size(); i++) {
		unsigned int cI = ((unsigned int)src[i])&0xFF;
		if (cI<32 || cI>125) {
			res <<"{" <<cI <<"}";
		} else {
			res <<src[i];
		}
	}
	return res.str();
}


//Test a single item.
void test_item(const DecodeTest& test)
{
	std::vector<std::string> res;
	Base64Escape::Decode(res, test.encoded, '\n');
	if (res.size() != test.decoded.size()) {
		std::cout <<"Decode result set mismatch; expected: " <<test.decoded.size() <<", actual: " <<res.size() <<"\n";
		std::cout <<"...on string: " <<test.encoded <<"\n";
		CPPUNIT_FAIL("Decoded result set size mismatch; see console.");
	}

	//Check each string.
	for (size_t i=0; i<test.decoded.size(); i++) {
		if (test.decoded[i] != res[i]) {
			std::cout <<"Decoding item " <<i <<" failed to match; expected: \n" <<esc(test.decoded[i]) <<"\nactual: \n" <<esc(res[i]) <<"\n";
			std::cout <<"...on string: " <<test.encoded <<"\n";
			CPPUNIT_FAIL("Decoded string doesn't match; see console.");
		}
	}
}


} //End un-named namespace.


void unit_tests::Base64EscapeUnitTests::setUp()
{
	//Init
	Base64Escape::Init();
}


void unit_tests::Base64EscapeUnitTests::test_Base64Escape_1()
{
	//Wikipedia example.
	DecodeTest test(
		"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
		"IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
		"dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
		"dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
		"ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=",
		"Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the "
		"mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short "
		"vehemence of any carnal pleasure."
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_2()
{
	DecodeTest test("", "");
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_3()
{
	DecodeTest test("YQ==", "a");
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_4()
{
	DecodeTest test("YWJjZGVm", "abcdef");
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_5()
{
	DecodeTest test("YWJjZGVmZw==", "abcdefg");
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_6()
{
	DecodeTest test("YWJjZGVmZ2g=", "abcdefgh");
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_7()
{
	DecodeTest test("YWJjZGVmZ2hp", "abcdefghi");
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_8()
{
	DecodeTest test("UQpFCkQ=", "Q\nE\nD");
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_9()
{
	DecodeTest test("UQpFCkQK", "Q\nE\nD\n");
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_1()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAAAA"
		"AAAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAADbGF0"
		"SQAGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZXJBbm5v"
		"dW5jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25xAH4AAUwA"
		"CXNpZ25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA10b2tlbnNP"
		"ZmZlcmVkdAAPTGphdmEvdXRpbC9TZXQ7eHC/gAAA/////wAAAAAAAAAAAAAAAAAAAAC/8AAAAAAA"
		"AAAAAAC/8AAAAAAAAAAAAAAAAAAAv4AAAAAAAUWvvX+8AAAAAAB0AABwcHQACW15LXNvdXJjZXBweA==",
		std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00"""""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataAct"
		"ivityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lngJ"""
		"\x00""""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04""ty"
		"peL""\x00""""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01""L"
		"\x00""""\x09""signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L""\x00"
		"""\x0b""tokenStringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/Set;xp"
		"\xbf""""\x80""""\x00""""\x00""""\xff""""\xff""""\xff""""\xff""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\xbf""""\xf0""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\xbf""""\xf0""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\xbf""""\x80""""\x00""""\x00""""\x00""""\x00""""\x01""E""\xaf"
		"""\xbd""""\x7f""""\xbc""""\x00""""\x00""""\x00""""\x00""""\x00""t""\x00""""\x00""ppt""\x00""""\x09""my-sourceppx", 400)
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_2()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAAAA"
		"AAAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAADbGF0"
		"SQAGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZXJBbm5v"
		"dW5jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25xAH4AAUwA"
		"CXNpZ25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA10b2tlbnNP"
		"ZmZlcmVkdAAPTGphdmEvdXRpbC9TZXQ7eHAAAAAA/////wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUWvvX/EAAAAAAB0AABwcHQACW15LXNvdXJjZXBweA==",
		std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataActi"
		"vityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lngJ""\x00"
		"""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04""typeL""\x00"
		"""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x09"
		"signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0b""token"
		"Stringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/Set;xp""\x00""""\x00"""
		"\x00""""\x00""""\xff""""\xff""""\xff""""\xff""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x01""E""\xaf""""\xbd""""\x7f"
		"""\xc4""""\x00""""\x00""""\x00""""\x00""""\x00""t""\x00""""\x00""ppt""\x00""""\x09""my-sourceppx", 400)
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_3()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAAAAA"
		"AAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAADbGF0SQ"
		"AGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZXJBbm5vdW5"
		"jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25xAH4AAUwACXNp"
		"Z25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA10b2tlbnNPZmZlc"
		"mVkdAAPTGphdmEvdXRpbC9TZXQ7eHAAAAAA/////wAAAAAAAAAAAAAAAAAAAAA/8BR64UeuFAAAAA"
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUWvvX/LAAAAAAB0AABwcHQACW15LXNvdXJjZXBweA==",
		std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataA"
		"ctivityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lng"
		"J""\x00""""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04"
		"typeL""\x00""""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01"
		"L""\x00""""\x09""signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L"
		"\x00""""\x0b""tokenStringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/"
		"Set;xp""\x00""""\x00""""\x00""""\x00""""\xff""""\xff""""\xff""""\xff""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""?""\xf0""""\x14""z""\xe1""G""\xae"""
		"\x14""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x01""E""\xaf""""\xbd"
		"""\x7f""""\xcb""""\x00""""\x00""""\x00""""\x00""""\x00""t""\x00""""\x00""ppt""\x00""""\x09""my-sourceppx", 400)
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_4()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAAAAA"
		"AAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAADbGF0SQ"
		"AGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZXJBbm5vdW5"
		"jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25xAH4AAUwACXNp"
		"Z25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA10b2tlbnNPZmZlc"
		"mVkdAAPTGphdmEvdXRpbC9TZXQ7eHAAAAAA/////wAAAAAAAAAAAAAAAAAAAAA/8BR64UeuFAAAAA"
		"BAUlCj1wo9cQAAAAAAAAAAAAAAAAAAAUWvvX/PAAAAAAB0AABwcHQACW15LXNvdXJjZXBweA==",
		std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataAc"
		"tivityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lngJ"
		"\x00""""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04""ty"
		"peL""\x00""""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01""L"
		"\x00""""\x09""signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L""\x00"
		"""\x0b""tokenStringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/Set;xp"
		"\x00""""\x00""""\x00""""\x00""""\xff""""\xff""""\xff""""\xff""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""?""\xf0""""\x14""z""\xe1""G""\xae""""\x14"""
		"\x00""""\x00""""\x00""""\x00""@RP""\xa3""""\xd7""\n=q""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x01""E""\xaf""""\xbd""""\x7f""""\xcf""""\x00""""\x00""""\x00""""\x00"
		"""\x00""t""\x00""""\x00""ppt""\x00""""\x09""my-sourceppx", 400)
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_5()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAAAAA"
		"AAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAADbGF0SQ"
		"AGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZXJBbm5vdW5"
		"jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25xAH4AAUwACXNp"
		"Z25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA10b2tlbnNPZmZlc"
		"mVkdAAPTGphdmEvdXRpbC9TZXQ7eHAAAAAA/////wAAAAAAAAAAAAAAAAAAAAA/8BR64UeuFAAAAA"
		"BAUlCj1wo9cQAAAAAAAAAAQg5mZgAAAUWvvX/SAAAAAAB0AABwcHQACW15LXNvdXJjZXBweA==",
		std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataAc"
		"tivityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lngJ"
		"\x00""""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04""t"
		"ypeL""\x00""""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01""L"
		"\x00""""\x09""signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L""\x00"
		"""\x0b""tokenStringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/Set;xp"
		"\x00""""\x00""""\x00""""\x00""""\xff""""\xff""""\xff""""\xff""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""?""\xf0""""\x14""z""\xe1""G""\xae""""\x14"""
		"\x00""""\x00""""\x00""""\x00""@RP""\xa3""""\xd7""\n=q""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"B""\x0e""ff""\x00""""\x00""""\x01""E""\xaf""""\xbd""""\x7f""""\xd2""""\x00""""\x00""""\x00""""\x00""""\x00""t""\x00"
		"""\x00""ppt""\x00""""\x09""my-sourceppx", 400)
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_6()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAAAAAAAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAADbGF0SQAGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZXJBbm5vdW5jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25xAH4AAUwACXNpZ25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA10b2tlbnNPZmZlcmVkdAAPTGphdmEvdXRpbC9TZXQ7eHAAAAAAAAAAZAAAAAAAAAAAAAAAAAAAAAA/8BR64UeuFAAAAABAUlCj1wo9cQAAAAAAAAAAQg5mZgAAAUWvvX/SAAAAAAB0AABwcHQACW15LXNvdXJjZXBweA==", std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataActivityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lngJ""\x00""""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04""typeL""\x00""""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x09""signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0b""tokenStringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/Set;xp""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""d""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""?""\xf0""""\x14""z""\xe1""G""\xae""""\x14""""\x00""""\x00""""\x00""""\x00""@RP""\xa3""""\xd7""\n=q""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""B""\x0e""ff""\x00""""\x00""""\x01""E""\xaf""""\xbd""""\x7f""""\xd2""""\x00""""\x00""""\x00""""\x00""""\x00""t""\x00""""\x00""ppt""\x00""""\x09""my-sourceppx", 400)
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_7()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAA"
		"AAAAAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAAD"
		"bGF0SQAGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZX"
		"JBbm5vdW5jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25x"
		"AH4AAUwACXNpZ25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA"
		"10b2tlbnNPZmZlcmVkdAAPTGphdmEvdXRpbC9TZXQ7eHAAAAAAAAAAZAAAAAAAAAAAAAAAAAAA"
		"AAA/8BR64UeuFAAAAABAUlCj1wo9cQAAAAAAAAAAQg5mZgAAAUWvvX/SAAAAAAB0AABwcHQACW"
		"15LXNvdXJjZXBzcgARamF2YS51dGlsLkhhc2hTZXS6RIWVlri3NAMAAHhwdwwAAAACP0AAAAAA"
		"AAB4eA==",
		std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataAc"
		"tivityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lngJ"
		"\x00""""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04""t"
		"ypeL""\x00""""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01""L"
		"\x00""""\x09""signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L""\x00"
		"""\x0b""tokenStringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/Set;xp"
		"\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""d""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""?""\xf0""""\x14""z""\xe1""G""\xae""""\x14""""\x00"""
		"\x00""""\x00""""\x00""@RP""\xa3""""\xd7""\n=q""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""B""\x0e"
		"ff""\x00""""\x00""""\x01""E""\xaf""""\xbd""""\x7f""""\xd2""""\x00""""\x00""""\x00""""\x00""""\x00""t""\x00""""\x00""ppt"
		"\x00""""\x09""my-sourcepsr""\x00""""\x11""java.util.HashSet""\xba""D""\x85""""\x95""""\x96""""\xb8""""\xb7""4""\x03"""
		"\x00""""\x00""xpw""\x0c""""\x00""""\x00""""\x00""""\x02""?@""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""xx", 448)
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_8()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAA"
		"AAAAAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAAD"
		"bGF0SQAGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZX"
		"JBbm5vdW5jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25x"
		"AH4AAUwACXNpZ25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA"
		"10b2tlbnNPZmZlcmVkdAAPTGphdmEvdXRpbC9TZXQ7eHAAAAAAAAAAZAAAAAAAAAAAAAAAAAAA"
		"AAA/8BR64UeuFAAAAABAUlCj1wo9cQAAAAAAAAAAQg5mZgAAAUWvvX/SAAAAAAB0AABwcHQACW"
		"15LXNvdXJjZXBzcgARamF2YS51dGlsLkhhc2hTZXS6RIWVlri3NAMAAHhwdwwAAAAEP0AAAAAA"
		"AAF0AAJoaXh4",
		std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataA"
		"ctivityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lngJ"
		"\x00""""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04"
		"typeL""\x00""""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01"
		"L""\x00""""\x09""signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L"
		"\x00""""\x0b""tokenStringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/Set"
		";xp""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""d""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""?""\xf0""""\x14""z""\xe1""G""\xae""""\x14""""\x00"
		"""\x00""""\x00""""\x00""@RP""\xa3""""\xd7""\n=q""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""B""\x0e"
		"ff""\x00""""\x00""""\x01""E""\xaf""""\xbd""""\x7f""""\xd2""""\x00""""\x00""""\x00""""\x00""""\x00""t""\x00""""\x00""ppt"
		"\x00""""\x09""my-sourcepsr""\x00""""\x11""java.util.HashSet""\xba""D""\x85""""\x95""""\x96""""\xb8""""\xb7""4""\x03"""
		"\x00""""\x00""xpw""\x0c""""\x00""""\x00""""\x00""""\x04""?@""\x00""""\x00""""\x00""""\x00""""\x00""""\x01""t""\x00"""
		"\x02""hixx", 453)
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_9()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAA"
		"AAAAAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAAD"
		"bGF0SQAGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZX"
		"JBbm5vdW5jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25x"
		"AH4AAUwACXNpZ25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA"
		"10b2tlbnNPZmZlcmVkdAAPTGphdmEvdXRpbC9TZXQ7eHAAAAAAAAAAZAAAAAAAAAAAAAAAAAAA"
		"AAA/8BR64UeuFAAAAABAUlCj1wo9cQAAAAAAAAAAQg5mZgAAAUWvvX/SAAAAAAB0AABwcHQACW"
		"15LXNvdXJjZXBzcgARamF2YS51dGlsLkhhc2hTZXS6RIWVlri3NAMAAHhwdwwAAAAEP0AAAAAA"
		"AAJ0AAV0aGVyZXQAAmhpeHg=",
		std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataAc"
		"tivityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lngJ"
		"\x00""""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04""t"
		"ypeL""\x00""""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01""L"
		"\x00""""\x09""signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L""\x00"
		"""\x0b""tokenStringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/Set;xp"
		"\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""d""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""?""\xf0""""\x14""z""\xe1""G""\xae""""\x14""""\x00"""
		"\x00""""\x00""""\x00""@RP""\xa3""""\xd7""\n=q""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""B""\x0e"
		"ff""\x00""""\x00""""\x01""E""\xaf""""\xbd""""\x7f""""\xd2""""\x00""""\x00""""\x00""""\x00""""\x00""t""\x00""""\x00""ppt"
		"\x00""""\x09""my-sourcepsr""\x00""""\x11""java.util.HashSet""\xba""D""\x85""""\x95""""\x96""""\xb8""""\xb7""4""\x03"""
		"\x00""""\x00""xpw""\x0c""""\x00""""\x00""""\x00""""\x04""?@""\x00""""\x00""""\x00""""\x00""""\x00""""\x02""t""\x00"""
		"\x05""theret""\x00""""\x02""hixx", 461)
	);
	test_item(test);
}

void unit_tests::Base64EscapeUnitTests::test_Base64Escape_roadrunner_10()
{
	DecodeTest test(
		"rO0ABXNyAC1lZHUubWl0LmNzYWlsLmphc29uZ2FvLnJvYWRydW5uZXIuQWRob2NQYWNrZXQAAA"
		"AAAAAAygMAEkYAB2JlYXJpbmdJAAxkYXRhQWN0aXZpdHlKAAdleHBpcmVzSgAGaXNzdWVkRAAD"
		"bGF0SQAGbGVuZ3RoRAADbG5nSgAFbm9uY2VGAAVzcGVlZEoACXRpbWVzdGFtcFoAD3RyaWdnZX"
		"JBbm5vdW5jZUkABHR5cGVMAAZkZXN0UlJ0ABJMamF2YS9sYW5nL1N0cmluZztMAAZyZWdpb25x"
		"AH4AAUwACXNpZ25hdHVyZXEAfgABTAAFc3JjUlJxAH4AAUwAC3Rva2VuU3RyaW5ncQB+AAFMAA"
		"10b2tlbnNPZmZlcmVkdAAPTGphdmEvdXRpbC9TZXQ7eHAAAAAAAAAAZAAAAAAAAAAAAAAAAAAA"
		"AAA/8BR64UeuFAAAAABAUlCj1wo9cQAAAAAAAAAAQg5mZgAAAUWvvX/SAAAAAAB0AABwcHQACW"
		"15LXNvdXJjZXBzcgARamF2YS51dGlsLkhhc2hTZXS6RIWVlri3NAMAAHhwdwwAAAAEP0AAAAAA"
		"AAJ0AAV0aGVyZXQAAmhpeHg=",
		std::string("""\xac""""\xed""""\x00""""\x05""sr""\x00""-edu.mit.csail.jasongao.roadrunner.AdhocPacket""\x00""""\x00"""
		"\x00""""\x00""""\x00""""\x00""""\x00""""\xca""""\x03""""\x00""""\x12""F""\x00""""\x07""bearingI""\x00""""\x0c""dataA"
		"ctivityJ""\x00""""\x07""expiresJ""\x00""""\x06""issuedD""\x00""""\x03""latI""\x00""""\x06""lengthD""\x00""""\x03""lngJ"
		"\x00""""\x05""nonceF""\x00""""\x05""speedJ""\x00""""\x09""timestampZ""\x00""""\x0f""triggerAnnounceI""\x00""""\x04""ty"
		"peL""\x00""""\x06""destRRt""\x00""""\x12""Ljava/lang/String;L""\x00""""\x06""regionq""\x00""""\x7e""""\x00""""\x01""L"
		"\x00""""\x09""signatureq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x05""srcRRq""\x00""""\x7e""""\x00""""\x01""L""\x00"
		"""\x0b""tokenStringq""\x00""""\x7e""""\x00""""\x01""L""\x00""""\x0d""tokensOfferedt""\x00""""\x0f""Ljava/util/Set;xp"
		"\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""d""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00"
		"""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""?""\xf0""""\x14""z""\xe1""G""\xae""""\x14""""\x00"""
		"\x00""""\x00""""\x00""@RP""\xa3""""\xd7""\n=q""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""B""\x0e"
		"ff""\x00""""\x00""""\x01""E""\xaf""""\xbd""""\x7f""""\xd2""""\x00""""\x00""""\x00""""\x00""""\x00""t""\x00""""\x00""ppt"
		"\x00""""\x09""my-sourcepsr""\x00""""\x11""java.util.HashSet""\xba""D""\x85""""\x95""""\x96""""\xb8""""\xb7""4""\x03"""
		"\x00""""\x00""xpw""\x0c""""\x00""""\x00""""\x00""""\x04""?@""\x00""""\x00""""\x00""""\x00""""\x00""""\x02""t""\x00"""
		"\x05""theret""\x00""""\x02""hixx", 461)
	);
	test_item(test);
}
