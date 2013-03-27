#include "Communication.hpp"
namespace sim_mob {

commResult NS3_Communication::FileBasedImpl::send(DataContainer& values) {
	std::cout << "Inside FileBasedImpl::send\n";
	//todo, i couldn't find a way to declare text_oarchive without a parameterized constructor,
	//hence I need to declare it in every send operation.
	//similar problem goes to registration of type. it occurs every time send is called.
	//none of the above is a serious issue.
//    std::ofstream ofs(sendFile.c_str(), std::ios::app); //std::ios::app : since the data from agents is written batch-by-batch
	if(!ofs.is_open()) {
		ofs.open(sendFile.c_str());
		ofs.close();
		ofs.open(sendFile.c_str() , std::ios::app);
	}
	if(!ofs.is_open()) {
	    std::cout << "NS3_Communication::FileBasedImpl::Send=> " << sendFile << "  is empty\n";
	    return commResult(commResult::failure);
	}

    boost::archive::text_oarchive oa(ofs);

    DATA_MSG_PTR value;
    //todo this causes seg fault during split load process
    BOOST_FOREACH(value, values.get())
    {
    	//todo this causes seg fault during split load process
//    	value->registerType(oa); no need anymore
    	std::cout << "Serializing '" << value->str << "'" << std::endl;
//    	oa & value;
    }
    oa & values;

};
commResult NS3_Communication::FileBasedImpl::receive(DataContainer& value){
	std::cout << "Inside NS3_Communication::FileBasedImpl::receive[" << receiveFile << "]" << std::endl;
	if(!ifs.is_open())
	{
		ifs.open(receiveFile.c_str(), std::fstream::in);
	}
	if(!ifs.is_open()) {
	    std::cout << receiveFile << "  is empty." << std::endl;
	    return commResult(commResult::failure);
	}

	try {
		if(ifs.good())
		{
			boost::archive::text_iarchive ia(ifs);
			ia & value;
			for(std::vector<DATA_MSG_PTR>::iterator it = value.get().begin(); it != value.get().end(); it++)
			{
				std::cout << "de-Serializing to '" << (*it)->str << "'" << std::endl;
			}
		}

	}
	catch(boost::archive::archive_exception e){
		std::cout << e.what() << std::endl;

	}
	catch(std::bad_alloc& ba )
	{
		std::cerr << "Receivefile is not yet ready: " << ba.what() << std::endl;
	}


};
void NS3_Communication::FileBasedImpl::shortCircuit(std::string sendFile_ , std::string receiveFile_)
{
	std::ostringstream out("");
	out << "cp " + sendFile_ + " " + receiveFile_;
	std::system(out.str().c_str());
}
NS3_Communication::FileBasedImpl::FileBasedImpl(std::string sendFile_, std::string receiveFile_)
{
	sendFile = sendFile_;
	receiveFile = receiveFile_;
	//reset files
	ofs.open(sendFile_.c_str());
	ofs.close();
//	ifs.open(receiveFile_.c_str());
//	ifs.close();
	//now open them ina decent way
	ofs.open(sendFile_.c_str(), std::ios::app);
//	ifs.open(receiveFile_.c_str(), std::ifstream::in);
}
NS3_Communication::NS3_Communication() { SR_Impl = new FileBasedImpl(); }
}
;
//namespace sim_mob
