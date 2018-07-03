//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

namespace sim_mob {
namespace simple_password {
void encryptionFunc(std::string &nString);
void decryptionFunc(std::string &nString);
void saveBinary(std::string value);
void loadBinary(std::string & value);
void save(std::string source);
std::string load(std::string dest);
}
}
