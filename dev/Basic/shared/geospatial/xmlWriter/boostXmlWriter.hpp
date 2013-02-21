#include <string>

namespace sim_mob {

//Forward Declaration
class RoadNetwork;

/**
 * Save the road network via the boost::xml functions.
 */
void BoostSaveXML(const std::string& outFileName, const sim_mob::RoadNetwork& network);

}
