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
