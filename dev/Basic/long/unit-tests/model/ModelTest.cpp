/*
 * ModelTest.cpp
 *
 *  Created on: Aug 26, 2014
 *      Author: gishara
 */

#include "ModelTest.hpp"
#include <typeinfo>
#include <string>

#include "model/DeveloperModel.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/WorkGroupManager.hpp"
#include "database/dao/ParcelMatchDao.hpp"
#include "database/dao/ParcelDao.hpp"
#include "database/dao/ProjectDao.hpp"
#include "util/HelperFunctions.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using namespace sim_mob::long_term;
using namespace unit_tests;
using std::cout;
using std::endl;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::ModelTest);

void ModelTest::setUp()
{
	WorkGroupManager wgMgr;
	    wgMgr.setSingleThreadMode(true);

	    const unsigned int tickStep = 1;
	    const unsigned int days = 365;
	    const unsigned int timeIntervalDevModel = 30;
		WorkGroup* devWorkers = wgMgr.newWorkGroup(2, days, tickStep);
		developerModel = new DeveloperModel(*devWorkers, timeIntervalDevModel );
		DB_Config config(LT_DB_CONFIG_FILE);
		config.load();
		DB_Connection conn(sim_mob::db::POSTGRES, config);
		conn.connect();

}


