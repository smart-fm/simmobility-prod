//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ExpandAndValidateConfigFile.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/PrintNetwork.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/Agent.hpp"
#include "entities/BusController.hpp"
#include "entities/Entity.hpp"
#include "entities/Person.hpp"
#include "entities/amodController/AMODController.hpp"
#include "entities/params/PT_NetworkEntities.hpp"
#include "geospatial/Incident.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/RoadItem.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/xmlLoader/geo10.hpp"
#include "geospatial/xmlWriter/boostXmlWriter.hpp"
#include "partitions/PartitionManager.hpp"
#include "util/Profiler.hpp"
#include "util/ReactionTimeDistributions.hpp"
#include "util/Utils.hpp"
#include "workers/WorkGroup.hpp"
#include "path/PT_PathSetManager.hpp"

using namespace sim_mob;

sim_mob::ExpandAndValidateConfigFile::ExpandAndValidateConfigFile(ConfigParams& result) : cfg(result)
{
    processConfig();
}

void sim_mob::ExpandAndValidateConfigFile::processConfig()
{

}





