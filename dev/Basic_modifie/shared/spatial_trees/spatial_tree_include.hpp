#pragma once

//#define MEAUSURE_COUNTS

//it is important, if there is no prior information about the density
//#define SIM_TREE_USE_REBALANCE

//it is important for large-scale simulation.
#define SIM_TREE_BOTTOM_UP_QUERY

//this one is only valid for 1 worker thread.
//#define QUERY_PROFILING

/**
 * when Sim-Tree rebuilds itself, it will firstly divide the network into a number of small cells (rectangles), and then collect the density in each cell.
 * After that, Sim-Tree build a Binary Tree based on these cells.
 * DIVIDE_NETWORK_X_INTO_CELLS = 1000 and DIVIDE_NETWORK_Y_INTO_CELLS = 1000 is a good choice for most networks.
 * It is suggested not to change these two values.
 */
const int DIVIDE_NETWORK_X_INTO_CELLS = 1000;
const int DIVIDE_NETWORK_Y_INTO_CELLS = 1000;
