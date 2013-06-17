/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Math.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on June 13, 2013, 6:15 PM
 */

#include "Math.hpp"
#include <cmath>

using namespace sim_mob;

double Math::Newton(Function func, FunctionDerivate derivate, double x0,
        double crit, int maxIterations) {
    
    double x1 = 0;
    double delta = 0;
    int iters = 0;
    do {
        x1 = x0 - func(x0) / derivate(x0);
        delta = fabs(x1 - x0);
        x0 = x1;
        iters++;
    } while (delta > crit && iters <= maxIterations);
    return x1;
}