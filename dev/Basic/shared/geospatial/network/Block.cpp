/*
 * Block.cpp
 *
 *  Created on: Feb 5, 2016
 *      Author: fm-simmobility
 */

#include <geospatial/network/Block.hpp>

namespace sim_mob {

Block::Block():blockId(0),length(0.0),speedLimit(0.0),accelerateRate(0.0),decelerateRate(0.0),polyLine(nullptr),attachedPlatform(nullptr) {
	// TODO Auto-generated constructor stub

}

Block::~Block() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
