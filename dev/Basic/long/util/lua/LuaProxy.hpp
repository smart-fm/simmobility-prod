/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LuaProxy.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 9, 2013, 4:39 PM
 */
#pragma once
#include <vector>
namespace sim_mob {

    namespace long_term {

        class LuaProxy {
        public:
            /**
             * Loads the given file lua to the current lua state
             * @param filePath
             */
            static void loadFile();
            
            /**
             * 
             * @param price last expectation (V(t+1))
             * @param expectation current expectation.
             * @param theta variable parameter.
             * @param alpha variable parameter.
             * @return The expectation.
             */
            static double HM_SellerExpectation(double price, double expectation, double theta, double alpha);
            static void arrayTest(std::vector<double>& out);
        private:

        };
    }
}

