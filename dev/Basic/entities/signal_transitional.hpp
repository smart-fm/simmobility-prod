#pragma once

//Either forward declare Signal_Parent or make a typedef.
//This is namespace pollution, but we'll fix this later.
namespace sim_mob {
class Signal;

#ifndef SIMMOB_NEW_SIGNAL
typedef Signal Signal_Parent;
#else
class Signal_Parent;
#endif

}

