/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Math.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on June 13, 2013, 6:15 PM
 */
#pragma once

namespace sim_mob {

    class Math {
    public:
        typedef double (*Function)(double x);
        typedef double (*FunctionDerivate)(double x);

        /**
         * Calculates the newton method based on given function and derivative.
         * 
         * The algorithm will stop when one of the following conditions was reached: 
         * - |NM(x) - x| > criteria
         * - Maximum number of iterations was reached.
         * 
         * @param func original function.
         * @param derivate Derivate function
         * @param x0 starting point.
         * @param criteria to stop
         * @param maxIterations maximum number of iterations.
         * @return minimum value found.
         */
        static double Newton(Function func, FunctionDerivate derivate, double x0,
                double criteria, int maxIterations);
    };
}

