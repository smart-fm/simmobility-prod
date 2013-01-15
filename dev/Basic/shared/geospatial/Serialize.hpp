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

#include <set>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <boost/noncopyable.hpp>

#include "util/XmlWriter.hpp"

#include "geospatial/RoadNetwork.hpp"
#include "boost/serialization/vector.hpp"


namespace sim_mob {
namespace xml {


void write_xml(XmlWriter& write, const sim_mob::Link& lnk)
{
	//TEMP
    write.prop("hi", 2);
}

void write_xml(XmlWriter& write, const sim_mob::MultiNode& mnd)
{
	//TEMP
    write.prop("hi", 2);
}

void write_xml(XmlWriter& write, const sim_mob::UniNode& und)
{
	//TEMP
    write.prop("hi", 2);
}

void write_xml(XmlWriter& write, const sim_mob::RoadNetwork& rn)
{
    write.prop_begin("RoadNetwork");

    //Nodes are also wrapped
    write.prop_begin("Nodes");
    write.prop("UniNodes", rn.getUniNodes());
    write.prop("Intersections", rn.getNodes());
    write.prop_end(); //Nodes

    write.prop("Links", rn.getLinks());
	write.prop_end(); //RoadNetwork
}



}}
