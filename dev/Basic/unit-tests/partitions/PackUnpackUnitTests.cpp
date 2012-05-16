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

	//Check that x/y positions line up before a call to flip.
	//NOTE: These will definitely NOT be valid after the flip, since Agents don't perform
	//      decisions before being transferred to the new partition.
	CPPUNIT_ASSERT_EQUAL(srcAgent.xPos.get(),destAgent.xPos.get());
	CPPUNIT_ASSERT_EQUAL(srcAgent.yPos.get(),destAgent.yPos.get());
	/*bdm.flip();
	CPPUNIT_ASSERT_EQUAL(srcAgent.xPos.get(),destAgent.xPos.get());
	CPPUNIT_ASSERT_EQUAL(srcAgent.yPos.get(),destAgent.yPos.get());*/
}

void unit_tests::PackUnpackUnitTests::test_PackUnpack_fixed_delayed()
{
	//Build up a simple sequence.
	FixedDelayed<double> srcFD(100);  //Max of 100ms delay
	for (size_t i=0; i<100; i+=10) { //[0,0.0, 10,1.1, ... 90,9.9]
		srcFD.update(i);
		srcFD.delay(i*1.1);
	}

	//Now pack it.
	PackageUtils p;
	p << srcFD;

	//Unpack it
	UnPackageUtils up(p.getPackageData());
	//TODO



	CPPUNIT_FAIL("TODO: Implement this test.");
}


#endif //SIMMOB_DISABLE_MPI







