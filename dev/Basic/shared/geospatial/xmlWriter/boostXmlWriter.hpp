//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <string>

namespace sim_mob {

//Forward Declaration
class RoadNetwork;

/**
 * Save the road network via the boost::xml functions.
 */
void BoostSaveXML(const std::string& outFileName, const sim_mob::RoadNetwork& network);

}
