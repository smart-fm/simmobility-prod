/*
 * PT_RouteChoiceLuaModel.cpp
 *
 *  Created on: 1 Apr, 2015
 *      Author: zhang
 */

#include "PT_RouteChoiceLuaModel.hpp"
#include "util/LangHelpers.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/foreach.hpp"
#include "boost/filesystem.hpp"

using namespace luabridge;

namespace sim_mob{

PT_RouteChoiceLuaModel* PT_RouteChoiceLuaModel::instance=nullptr;

PT_RouteChoiceLuaModel::PT_RouteChoiceLuaModel() : publicTransitPathSet(nullptr) {
	// TODO Auto-generated constructor stub

}

PT_RouteChoiceLuaModel::~PT_RouteChoiceLuaModel() {
	// TODO Auto-generated destructor stub
}

PT_RouteChoiceLuaModel* PT_RouteChoiceLuaModel::Instance(){
	if(instance==nullptr){
		instance = new PT_RouteChoiceLuaModel();
	}
	return instance;
}

unsigned int PT_RouteChoiceLuaModel::GetSizeOfChoiceSet(){
	unsigned int size = 0;
	if(publicTransitPathSet){
		size = publicTransitPathSet->pathSet.size();
	}
	return size;
}

double PT_RouteChoiceLuaModel::total_in_vehicle_time(unsigned int index){
	double ret=0.0;
	unsigned int sizeOfChoiceSet = GetSizeOfChoiceSet();
	if(publicTransitPathSet && index<=sizeOfChoiceSet && index>0){
		std::set<PT_Path,cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index-1);
		ret = it->getTotalInVehicleTravelTimeSecs();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::total_walk_time(unsigned int index){
	double ret=0.0;
	unsigned int sizeOfChoiceSet = GetSizeOfChoiceSet();
	if(publicTransitPathSet && index<=sizeOfChoiceSet && index>0){
		std::set<PT_Path,cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index-1);
		ret = it->getTotalWalkingTimeSecs();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::total_wait_time(unsigned int index){
	double ret=0.0;
	unsigned int sizeOfChoiceSet = GetSizeOfChoiceSet();
	if(publicTransitPathSet && index<=sizeOfChoiceSet && index>0){
		std::set<PT_Path,cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index-1);
		ret = it->getTotalWaitingTimeSecs();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::total_path_size(unsigned int index){
	double ret=0.0;
	unsigned int sizeOfChoiceSet = GetSizeOfChoiceSet();
	if(publicTransitPathSet && index<=sizeOfChoiceSet && index>0){
		std::set<PT_Path,cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index-1);
		ret = it->getPathSize();
	}
	return ret;
}

int PT_RouteChoiceLuaModel::total_no_txf(unsigned int index){
	int ret=0.0;
	unsigned int sizeOfChoiceSet = GetSizeOfChoiceSet();
	if(publicTransitPathSet && index<=sizeOfChoiceSet && index>0){
		std::set<PT_Path,cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index-1);
		ret = it->getTotalNumberOfTransfers();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::total_cost(unsigned int index){
	double ret=0.0;
	unsigned int sizeOfChoiceSet = GetSizeOfChoiceSet();
	if(publicTransitPathSet && index<=sizeOfChoiceSet && index>0){
		std::set<PT_Path,cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index-1);
		ret = it->getTotalCost();
	}
	return ret;
}


int PT_RouteChoiceLuaModel::MakePT_RouteChoice()
{
	unsigned int sizeofChoiceSet = GetSizeOfChoiceSet();
    LuaRef funcRef = getGlobal(state.get(), "choose_PT_path");
    LuaRef retVal = funcRef(this, sizeofChoiceSet);
    int ret=-1;
    if (retVal.isNumber()) {
        ret = retVal.cast<int>();
    }
    return ret;
}

void PT_RouteChoiceLuaModel::mapClasses() {
    getGlobalNamespace(state.get())
            .beginClass <PT_RouteChoiceLuaModel> ("PT_RouteChoiceLuaModel")
            .addFunction("total_in_vehicle_time", &PT_RouteChoiceLuaModel::total_in_vehicle_time)
            .addFunction("total_walk_time", &PT_RouteChoiceLuaModel::total_walk_time)
			.addFunction("total_wait_time", &PT_RouteChoiceLuaModel::total_wait_time)
			.addFunction("total_path_size", &PT_RouteChoiceLuaModel::total_path_size)
			.addFunction("total_no_txf", &PT_RouteChoiceLuaModel::total_no_txf)
			.addFunction("total_cost", &PT_RouteChoiceLuaModel::total_cost)
            .endClass();
}

const boost::shared_ptr<soci::session> & sim_mob::PT_RouteChoiceLuaModel::getSession(){
	boost::upgrade_lock<boost::shared_mutex> lock(cnnRepoMutex);
	std::map<boost::thread::id, boost::shared_ptr<soci::session> >::iterator it;
	it = cnnRepo.find(boost::this_thread::get_id());
	if (it == cnnRepo.end()) {
		std::string dbStr(
				ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(
						false));
		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
			boost::shared_ptr<soci::session> t(
					new soci::session(soci::postgresql, dbStr));
			it =
					cnnRepo.insert(
							std::make_pair(boost::this_thread::get_id(), t)).first;
		}
	}
	return it->second;
}

PT_PathSet PT_RouteChoiceLuaModel::LoadPT_PathSet(const std::string& original, const std::string& dest)
{
	PT_PathSet pathSet;
	std::string pathSetId = original+","+dest;
	aimsun::Loader::LoadPT_ChoiceSetFrmDB(*getSession(),pathSetId, pathSet);
	return pathSet;
}

}
