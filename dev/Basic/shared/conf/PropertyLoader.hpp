/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   PropertyLoader.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on November 6, 2013, 11:26 AM
 */
#pragma once
#include <string>
#include <boost/property_tree/ptree.hpp>

namespace sim_mob {

    /**
     * Represents an abstract implementation (.ini) properties file loader.
     * 
     * Implementing the method **loadImpl** derived classes shall load their 
     * properties inside of this method.
     * 
     * Example: 
     * 
     * void MyConfig::loadImpl(const boost::property_tree::ptree& tree) {
     *  myProp = tree.get<string>(toProp(getSection(), "myPropName"));
     * }
     * 
     */
    class PropertyLoader {
    public:
        /**
         * @param file to read properties.
         * @param section properties prefix.
         */
        PropertyLoader(const std::string& filePath, const std::string& section);
        PropertyLoader(const PropertyLoader& source);
        PropertyLoader();
        virtual ~PropertyLoader();

        const std::string& getSection() const;
        const std::string& getFilePath() const;

        /**
         * Loads properties.
         */
        void load();

        /**
         * Merges the given section with given property like (section + "." + prop)
         * @param section property prefix.
         * @param prop property name.
         * @return new string with property full path.
         */
        static const std::string toProp(const std::string& section, const std::string& prop);
        
    protected:
        /**
         * Method to be implemented by specific loader.
         * @param tree to load.
         */
        virtual void loadImpl(const boost::property_tree::ptree& tree) = 0;
    private:
        std::string filePath;
        std::string section;
    };
}
