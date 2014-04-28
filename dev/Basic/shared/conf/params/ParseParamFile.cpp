/*
 * ParseParamFile.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Max
 */

#include "ParseParamFile.hpp"

namespace sim_mob {

using namespace xercesc;
using namespace std;

ParseParamFile::ParseParamFile(const std::string& paramFileName) {
	//Xerces initialization.
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException& error) {
		throw std::runtime_error(string(error.getMessage()).c_str());
	}

}

ParseParamFile::~ParseParamFile() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
