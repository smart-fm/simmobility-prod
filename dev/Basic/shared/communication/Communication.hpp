#include <iostream>
namespace sim_mob
{

class dataCapsule
{
public:
	std::string data;
};

class commArguments
{
private:
	std::string fileName;
	dataCapsule data;
public:
	commArguments(std::string &fileName_,dataCapsule &data_):fileName(fileName_), data(data_){}
	commArguments(){}

	dataCapsule getData() const {
		return data;
	}

	void setData(dataCapsule data) {
		this->data = data;
	}

	std::string getFileName() const {
		return fileName;
	}

	void setFileName(std::string fileName) {
		this->fileName = fileName;
	}
};

class commResult
{
	enum result
	{
		success,
		failure,
		unknown
	};
private:
	result res;
public:
	commResult(result result_ = unknown):res(result_){};

};


template<class ARGS, class RET>
class Communication
{
public:
	virtual RET send(ARGS) = 0;
	virtual RET receive(ARGS) = 0;
};

class NS3_Communication: public Communication<commArguments, commResult> {
public:

	class FileBasedImpl: public Communication<commArguments, commResult> {
	public:
		commResult send(commArguments &args);
		commResult receive(commArguments &args);
	};

	commResult send(commArguments &args) {
		return SR_Impl->send(args);
	}
	commResult receive(commArguments &args) {
		return SR_Impl->receive(args);
	}

private:
	FileBasedImpl *SR_Impl; //Send & Receive implementation
};

}//namespace

