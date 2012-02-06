/*
 * RoadNode.hpp
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#pragma once

namespace partitioning {
class RoadNode {
public:
int node_id;
int node_index; //start from 0, for METIS

double x_pos;
double y_pos;

double node_weight;
};
}
