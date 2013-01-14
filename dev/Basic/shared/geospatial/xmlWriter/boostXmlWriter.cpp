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
    //boost::archive::xml_oarchive xml(outFile);
    //xml << boost::serialization::make_nvp("SimMobility", network);
    sim_mob::xml::xml_writer xml(outFile);
    xml.prop("geo:SimMobility", network);
}
