//#include <cstddef> // NULL
//#include <iomanip>
//#include <iostream>
//#include <fstream>
#include <string>
//#include <typeinfo>

#include "Communication.hpp"
//class FileBasedCommunication : public Communication<commArguments,retutrnValue>
//{
//	retutrnValue send(commArguments args)
//	{
//
//
//	}
//	retutrnValue receive(commArguments args)
//	{
//
//	}
//
//
//};
///
namespace sim_mob {

commResult NS3_Communication::FileBasedImpl::send(std::set<DATA_MSG_PTR>& value) {
	//todo, i couldn't find a way to declare text_oarchive without a parameterized constructor,
	//hence I need to declare it in every send operation.
	//similar problem goes to registration of type. it occurs every time send is called.
	//none of the above is a serious issue.
    std::ofstream ofs(sendFile.c_str());
    boost::archive::text_oarchive oa(ofs);
	//todo, see if there is any way to avoid repetition of this registration
    for(std::set<DATA_MSG_PTR>::iterator it = value.begin(); it != value.end(); it++)
    {
    	(*it)->registerType(oa);
    }
    oa & value;
};
commResult NS3_Communication::FileBasedImpl::receive(std::set<DATA_MSG_PTR>& value){

    std::ifstream ifs(receiveFile.c_str());
    boost::archive::text_iarchive ia(ifs);
	//todo, see if there is any way to avoid repetition of this registration
    for(std::set<DATA_MSG_PTR>::iterator it = value.begin(); it != value.end(); it++)
    {
    	(*it)->registerType(ia);
    }
    ia & value;
};

}
;
//namespace sim_mob
