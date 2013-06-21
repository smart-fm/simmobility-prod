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
#include <stdio.h>

using namespace sim_mob;

typedef double (*Formula)(Math::Function func, double x, double* params, double crit);

double Math::E = 2.71828182845904523536;

/**
 * 
 * F'(x) = (f(x + crit) - f(x - crit)) / 2*crit
 *  
 * 
 */
double Numerical1Derivative(Math::Function func, double x0, double* params, double crit) {
    return ((func((x0 + crit), params) - func((x0 - crit), params)) / (2 * crit));
}

/**
 * 
 * F''(x) = (f(x + crit) - (2 * f(x)) + f(x - crit)) / crit^2
 *  
 */
double Numerical2Derivative(Math::Function func, double x0, double* params, double crit) {
    return ((func((x0 + crit), params) - (2 * func((x0), params)) + (func((x0 - crit), params))) / (crit * crit));
}

/**
 *
 * Formula: x0 - func(x0)/ func'(x0)
 *  
 */
double NewtonFindMin(Math::Function func, double x0, double* params, double crit) {
    return x0 - func(x0, params) / Numerical1Derivative(func, x0, params, crit);
}

/**
 *
 * Formula: x0 - func'(x0)/ func''(x0)
 *  
 */
double NewtonFindMax(Math::Function func, double x0, double* params, double crit) {
    return x0 - Numerical1Derivative(func, x0, params, crit) / Numerical2Derivative(func, x0, params, crit);
}

double NewtonMethod(Formula formula, Math::Function func, double x0, double* params, double crit, int maxIterations) {
    double x1 = 0;
    double delta = 0;
    int iters = 0;
    do {
        x1 = formula(func, x0, params, crit);
        delta = fabs(x1 - x0);
        x0 = x1;
        iters++;
    } while (delta > crit && iters <= maxIterations);
    return x0;
}

double Math::Newton(Function func, double x0, double* params, double crit, int maxIterations) {
    return NewtonMethod(NewtonFindMin, func, x0, params, crit, maxIterations);
}

double Math::FindMaxArg(Function func, double x0, double* params, double crit, int maxIterations) {
    return NewtonMethod(NewtonFindMax, func, x0, params, crit, maxIterations);
}