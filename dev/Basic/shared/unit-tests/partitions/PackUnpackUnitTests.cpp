/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "PackUnpackUnitTests.hpp"

#include "conf/settings/DisableMPI.h"

#ifndef SIMMOB_DISABLE_MPI

#include <cmath>
#include <limits>
#include <string>
#include <sstream>

#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"

#include "buffering/BufferedDataManager.hpp"
#include "util/DynamicVector.hpp"
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
	srcFD.set_delay(90);
	for (size_t i=0; i<100; i+=10) { //[0,0.0, 10,1.1, ... 90,9.9]
		srcFD.update(i);
		srcFD.delay(i*1.1);
	}
	srcFD.update(100);

	//Now pack it.
	PackageUtils p;
	p << srcFD;

	//Unpack it
	UnPackageUtils up(p.getPackageData());
	FixedDelayed<double> destFD(10);  //Intentionally create the WRONG time delay.
	up >> destFD;

	//At this point, the two items should be in lock-step, at time 100, with a delay of 90ms.
	CPPUNIT_ASSERT_EQUAL(srcFD.sense(), destFD.sense());

	//Advance 1, then the other. Ensure they're not equal in between.
	srcFD.update(110);
	CPPUNIT_ASSERT(srcFD.sense()!=destFD.sense());
	destFD.update(110);
	CPPUNIT_ASSERT_EQUAL(srcFD.sense(), destFD.sense());

	//Step through a few more time steps.
	for (size_t i=120; i<190; i++) {
		srcFD.update(i);
		destFD.update(i);
		CPPUNIT_ASSERT_EQUAL(srcFD.sense(), destFD.sense());
	}

	//Ensure that our delay of 90 is enforced by having one point go past it.
	srcFD.update(190);
	destFD.update(200);
	CPPUNIT_ASSERT_EQUAL(srcFD.sense(), destFD.sense());
}

void unit_tests::PackUnpackUnitTests::test_PackUnpack_fixed_delayed_dpoint()
{
	//We only care that one point works.
	FixedDelayed<DPoint> srcFD(10); //10ms delay
	srcFD.delay(DPoint(1.1, 3.3));
	srcFD.update(9);

	//Now pack it.
	PackageUtils p;
	p << srcFD;

	//Unpack it
	UnPackageUtils up(p.getPackageData());
	FixedDelayed<DPoint> destFD; //Use the default constructor (0ms delay)
	up >> destFD;

	//Ensure neither one can be sensed
	CPPUNIT_ASSERT(!srcFD.can_sense());
	CPPUNIT_ASSERT(!destFD.can_sense());

	//Advance, make sure they can be sensed
	srcFD.update(10);
	destFD.update(10);
	CPPUNIT_ASSERT(srcFD.can_sense());
	CPPUNIT_ASSERT(destFD.can_sense());

	//Ensure they're equal
	CPPUNIT_ASSERT_EQUAL(srcFD.sense().x, destFD.sense().x);
	CPPUNIT_ASSERT_EQUAL(srcFD.sense().y, destFD.sense().y);
}



void unit_tests::PackUnpackUnitTests::test_PackUnpack_dynamic_vector()
{
	DynamicVector srcVec(10, 20, 50, 60);

	//Now pack it.
	PackageUtils p;
	p << srcVec;

	//Unpack it
	UnPackageUtils up(p.getPackageData());
	DynamicVector destVec; //Default constructor will leave destVec in an invalid state.
	up >> destVec;

	//Ensure that the two are equal
	CPPUNIT_ASSERT_EQUAL(srcVec.getX(), destVec.getX());
	CPPUNIT_ASSERT_EQUAL(srcVec.getY(), destVec.getY());
	CPPUNIT_ASSERT_EQUAL(srcVec.getEndX(), destVec.getEndX());
	CPPUNIT_ASSERT_EQUAL(srcVec.getEndY(), destVec.getEndY());
	CPPUNIT_ASSERT_EQUAL(srcVec.getMagnitude(), destVec.getMagnitude());
	CPPUNIT_ASSERT_EQUAL(srcVec.getAngle(), destVec.getAngle());
}

void unit_tests::PackUnpackUnitTests::test_PackUnpack_dynamic_vector2()
{
	//Make a "zero-length" vector.
	DynamicVector srcVec(10, 20, 10, 20);

	//Now pack it.
	PackageUtils p;
	p << srcVec;

	//Unpack it
	UnPackageUtils up(p.getPackageData());
	DynamicVector destVec(1,2,3,4); //Default constructor is "valid".
	up >> destVec;

	//Ensure that the two are equal
	CPPUNIT_ASSERT_EQUAL(srcVec.getX(), destVec.getX());
	CPPUNIT_ASSERT_EQUAL(srcVec.getY(), destVec.getY());
	CPPUNIT_ASSERT_EQUAL(srcVec.getEndX(), destVec.getEndX());
	CPPUNIT_ASSERT_EQUAL(srcVec.getEndY(), destVec.getEndY());
	CPPUNIT_ASSERT_EQUAL(srcVec.getMagnitude(), destVec.getMagnitude());

	//Ensure these fail
	CPPUNIT_ASSERT_THROW(srcVec.getAngle(), std::runtime_error);
	CPPUNIT_ASSERT_THROW(destVec.getAngle(), std::runtime_error);
}



#endif //SIMMOB_DISABLE_MPI







