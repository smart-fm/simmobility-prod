/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LuaModel.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 10, 2013, 11:21 AM
 */
#pragma once

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include "LuaLibrary.hpp"

namespace sim_mob {
    namespace lua {

        /**
         * Repreents a generic proxy and script loader
         * to interact with Lua scripts.
         */
        class LuaModel {
        public:
            LuaModel();
            LuaModel(const LuaModel& orig);
            virtual ~LuaModel();
            
            /**
             * Initializes lua loading libraries, mapping classes.
             */
            void Initialize();
            
            /**
             * Loads all scripts included on given directory. 
             * @param dirPath
             */ 
            void loadDirectory(const std::string& dirPath);
            
            /**
             * Loads the given file using the current lua state.
             * @param filePath
             */
            void loadFile(const std::string& filePath);
        protected:
            /**
             * Map C++ classes to Lua.
             */
             virtual void mapClasses()=0;
        protected:
            bool initialized;
            boost::shared_ptr<lua_State> state;
            std::list<std::string> files;
        };
    }
}

