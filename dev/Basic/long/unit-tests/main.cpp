/*
 * main.cpp : Unit Testing long term code
 *
 *  Created on: May 14, 2014
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

///Define SIMMOB_USE_TEST_GUI to use the GUI for CPPUnit tests.
#include "GenConfig.h"

//Dependencies for cppunit
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

//Additional dependencies for QXCppunit
#ifdef SIMMOB_USE_TEST_GUI
#include <QtGui/QApplication>
#include <qxcppunit/testrunner.h>
#endif

int main(int argc, char *argv[])
{
#ifdef SIMMOB_USE_TEST_GUI
	QApplication app(argc, argv);
	QxCppUnit::TestRunner runner;

	runner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
	runner.run();

	return 0;
#else
	CppUnit::TestResult controller;

    CppUnit::TestResultCollector result;
    controller.addListener(&result);

    CppUnit::BriefTestProgressListener progress;
    controller.addListener(&progress);

    CppUnit::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    runner.run(controller);

    CppUnit::CompilerOutputter outputter(&result, CppUnit::stdCOut());
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
#endif
}
