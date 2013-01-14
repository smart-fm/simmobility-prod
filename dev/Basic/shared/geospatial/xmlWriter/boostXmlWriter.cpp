#include "boostXmlWriter.hpp"

#include "geospatial/Serialize.hpp"

#include <iostream>
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>

#include "geospatial/RoadNetwork.hpp"

using std::ios;

void sim_mob::BoostSaveXML(const std::string& outFileName, const sim_mob::RoadNetwork& network)
{
    std::ofstream outFile(outFileName.c_str());
    boost::archive::xml_oarchive xml(outFile);
    xml << boost::serialization::make_nvp("SimMobility", network);

    //TODO: need to add "geo" namespace back to "SimMobility" tag, above.

    //TODO: Need to set the following top-level options:
    //"1.0", "utf-8"

    //TODO: Need to set the following attributes:
	//SetAttribute("xmlns:geo" , "http://www.smart.mit.edu/geo");
	//SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	//SetAttribute("xsi:schemaLocation", "http://www.smart.mit.edu/geo file:/home/vahid/Desktop/geo8/geo10.xsd");
}
