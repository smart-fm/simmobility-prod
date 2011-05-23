#pragma once

//Constant parameters
const unsigned int shortestPathLoopTimeStep   =    10;
const unsigned int agentDecompositionTimeStep =   100;
const unsigned int objectMgmtTimeStep         =  1000;

//Work Group sizes
const unsigned int WG_AGENTS_SIZE = 5;

//Driver modes
enum DRIVER_MODES {
	DRIVER,
	PEDESTRIAN,
	CYCLIST,
	PASSENGER
};

//"Trivial" function does nothing, returns a trivial condition. Used to indicate future functionality.
//  Declared here
bool trivial(unsigned int id);
