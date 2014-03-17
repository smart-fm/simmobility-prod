/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Model.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 21, 2013, 4:24 PM
 */
#pragma once
#include <map>
#include <vector>
#include <string>
#include "workers/WorkGroup.hpp"
#include "entities/Agent.hpp"
#include "util/Utils.hpp"

namespace sim_mob {
    namespace long_term {

        /**
         * Represents a generic model that runs within a given workgroup.
         */
        class Model {
        public:

            struct MetadataEntry {
            public:
                std::string getKey() const {return key;}
                std::string getValue() const {return value;}
                
            private:
                friend class Model;
                std::string key;
                std::string value;
            };
            
            typedef std::vector<MetadataEntry> Metadata;
        
        public:
            Model(const std::string& name, WorkGroup& workGroup);
            virtual ~Model();
            
            /**
             * Starts the model. 
             * This method calls the abstract method startImpl.
             */
            void start();
            
            /**
             * Stops the model if it is running. 
             * This method calls the abstract method stopImpl.
             */
            void stop();
            
            /**
             * Tells if the model is running or not.
             * @return true is start method was called with success,
             *         false otherwise
             */
            bool isRunning() const;
             
             /**
              * Gets the time that the start method took to finish.
              * @return time.
              */
            double getStartTime() const;
            
            /**
             * Gets the time that the stop method took to finish.
             * @return 
             */
            double getStopTime() const;
            
            /**
             * Gets the module name.
             * @return module name.
             */
            const std::string& getName() const;
            
            /**
             * Gets the relevant information (metadata) about the model 
             * to print out on the output report.
             * 
             * Please **do not use** this function to exchange data for other 
             * purposes.
             * 
             * @return 
             */
            const Metadata& getMetadata() const;
            
        protected:
            /**
             * Abstract method to be implemented by Model specializations.
             * You should load data from data sources, create agents
             * and fulfill the workgroup inside this method.  
             */
            virtual void startImpl() = 0;
            
            /**
             * Abstract method to be implemented by Model specializations to 
             * finalize the workgroup. 
             * 
             * NOTE: do not DELETE the agents inside this method.
             * Agents are deleted automatically in the destructor.
             */
            virtual void stopImpl() = 0;
        
            /**
             * Adds metadata to the model to be printed in the end of the simulation.
             * @param varName variable name.
             * @param value of the variable.
             */
            template<typename T>
            void addMetadata(const std::string& varName, const T& value) {
                MetadataMap::iterator itMap = metadataMapping.find(varName);
                if (itMap != metadataMapping.end()){
                    for (Metadata::iterator it = metadata.begin();
                            it != metadata.end(); it++) {
                        if (it->key == varName){
                            it->value = Utils::toStr(value);
                        }
                    }
                } else {
                    MetadataEntry entry;
                    entry.key = varName;
                    entry.value = Utils::toStr(value);
                    metadata.push_back(entry);
                    metadataMapping.insert(std::make_pair(entry.getKey(), 
                            entry.getValue()));
                }
            }
        protected:
            WorkGroup& workGroup;
            std::vector<Agent*> agents;
        
        private:
            typedef std::map<std::string, std::string> MetadataMap;
            
            volatile bool running;
            //watches
            StopWatch startWatch;
            StopWatch stopWatch;
            std::string name;
            Metadata metadata;
            MetadataMap metadataMapping;// only for mapping.
        };
    }
}

