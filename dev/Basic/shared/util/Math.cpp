//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Math.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on June 13, 2013, 6:15 PM
 */

#include "Math.hpp"
#include <cmath>
#include <stdio.h>

using namespace sim_mob;

typedef double (*InternalFunction)(Math::Function func, double x, const boost::tuple<double,double,double>& params, double crit);

const double Math::E (2.71828182845904523536);
const double Math::PI (3.1415926535897932385);

/**
 * Original function 
 */
double OriginalFunction(Math::Function func, double x0, const boost::tuple<double,double,double>& params, double crit) {
    return func(x0, params);
}

/**
 * 
 * F'(x) = (f(x + crit) - f(x - crit)) / 2*crit 
 * 
 */
double Numerical1Derivative(Math::Function func, double x0, const boost::tuple<double,double,double>& params, double crit) {
    return ((func((x0 + crit), params) - func((x0 - crit), params)) / (2 * crit));
}

/**
 * 
 * F''(x) = (f(x + crit) - (2 * f(x)) + f(x - crit)) / crit^2
 *  
 */
double Numerical2Derivative(Math::Function func, double x0, const boost::tuple<double,double,double>& params, double crit) {
    return ((func((x0 + crit), params) - (2 * func((x0), params)) + (func((x0 - crit), params))) / (crit * crit));
}

template<InternalFunction numerator, InternalFunction denominator>
double NewtonMethod(Math::Function func, double x0, const boost::tuple<double,double,double>& params, double crit, int maxIterations) {
    double x1 = 0;
    double delta = 0;
    int iters = 0;
    do {
        x1 = x0 - numerator(func, x0, params, crit) / denominator(func, x0, params, crit);
        delta = fabs(x1 - x0);
        x0 = x1;
        iters++;
    } while (delta > crit && iters <= maxIterations);
    return x0;
}

double Math::Newton(Function func, double x0, const boost::tuple<double,double,double>& params, double crit, int maxIterations) {
    return NewtonMethod<OriginalFunction, Numerical1Derivative>(func, x0, params, crit, maxIterations);
}

double Math::FindMaxArg(Function func, double x0, const boost::tuple<double,double,double>& params, double crit, int maxIterations) {
    return NewtonMethod<Numerical1Derivative, Numerical2Derivative>(func, x0, params, crit, maxIterations);
}
