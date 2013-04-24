#include "../../../Communication_Base.hpp"
namespace sim_mob
{
#define sendFile_temp "./send.txt"
#define receiveFile_temp "./receive.txt"


//begin nested class FileBasedImpl
class FileBasedImpl: public Communication<DataContainer&, commResult> {
    std::string sendFile;
    std::string receiveFile;
    std::ofstream ofs;
    std::ifstream ifs;
public:
    FileBasedImpl(std::string sendFile_ = sendFile_temp, std::string receiveFile_ = sendFile_temp);//for test purpose, send & receive are same
	commResult send(DataContainer& value);
	commResult receive(DataContainer&value);
	void shortCircuit(std::string sendFile_ = sendFile_temp, std::string receiveFile_ = sendFile_temp);
};
}
