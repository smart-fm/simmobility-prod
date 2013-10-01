//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Math.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on June 13, 2013, 6:15 PM
 */
#pragma once

#include "boost/tuple/tuple.hpp"

namespace sim_mob {

    class Math {
    public:
        static const double E;
        static const double PI;
    public:
        typedef double (*Function)(double x, const boost::tuple<double,double,double>& params);

        /**
         * Calculates the newton method by using the default derivate definition.
         * Attention: in some functions you can have some error. 
         * 
         * The algorithm will stop when one of the following conditions was reached: 
         * - |NM(x) - x| > criteria
         * - Maximum number of iterations was reached.
         * 
         * Derivate definition that this method uses is:
         * 
         * f'(x) = (f(x + crit) - f(x - crit)) / (2*crit) 
         * 
         * @param func original function.
         * @param x0 starting point.
         * @param params that can be passed to the function.
         * @param criteria to stop
         * @param maxIterations maximum number of iterations.
         * @return minimum value found.
         */
        static double Newton(Function func, double x0, const boost::tuple<double,double,double>& params, double crit, int maxIterations);

        /**
         * Try to find the maxium of the given original function. 
         * 
         * Basically, this method uses the Newton's methodology but using the 
         * Derivate and the second derivate of the function. (Numerical approximations).
         * 
         * x = x0 - func'(x0)/func''(x0);
         * 
         *The algorithm will stop when one of the following conditions was reached: 
         * - |NM(x) - x| > criteria
         * - Maximum number of iterations was reached.
         * 
         * @param func Original function.
         * @param x0 starting point.
         * @param params to pass to the function.
         * @param crit criteria to check the convergence of the function.
         * @param maxIterations to do, if the function gets a maximum 
         *        that satisfies the criteria the algorithm will stop.
         * @return maximum x argument.
         */
        static double FindMaxArg(Function func, double x0, const boost::tuple<double,double,double>& params, double criteria, int maxIterations);
    };
}

