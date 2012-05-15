/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "PackUnpackUnitTests.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include <cmath>
#include <limits>
#include <string>
#include <sstream>

#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"

#include "buffering/BufferedDataManager.hpp"
#include "entities/Person.hpp"
#include "entities/Agent.hpp"

using namespace sim_mob;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::PackUnpackUnitTests);

void unit_tests::PackUnpackUnitTests::test_PackUnpack_simple_set_get()
{
	//Create two agents
	Person srcAgent(MtxStrat_Buffered);
	Person destAgent(MtxStrat_Buffered);

	//Manually start managing their flip-able properties.
	BufferedDataManager bdm;
	bdm.beginManaging(&srcAgent.xPos);
	bdm.beginManaging(&srcAgent.yPos);
	bdm.beginManaging(&destAgent.xPos);
	bdm.beginManaging(&destAgent.yPos);

	//Set the first agent halfway between tick 1 and tick 21
	srcAgent.xPos.set(100);
	srcAgent.yPos.set(200);
	bdm.flip();
	srcAgent.xPos.set(300);
	srcAgent.yPos.set(400);

	//Pack it
	PackageUtils p;
	srcAgent.pack(p);

	//Unpack it.
	UnPackageUtils up(p.getPackageData());
	destAgent.unpack(up);

	//Check that x/y positions line up before AND after a call to flip()
	CPPUNIT_ASSERT_EQUAL(srcAgent.xPos.get(),destAgent.xPos.get());
	CPPUNIT_ASSERT_EQUAL(srcAgent.yPos.get(),destAgent.yPos.get());
	bdm.flip();
	CPPUNIT_ASSERT_EQUAL(srcAgent.xPos.get(),destAgent.xPos.get());
	CPPUNIT_ASSERT_EQUAL(srcAgent.yPos.get(),destAgent.yPos.get());
}



#endif //SIMMOB_DISABLE_MPI
