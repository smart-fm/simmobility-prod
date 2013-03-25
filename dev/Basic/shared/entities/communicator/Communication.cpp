#include<fstream>
#include "Communication.hpp"
namespace sim_mob {

commResult NS3_Communication::FileBasedImpl::send(std::set<DATA_MSG_PTR>& values) {
	std::cout << "Inside FileBasedImpl::send\n";
	//todo, i couldn't find a way to declare text_oarchive without a parameterized constructor,
	//hence I need to declare it in every send operation.
	//similar problem goes to registration of type. it occurs every time send is called.
	//none of the above is a serious issue.
    std::ofstream ofs(sendFile.c_str(), std::ios::ate); //std::ios::ate : since the data from agents is written batch-by-batch


	if(!ofs.is_open()) {
	    std::cout << "NS3_Communication::FileBasedImpl::Send=> " << receiveFile << "  is empty\n";
	    return commResult(commResult::failure);
	}

    boost::archive::text_oarchive oa(ofs);
	//todo, see if there is any way to avoid repetition of this registration
//    for(std::set<DATA_MSG_PTR>::iterator it = values.begin(); it != values.end(); it++)
//    {
//    	(*it)->registerType(oa);
//    }
//    std::set<DATA_MSG_PTR>
    DATA_MSG_PTR value;
    BOOST_FOREACH(value, values)
    {
    	value->registerType(oa);
    	std::cout << "Serializing '" << value->str << "'" << std::endl;
    	oa & value;
    }
//    oa & value;
};
commResult NS3_Communication::FileBasedImpl::receive(std::set<DATA_MSG_PTR>& value){

	std::ifstream ifs(receiveFile.c_str(), std::ios::trunc); //so far, the assumption is

	if(!ifs.is_open()) {
	    std::cout << "NS3_Communication::FileBasedImpl::receive=> " << receiveFile << "  is empty\n";
	    return commResult(commResult::failure);
	}

	try {
		boost::archive::text_iarchive ia(ifs);
		//todo, see if there is any way to avoid repetition of this registration
	    for(std::set<DATA_MSG_PTR>::iterator it = value.begin(); it != value.end(); it++)
	    {
	    	(*it)->registerType(ia);
	    }
	    ia & value;
	}
	catch(boost::archive::archive_exception e){
		throw std::runtime_error(e.what());

	}


};

NS3_Communication::FileBasedImpl::FileBasedImpl(std::string sendFile_, std::string receiveFile_)
{
	sendFile = sendFile_;
	receiveFile = receiveFile_;
	//reset files
	std::ofstream ofs;
	ofs.open(sendFile_.c_str(), std::ios::trunc);
	ofs.close();
	ofs.open(receiveFile_.c_str(), std::ios::trunc);
	ofs.close();
}
NS3_Communication::NS3_Communication() { SR_Impl = new FileBasedImpl(); }
}
;
//namespace sim_mob
