//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace sim_mob
{
namespace simple_password
{
void encryptionFunc(std::string &nString)
{
	const int KEY = 23;                       // Key used for XOR operation
	int strLen = (nString.length());          // Grab string length for iterating thru string
	char *cString = (char*)(nString.c_str()); // Convert string to C string so we can actually iterate through it(std::string will treat it as one full object :()

	for (int i = 0; i < strLen; i++)          // time to iterate thru string and XOR each individual char.
	{
	 *(cString+i) = (*(cString+i) ^ KEY);     // ^ is the binary operator for XOR
	}
}

void decryptionFunc(std::string &nString)    // Time to undo what we did from above :D
{
	const int KEY = 23;
	int strLen = (nString.length());
	char *cString = (char*)(nString.c_str());

	for (int i = 0; i < strLen; i++)
	{
	 *(cString+i) = (*(cString+i) ^ KEY);
	}
}
void saveBinary(std::string value)
{
  std::ofstream file("shared/password/password");
  boost::archive::binary_oarchive oa(file);
  oa << value;
}


void loadBinary(std::string & value)
{
	try
	{
		std::ifstream file("shared/password/password");
		boost::archive::binary_iarchive ia(file);
		ia >> value;
	}
	catch(...)
	{

	}
}

void save(std::string source)
{

	encryptionFunc(source);
    saveBinary(source);
}

std::string load(std::string dest)
{

    loadBinary(dest);
    decryptionFunc(dest);
    return dest;
}

}//simple_password
}//sim_mob


//int main(int argc, char * argv[])
//{
//	std::ostringstream out;
//	out << argv[1];
//	std::string source = out.str();
//    std::string dest;
//    loadBinary(dest);
//    decryptionFunc(dest);
//    std::cout << dest << std::endl;
//
//
//}
