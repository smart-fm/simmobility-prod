//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob {

namespace comm {

//TODO: This is not the best place for this, but it's certainly better than in ConfigParams
enum ClientType {
	UNKNOWN = 0,
	ANDROID_EMULATOR = 1,
	NS3_SIMULATOR = 2,
	//add your client type here
};

}}
