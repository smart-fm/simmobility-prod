/*
 * PT_RouteChoiceLuaModel.hpp
 *
 *  Created on: 1 Apr, 2015
 *      Author: zhang
 */

#pragma once

#include "lua/LuaModel.hpp"
#include "Path.hpp"

namespace sim_mob{

class PT_RouteChoiceLuaModel : public lua::LuaModel  {
public:
	PT_RouteChoiceLuaModel();
	virtual ~PT_RouteChoiceLuaModel();

    /**
     * Inherited from LuaModel
     */
	void mapClasses();

    /**
     * make public transit route choice from lua scripts.
     *
     * return the index of best selected path in the path set
     */
	int MakePT_RouteChoice();

    /**
      * retrieve a singleton object
      * @return a pointer to PT_RouteChoiceModel .
      */
	static PT_RouteChoiceLuaModel* Instance();

	void SetPathSet(PT_PathSet* set){
		this->pathSet = set;
	}

	/**
	 * get the size of current path set.
	 *
	 * return the size of current choice set
	 */
	unsigned int GetSizeOfChoiceSet();

	/**
	 * interfaces for Lua script
	 * @param index is the index in the choice set
	 * @return the corresponding property value
	 */
	double total_in_vehicle_time(unsigned int index);
	double total_walk_time(unsigned int index);
	double total_wait_time(unsigned int index);
	double total_path_size(unsigned int index);
	int total_no_txf(unsigned int index);

private:
	PT_PathSet* publicTransitPathSet;
	static PT_RouteChoiceLuaModel* instance;
};

}
