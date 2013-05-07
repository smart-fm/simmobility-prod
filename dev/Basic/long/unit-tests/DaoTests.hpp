/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DaoTests.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 7, 2013, 5:22 PM
 */
#pragma once

namespace unit_tests {

    class DaoTests {
    public:
        /**
         * Tests the Individual DAO
         */
        void TestIndividualDao();

        /**
         * Tests the Household DAO
         */
        void TestHouseholdDao();
    };
}


