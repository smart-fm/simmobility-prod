//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
    sim_mob::xml::XmlWriter write(outFile);
    write.prop_begin("geo:SimMobility");
    write.attr_prefix("\n    ");  //Kind of a hack, but we want to format attributes just right.
	write.attr("xmlns:geo", "http://www.smart.mit.edu/geo");
	write.attr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	write.attr("xsi:schemaLocation", "http://www.smart.mit.edu/geo  ../shared/geospatial/xmlLoader/geo10.xsd");
	write.endl();
	write.prop("GeoSpatial", network);
	write.prop("Signals", sim_mob::Signal::all_signals_, sim_mob::xml::namer("<Signal>"));
	write.prop_end();

}
