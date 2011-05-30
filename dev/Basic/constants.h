#pragma once

//Constant parameters
const unsigned int shortestPathLoopTimeStep   =    10;
const unsigned int agentDecompositionTimeStep =   100;
const unsigned int objectMgmtTimeStep         =  1000;

//Work Group sizes
const unsigned int WG_TRIPCHAINS_SIZE = 4;
const unsigned int WG_CREATE_AGENT_SIZE = 1; //Testing size 1
const unsigned int WG_CHOICESET_SIZE = 6;
const unsigned int WG_VEHICLES_SIZE = 5;
const unsigned int WG_AGENTS_SIZE = 5;

//"Trivial" function does nothing, returns a trivial condition. Used to indicate future functionality.
//  Declared here
bool trivial(unsigned int id);
