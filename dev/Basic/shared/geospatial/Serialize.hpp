/* Copyright Singapore-MIT Alliance for Research and Technology */

/**
 * \file Serialize.hpp

 * This file contains a set of serialization functions (boost::serialize) for the Geospatial road
 *   network classes which are compatible with boost::xml.
 * Be careful when including this file, as it has a large number of dependencies coupled with template
 *   expansion.
 *
 * \note
 * In terms of justification for why we put all of these functions outside of their respective classes,
 * there are three. First, I don't want tiny classes like Point2D and Node having boost::xml as a
 * dependency. Second, no "friends" are needed if the proper accessors are provided. Third, a friend
 * class would have been needed anyway in boost::serialization::access, so we might as well remove
 * these functions. The downside is that changes to the RoadNetwork classes will force Serialize.hpp to
 * recompile, but that is mostly unavoidable (and the classes are mostly stable now anyway). If performance
 * really becomes a problem, we can force the template into a separate cpp file (although it's messy).
 *
 * \note
 * These serialization functions are used for *writing* only; they are untested and unintended for reading.
 *
 * \author Seth N. Hetu
 */


#pragma once

#include <iostream>
#include <stdexcept>
#include <boost/noncopyable.hpp>

#include "util/XmlWriter.hpp"

#include "geospatial/RoadNetwork.hpp"

namespace sim_mob {
namespace xml {


//TODO: Serialization functions go here.
void write_xml(xml_writer& write, const sim_mob::RoadNetwork& rn) {
	//TODO: These go into lightweight containers later.
	//TODO: Or, we can do something like "prop_start", "prop_end" that allows us to fake simple containers.
	write.attr("xmlns:geo", "http://www.smart.mit.edu/geo");
	write.attr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	write.attr("xsi:schemaLocation", "http://www.smart.mit.edu/geo  ../shared/geospatial/xmlLoader/geo10.xsd");
	write.prop("my_prop", 2);
	write.prop("my_side", rn.drivingSide);
}

void write_xml(xml_writer& write, const sim_mob::DRIVING_SIDE& temp) {
	write.attr("internal_attr", "2");
}

}}
