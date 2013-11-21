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

using namespace sim_mob;
typedef double (*InternalFunction)(Math::Function func, double x,
        const boost::tuple<double, double, double>& params, double crit);

const double Math::E(2.71828182845904523536);
const double Math::PI(3.1415926535897932385);

namespace {

    /**
     * Newtows method based on given N(numerator function) and 
     * D (denominator function)
     * @param func to run.
     * @param x0 starting point.
     * @param params to pass to the function.
     * @param crit sensitivity of the function. (ex:0.001)
     * @param maxIterations Maximum number of iterations to find the zero.
     * @return 
     */
    template<InternalFunction N, InternalFunction D>
    inline double newtonMethod(Math::Function func, double x0,
            const boost::tuple<double, double, double>& params,
            double crit, int maxIterations) {
        double x1 = 0;
        double delta = 0;
        int iters = 0;
        do {
            x1 = x0 - N(func, x0, params, crit) / D(func, x0, params, crit);
            delta = fabs(x1 - x0);
            x0 = x1;
            iters++;
        } while (delta > crit && iters <= maxIterations);
        return x0;
    }

    /**
     * Original function 
     */
    inline double originalFunc(Math::Function func, double x0,
            const boost::tuple<double, double, double>& params, double crit) {
        return func(x0, params);
    }

    /**
     * First numerical derivative.
     * F'(x) = (f(x + crit) - f(x - crit)) / 2*crit 
     * 
     */
    inline double numericalDerivative(Math::Function func, double x0,
            const boost::tuple<double, double, double>& params, double crit) {
        return ((func((x0 + crit), params) - func((x0 - crit), params)) / (2 * crit));
    }

    /**
     * Second numerical derivative.
     * F''(x) = (f(x + crit) - (2 * f(x)) + f(x - crit)) / crit^2
     *  
     */
    inline double numerical2Derivative(Math::Function func, double x0,
            const boost::tuple<double, double, double>& params, double crit) {
        return ((func((x0 + crit), params) -
                (2 * func((x0), params)) +
                (func((x0 - crit), params))) / (crit * crit));
    }
}

double Math::newton(Function func, double x0,
        const boost::tuple<double, double, double>& params,
        double crit, int maxIterations) {
    return newtonMethod<originalFunc, numericalDerivative>
            (func, x0, params, crit, maxIterations);
}

double Math::findMaxArg(Function func, double x0,
        const boost::tuple<double, double, double>& params,
        double crit, int maxIterations) {
    return newtonMethod<numericalDerivative, numerical2Derivative>
            (func, x0, params, crit, maxIterations);
}
