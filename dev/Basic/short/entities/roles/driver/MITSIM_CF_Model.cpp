//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * CF_Model.cpp
 *
 *  Created on: 2011-8-15
 *      Author: wangxy & Li Zhemin
 */

#include <boost/random.hpp>

#include <limits>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "entities/vehicle/Vehicle.hpp"
#include "entities/models/CarFollowModel.hpp"
#include "util/Math.hpp"
#include "util/Utils.hpp"
#include "Driver.hpp"
#include "util/SingletonHolder.hpp"

using std::numeric_limits;
using namespace sim_mob;
using namespace std;


namespace {
    //Random number generator
    //TODO: We need a policy on who can get a generator and why.
    //boost::mt19937 gen;

    //Threshold defaults
    //const double hBufferUpper			=	  1.6;	 ///< upper threshold of headway
    //const double hBufferLower			=	  0.8;	 ///< lower threshold of headway

    //Set default data for acceleration
    //const double maxAcceleration = 5.0;   ///< 5m/s*s, might be tunable later
    //const double normalDeceleration = -maxAcceleration*0.6;
    //const double maxDeceleration = -maxAcceleration;

    //Simple conversion

    double feet2Unit(double feet) {
        return feet * 0.158;
    }

    //Simple struct to hold Car Following model parameters

    struct CarFollowParam {
        double alpha;
        double beta;
        double gama;
        double lambda;
        double rho;
        double stddev;
    };

    //Car following parameters for this model.
    const CarFollowParam CF_parameters[2] = {
        //    alpha   beta    gama    lambda  rho     stddev
        { 0.0400, 0.7220, 0.2420, 0.6820, 0.6000, 0.8250},
        {-0.0418, 0.0000, 0.1510, 0.6840, 0.6800, 0.8020}
    };

    const double targetGapAccParm[] = {0.604, 0.385, 0.323, 0.0678, 0.217,
        0.583, -0.596, -0.219, 0.0832, -0.170, 1.478, 0.131, 0.300};

    //Acceleration mode// CLA@04/2014 this enum can be deleted

    enum ACCEL_MODE {
        AM_VEHICLE = 0,
        AM_PEDESTRIAN = 1,
        AM_TRAFF_LIGHT = 2,
        AM_NONE = 3
    };

    double uRandom(boost::mt19937& gen) {
        boost::uniform_int<> dist(0, RAND_MAX);
        long int seed_ = dist(gen);

        const long int M = 2147483647; // M = modulus (2^31)
        const long int A = 48271; // A = multiplier (was 16807)
        const long int Q = M / A;
        const long int R = M % A;
        seed_ = A * (seed_ % Q) - R * (seed_ / Q);
        seed_ = (seed_ > 0) ? (seed_) : (seed_ + M);
        return (double) seed_ / (double) M;
    }

    double nRandom(boost::mt19937& gen, double mean, double stddev) {
        double r1 = uRandom(gen), r2 = uRandom(gen);
        double r = -2.0 * log(r1);
        if (r > 0.0) return (mean + stddev * sqrt(r) * sin(2 * 3.1415926 * r2));
        else return (mean);
    }

    double CalcHeadway(double space, double speed, double elapsedSeconds, double maxAcceleration) {
        if (speed == 0) {
            return 2 * space * 100000;
        } else {
            return 2 * space / (speed + speed + elapsedSeconds * maxAcceleration);
        }
    }

    /**
     * Converts the given string into a vector of doubles.<br> 
     * Values shall be separated by whitespace.  
     * <br>
     *     e.g "16.0   14.5   13.0   11.0   10.0"<br>
     * <br>
     *  
     * @param str to process.
     * @param out data to fill.
     */
    void strToArray(std::string& str, std::vector<double>& out) {
        std::vector<std::string> arrayStr;
        boost::trim(str);
        boost::split(arrayStr, str, boost::is_any_of(" "),
                boost::token_compress_on);
        std::vector<std::string>::const_iterator itr;
        for (itr = arrayStr.begin(); itr != arrayStr.end(); itr++) {
            const std::string& strVal = (*itr);
            double res = Utils::cast<double>(strVal);
            out.push_back(res);
        }
    }

    const std::string MODEL_NAME = "general_driver_model";

    /**
     * Accelerations/Decelerations models.
     */
    class ModelSpecification {
    public:

        enum Type {
            NORMAL_DECELERATION = 0,
            MAX_DECELERATION,
            MAX_ACCELERATION,
        };

        struct Data {

            Data() : upperBound(0) {
            }
            std::map<Vehicle::VEHICLE_TYPE, std::map<int, double> > speedMap;
            int upperBound;
            std::vector<double> scale;
        };

        ModelSpecification() : timeStep(0.0), tmpGrade(0.0), minSpeed(0.0),
        minResponseDistance(0.0) {
            initParam(MODEL_NAME);
        }
        ~ModelSpecification(){}

    public:

        double getRandScaleValue(const ModelSpecification::Type& type) const {
            // get random number (uniform distribution), as Random::urandom(int n) in MITSIM Random.cc
            int scaleNo = Utils::generateInt(0, data.at(type).scale.size());
            return data.at(type).scale[scaleNo];
        }
        
        double getSpeedMapValue(const ModelSpecification::Type& type,
                Vehicle::VEHICLE_TYPE vhType,
                int speed) const {
            return data.at(type).speedMap.at(vhType).at(speed);
        }
        
        double getUpperBound(const ModelSpecification::Type& type) const {
            return data.at(type).upperBound;
        }

        double getTimeStep() const {
            return timeStep;
        }

        double getTmpGrade() const {
            return tmpGrade;
        }

        double getMinSpeed() const {
            return minSpeed;
        }

        double getMinResponseDistance() const {
            return minResponseDistance;
        }

        double getAccGradeFactor() const {
            return accGradeFactor;
        }
    private:

        void initParam(const std::string& modelName) {
            // speed scaler
            string speedScalerStr, maxAccStr, decelerationStr, maxAccScaleStr, normalDecScaleStr, maxDecScaleStr;
            ParameterManager::Instance()->param(modelName, "speed_scaler", speedScalerStr, string("5 20 20"));
            // max acceleration
            ModelSpecification::Data maxAccModel;
            ParameterManager::Instance()->param(modelName, "max_acc_car1", maxAccStr, string("10.00  7.90  5.60  4.00  4.00"));
            buildSpeedMap(Vehicle::CAR, speedScalerStr, maxAccStr, maxAccModel);
            ParameterManager::Instance()->param(modelName, "max_acceleration_scale", maxAccScaleStr, string("0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"));
            buildScale(maxAccScaleStr, maxAccModel);
            // normal deceleration
            ModelSpecification::Data normalDecModel;
            ParameterManager::Instance()->param(modelName, "normal_deceleration_car1", decelerationStr, string("7.8 	6.7 	4.8 	4.8 	4.8"));
            buildSpeedMap(Vehicle::CAR, speedScalerStr, decelerationStr, normalDecModel);
            ParameterManager::Instance()->param(modelName, "normal_deceleration_scale", normalDecScaleStr, string("0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"));
            buildScale(maxAccScaleStr, normalDecModel);
            // max deceleration
            ModelSpecification::Data maxDecModel;
            ParameterManager::Instance()->param(modelName, "Max_deceleration_car1", decelerationStr, string("16.0   14.5   13.0   11.0   10.0"));
            buildSpeedMap(Vehicle::CAR, speedScalerStr, decelerationStr, maxDecModel);
            ParameterManager::Instance()->param(modelName, "max_deceleration_scale", maxDecScaleStr, string("0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"));
            buildScale(maxDecScaleStr, maxDecModel);
            // acceleration grade factor
            ParameterManager::Instance()->param(modelName, "acceleration_grade_factor", accGradeFactor, 0.305);
            ParameterManager::Instance()->param(modelName, "tmp_all_grades", tmpGrade, 0.0);
            // param for distanceToNormalStop()
            ParameterManager::Instance()->param(modelName, "min_speed", minSpeed, 0.1);
            ParameterManager::Instance()->param(modelName, "min_response_distance", minResponseDistance, 5.0);
            
            data.insert(std::make_pair(MAX_ACCELERATION, maxAccModel));
            data.insert(std::make_pair(NORMAL_DECELERATION, normalDecModel));
            data.insert(std::make_pair(MAX_DECELERATION, maxDecModel));
        }

        /**
         * Converts the given string into a vector of doubles.<br> 
         * Values shall be separated by whitespace.  
         * <br>
         *     e.g "16.0   14.5   13.0   11.0   10.0"<br>
         * <br>
         *  
         * @param str to process.
         * @param out data to fill.
         */
        void buildScale(std::string& str, ModelSpecification::Data& out) {
            strToArray(str, out.scale);
        }

        /**
         * Builds the speed map with by given accelerations and speed scaler. 
         * @param vhType
         * @param speedScalerStr speed scaler
         * @param maxAccStr string value
         * @param out data to be filled.
         */
        void buildSpeedMap(Vehicle::VEHICLE_TYPE vhType,
                std::string& speedScalerStr,
                std::string& maxAccStr,
                ModelSpecification::Data& out) {

            // for example
            // speedScalerStr "5 20 20" ft/sec
            // maxAccStr      "10.00  7.90  5.60  4.00  4.00" ft/(s^2)

            //processes the speed scaler array.
            std::vector<double> speedScalerArrayDouble;
            strToArray(speedScalerStr, speedScalerArrayDouble);

            //processes max acceleration array.
            std::vector<double> maxAccArray;
            strToArray(maxAccStr, maxAccArray);

            out.upperBound = round(speedScalerArrayDouble[1] * (speedScalerArrayDouble[0] - 1));
            map<int, double> cIdx;
            for (int speed = 0; speed <= out.upperBound; ++speed) {
                double maxAcc = 0.0;
                // Convert speed value to a table index.
                int j = speed / speedScalerArrayDouble[1];
                if (j >= (speedScalerArrayDouble[0] - 1)) {
                    maxAcc = maxAccArray[speedScalerArrayDouble[0] - 1];
                } else {
                    maxAcc = maxAccArray[j];
                }
                cIdx.insert(std::make_pair(speed, maxAcc));
            }
            out.speedMap[vhType] = cIdx;
        }

    private:
        map<ModelSpecification::Type, ModelSpecification::Data> data;
        /// time step to calculate state variables
        double timeStep;
        /// grade is the road slope
        double tmpGrade;
        double minSpeed;
        double minResponseDistance;
        double accGradeFactor;
    };

    /**
     * ModelSpecification Singleton.
     */
    template<typename T = ModelSpecification>
    struct ModelSpecificationLifeCycle {

        static T * create() {
            T* obj = new T();
            return obj;
        }

        static void destroy(T* obj) {
            delete obj;
        }
    };

    typedef SingletonHolder<ModelSpecification, ModelSpecificationLifeCycle> ModelSpecificationSingleton;

    /**
     * Gets the acceleration value depending of the given Acceleration model.
     * @param type
     * @param params
     * @param vhType
     * @return 
     */
    inline double getAccValue(const ModelSpecification::Type& type,
            sim_mob::DriverUpdateParams& params,
            Vehicle::VEHICLE_TYPE vhType = Vehicle::CAR) {

        if (!params.driver) {
            throw std::runtime_error("no driver");
        }

        const ModelSpecification& model = ModelSpecificationSingleton::getInstance();
        // convert speed to int in meters
        int speed = round(Utils::cmToMeter(params.perceivedFwdVelocity));
        //Keep the speed value within the range 0 to modelData.upperBound.
        speed = Utils::clamp(speed, 0, static_cast<int>(model.getUpperBound(type)));

        // TODO: get random multiplier from data file and normal distribution
        return (model.getSpeedMapValue(type, vhType, speed) - model.getTmpGrade() *
                model.getAccGradeFactor())
                * model.getRandScaleValue(type);
    }

} //End anon namespace

/*
 *--------------------------------------------------------------------
 * The acceleration model calculates the acceleration rate based on
 * interaction with other vehicles. The function returns a the
 * most restrictive acceleration (deceleration if negative) rate
 * among the rates given by several constraints.
 *--------------------------------------------------------------------
 */
sim_mob::MITSIM_CF_Model::MITSIM_CF_Model() {
    timeStep = ModelSpecificationSingleton::getInstance().getTimeStep();
}

double sim_mob::MITSIM_CF_Model::makeAcceleratingDecision(DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed) {

    // VARIABLE || FUNCTION ||				REGIME
    calcStateBasedVariables(p);

    double acc = maxAcceleration;
    //double aB = calcMergingRate(p);		// MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
    double aC = calcSignalRate(p); // near signal or incidents
    double aD = calcYieldingRate(p, targetSpeed, maxLaneSpeed); // when yielding
    double aE = waitExitLaneRate(p); //
    //double  aF = waitAllowedLaneRate(p);	// MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
    //double  aG = calcLaneDropRate(p);		// MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
    double aH1 = calcAdjacentRate(p); // to reach adjacent gap
    double aH2 = calcBackwardRate(p); // to reach backward gap
    double aH3 = calcForwardRate(p); // to reach forward gap
    // The target gap acceleration should be based on the target gap status and not on the min
    // MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
    /*
    if (status(STATUS_ADJACENT)) {
        double aH = calcAdjacentRate(p);	// to reach adjacent gap
      }
      else if (status(STATUS_BACKWARD)) {
        double aH = calcBackwardRate(p);	// to reach backward gap
      }
      else if (status(STATUS_FORWARD)) {
            double aH = calcForwardRate(p);		// to reach forward gap
      } else {
        double aH = desiredSpeedRate(p); // FUNCTION desiredSpeedRate MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
      }
     */

    // if (intersection){
    // double aI = approachInter(p); // when approaching intersection to achieve the turn speed
    // if(acc > aI) acc = aI;
    // }
    // FUNCTION approachInter MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)

    double aZ1 = carFollowingRate(p, targetSpeed, maxLaneSpeed, p.nvFwd);
    double aZ2 = carFollowingRate(p, targetSpeed, maxLaneSpeed, p.nvFwdNextLink);

    // Make decision
    // Use the smallest
    //	if(acc > aB) acc = aB;
    if (acc > aD) acc = aD;
    //if(acc > aF) acc = aF;
    if (acc > aH1) acc = aH1;
    if (acc > aH2) acc = aH2;
    if (acc > aH3) acc = aH3;
    //if(acc > aG) acc = aG;
    if (acc > aC) acc = aC;
    if (acc > aE) acc = aE;
    if (acc > aZ1) acc = aZ1;
    if (acc > aZ2) acc = aZ2;

    // SEVERAL CONDITONS MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)

    return acc;
}

/*
 *--------------------------------------------------------------------
 * Calculate acceleration rate by car-following constraint. This
 * function may also be used in the lane changing algorithm to find
 * the potential acceleration rate in neighbor lanes.
 *
 * CAUTION: The two vehicles concerned in this function may not
 * necessarily be in the same lane or even the same segment.
 *
 * A modified GM model is used in this implementation.
 *--------------------------------------------------------------------
 */
double sim_mob::MITSIM_CF_Model::carFollowingRate(DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed, NearestVehicle& nv) {
    p.space = Utils::cmToMeter(p.perceivedDistToFwdCar);
    double res = 0.0;
    if (p.space < 2.0 && p.isAlreadyStart && p.isBeforIntersecton 
            && Utils::cmToMeter(p.perceivedFwdVelocityOfFwdCar) < 1.0) {
        return maxDeceleration * 4.0;
    }
    
    if (p.space > 0) {
        if (!nv.exists()) {
            return accOfFreeFlowing(p, targetSpeed, maxLaneSpeed);
        }
        p.v_lead = Utils::cmToMeter(nv.driver->fwdVelocity.get());
        p.a_lead = Utils::cmToMeter(nv.driver->fwdAccel.get());
        
        double dt = p.elapsedSeconds;
        double headway = CalcHeadway(p.space, Utils::cmToMeter(p.perceivedFwdVelocity), dt, maxAcceleration);
        
        //Emergency deceleration overrides the perceived distance; check for it.
        {
            double emergSpace = Utils::cmToMeter(nv.distance);
            double emergHeadway = CalcHeadway(emergSpace, Utils::cmToMeter(p.perceivedFwdVelocity), dt, maxAcceleration);
            if (emergHeadway < hBufferLower) {
                //We need to brake. Override.
                p.space = emergSpace;
                headway = emergHeadway;
            }
        }
        
        p.space_star = p.space + p.v_lead * dt + 0.5 * p.a_lead * dt * dt;
        if (headway < hBufferLower) {
            res = accOfEmergencyDecelerating(p);
        }
        if (headway > hBufferUpper) {
            res = accOfMixOfCFandFF(p, targetSpeed, maxLaneSpeed);
        }
        if (headway <= hBufferUpper && headway >= hBufferLower) {
            res = accOfCarFollowing(p);
        }
    }
    return res;
}

double sim_mob::MITSIM_CF_Model::calcSignalRate(DriverUpdateParams& p) {
    double minacc = maxAcceleration;
    double yellowStopHeadway = 1; //1 second
    double minSpeedYellow = 2.2352; //5 mph = 2.2352 m / s

    sim_mob::TrafficColor color;

#if 0
    Signal::TrafficColor color;
#endif
    double distanceToTrafficSignal;
    distanceToTrafficSignal = p.perceivedDistToTrafficSignal;
    color = p.perceivedTrafficColor;
    double dis = p.perceivedDistToFwdCar;
    if (distanceToTrafficSignal < 500) {
        double dis = Utils::cmToMeter(distanceToTrafficSignal);

#if 0
        if (p.perceivedTrafficColor == sim_mob::Red) {
            double a = brakeToStop(p, dis);
            if (a < minacc)
                minacc = a;
        } else if (p.perceivedTrafficColor == sim_mob::Amber) {
            double maxSpeed = (speed > minSpeedYellow) ? speed : minSpeedYellow;
            if (dis / maxSpeed > yellowStopHeadway) {
                double a = brakeToStop(p, dis);
                if (a < minacc)
                    minacc = a;
            }
        } else if (p.perceivedTrafficColor == sim_mob::Green) {
            minacc = maxAcceleration;
        }
#else
        if (color == sim_mob::Red)
#if 0
            if (color == Signal::Red)
#endif
            {
                double a = brakeToStop(p, dis);
                if (a < minacc)
                    minacc = a;
            } else if (color == sim_mob::Amber)
#if 0
            else if (color == Signal::Amber)
#endif
            {
                double fwdVelocityInMeters = Utils::cmToMeter(p.perceivedFwdVelocity);
                double maxSpeed = (fwdVelocityInMeters > minSpeedYellow) ? fwdVelocityInMeters : minSpeedYellow;
                if (dis / maxSpeed > yellowStopHeadway) {
                    double a = brakeToStop(p, dis);
                    if (a < minacc)
                        minacc = a;
                }
            } else if (color == sim_mob::Green)
#if 0
            else if (color == Signal::Green)
#endif
            {
                minacc = maxAcceleration;
            }

#endif

    }
    return minacc;
}

double sim_mob::MITSIM_CF_Model::calcYieldingRate(DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed) {
    if (p.turningDirection == LCS_LEFT) {
        return carFollowingRate(p, targetSpeed, maxLaneSpeed, p.nvLeftFwd);
    } else if (p.turningDirection == LCS_RIGHT) {
        return carFollowingRate(p, targetSpeed, maxLaneSpeed, p.nvRightFwd);
    }
    return maxAcceleration;
}

double sim_mob::MITSIM_CF_Model::waitExitLaneRate(DriverUpdateParams& p) {
    double dx = Utils::cmToMeter(p.perceivedDistToFwdCar) - 5;
    if (p.turningDirection == LCS_SAME || dx > p.distanceToNormalStop)
        return maxAcceleration;
    else
        return brakeToStop(p, dx);
}

double sim_mob::MITSIM_CF_Model::calcForwardRate(DriverUpdateParams& p) {
    /*
    if(p.turningDirection == LCS_SAME)
            return maxAcceleration;
    NearestVehicle& nv = (p.turningDirection == LCS_LEFT)?p.nvLeftFwd:p.nvRightFwd;
     */

    if (p.targetGap != TG_Left_Fwd || p.targetGap != TG_Right_Fwd)
        return maxAcceleration;
    NearestVehicle& nv = (p.targetGap == TG_Left_Fwd) ? p.nvLeftFwd : p.nvRightFwd;

    if (!nv.exists())
        return maxAcceleration;
    double dis = Utils::cmToMeter(nv.distance) + targetGapAccParm[0];
    double dv = Utils::cmToMeter(nv.driver->fwdVelocity.get()) - Utils::cmToMeter(p.perceivedFwdVelocity);
    double acc = targetGapAccParm[1] * pow(dis, targetGapAccParm[2]);

    if (dv > 0) {
        acc *= pow(dv, targetGapAccParm[3]);
    } else if (dv < 0) {
        acc *= pow(-dv, targetGapAccParm[4]);
    }
    acc += targetGapAccParm[5] / 0.824;
    return acc;
}

double sim_mob::MITSIM_CF_Model::calcBackwardRate(DriverUpdateParams& p) {
    /*
    if(p.turningDirection == LCS_SAME)
            return maxAcceleration;
    //NearestVehicle& nv = (p.turningDirection == LCS_LEFT)?p.nvLeftFwd:p.nvRightFwd;
    NearestVehicle& nv = (p.turningDirection == LCS_LEFT)?p.nvLeftBack:p.nvRightBack;//change a mistake!!!
     */

    if (p.targetGap != TG_Left_Back || p.targetGap != TG_Right_Back)
        return maxAcceleration;
    NearestVehicle& nv = (p.targetGap == TG_Left_Back) ? p.nvLeftBack : p.nvRightBack;

    if (!nv.exists())
        return maxAcceleration;

    double dis = Utils::cmToMeter(nv.distance) + targetGapAccParm[0];
    double dv = Utils::cmToMeter(nv.driver->fwdVelocity.get()) - Utils::cmToMeter(p.perceivedFwdVelocity);

    double acc = targetGapAccParm[6] * pow(dis, targetGapAccParm[7]);

    if (dv > 0) {
        acc *= pow(dv, targetGapAccParm[8]);
    } else if (dv < 0) {
        acc *= pow(-dv, targetGapAccParm[9]);
    }
    acc += targetGapAccParm[10] / 0.824;
    return acc;
}

double sim_mob::MITSIM_CF_Model::calcAdjacentRate(DriverUpdateParams& p) {
    if (p.nextLaneIndex == p.currLaneIndex)
        return maxAcceleration;
    NearestVehicle& av = (p.nextLaneIndex > p.currLaneIndex) ? p.nvLeftFwd : p.nvRightFwd;
    NearestVehicle& bv = (p.nextLaneIndex > p.currLaneIndex) ? p.nvLeftBack : p.nvRightBack;
    if (!av.exists())
        return maxAcceleration;
    if (!bv.exists())
        return normalDeceleration;
    double position = Utils::cmToMeter(bv.distance);
    double gap = position + Utils::cmToMeter(av.distance);
    double acc = targetGapAccParm[11] * (targetGapAccParm[0] * gap - position);

    acc += targetGapAccParm[12] / 0.824;
    return acc;
}
/*
 *-------------------------------------------------------------------
 * This function returns the acceleration rate required to
 * accelerate / decelerate from current speed to a full
 * stop within a given distance.
 *-------------------------------------------------------------------
 */
double sim_mob::MITSIM_CF_Model::brakeToStop(DriverUpdateParams& p, double dis) {
    double fwdVelocityInMeters = Utils::cmToMeter(p.perceivedFwdVelocity);
    double dt = p.elapsedSeconds;
    if (dis > sim_mob::Math::DOUBLE_EPSILON) {
        double u2 = pow(fwdVelocityInMeters, 2.0);
        double acc = -u2 / dis * 0.5;
        if (acc <= normalDeceleration) {
            return acc;
        }
        double vt = fwdVelocityInMeters * dt;
        double a = dt * dt;
        double b = 2.0 * vt - normalDeceleration * a;
        double c = u2 + 2.0 * normalDeceleration * (dis - vt);
        double d = b * b - 4.0 * a * c;

        if (d < 0.0 || a <= 0.0) {
            return acc;
        }

        return (sqrt(d) - b) / a * 0.5;
    } else {
        return (dt > 0.0) ? -(fwdVelocityInMeters) / dt : maxDeceleration;
    }
}

/*
 *-------------------------------------------------------------------
 * This function returns the acceleration rate required to
 * accelerate / decelerate from current speed to a target
 * speed within a given distance.
 *-------------------------------------------------------------------
 */
double sim_mob::MITSIM_CF_Model::brakeToTargetSpeed(DriverUpdateParams& p, double s, double v) {
    //	//NOTE: This is the only use of epsilon(), so I just copied the value directly.
    //	//      See LC_Model for how to declare a private temporary variable. ~Seth
    //	if(p.space_star > sim_mob::Math::DOUBLE_EPSILON) {
    //		return  ((p.v_lead + p.a_lead * dt ) * ( p.v_lead + p.a_lead * dt) - v * v) / 2 / p.space_star;
    //	} else if ( dt <= 0 ) {
    //		return maxAcceleration;
    //	} else {
    //		return ( p.v_lead + p.a_lead * dt - v ) / dt;
    //	}
    double currentSpeed = Utils::cmToMeter(p.perceivedFwdVelocity);
    if (s > sim_mob::Math::DOUBLE_EPSILON) {
        double v2 = v * v;
        double u2 = currentSpeed * currentSpeed;
        return (v2 - u2) / s * 0.5;
    } 
    return (v - currentSpeed) / p.elapsedSeconds;
}

double sim_mob::MITSIM_CF_Model::accOfEmergencyDecelerating(DriverUpdateParams& p) {
    const double dv = Utils::cmToMeter(p.perceivedFwdVelocity) - p.v_lead;
    const double epsilon_v = 0.001;
    double result = 0.0;
    if (dv < epsilon_v) {
        result = p.a_lead + 0.25 * normalDeceleration;
    } else if (p.space > 0.01) {
        result = p.a_lead - dv * dv / 2 / p.space;
    } else {
        double v = p.v_lead + p.a_lead * p.elapsedSeconds;
        result = brakeToTargetSpeed(p, p.space_star, v);
    }
    return result;
}

double sim_mob::MITSIM_CF_Model::accOfCarFollowing(DriverUpdateParams& p) {
    const double density = 1; //represent the density of vehicles in front of the subject vehicle
    //now we ignore it, assuming that it is 1.
    double v = Utils::cmToMeter(p.perceivedFwdVelocity);
    int i = (v > p.v_lead) ? 1 : 0;
    double dv = (v > p.v_lead) ? (v - p.v_lead) : (p.v_lead - v);

    double res = CF_parameters[i].alpha * pow(v, CF_parameters[i].beta) / pow(v, CF_parameters[i].gama);
    res *= pow(dv, CF_parameters[i].lambda) * pow(density, CF_parameters[i].rho);
    res += feet2Unit(nRandom(p.gen, 0, CF_parameters[i].stddev));

    return res;
}

double sim_mob::MITSIM_CF_Model::accOfFreeFlowing(DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed) {
    double vn = Utils::cmToMeter(p.perceivedFwdVelocity);
    if (vn <= targetSpeed) {
        return (vn < maxLaneSpeed) ? maxAcceleration : 0;
    }
    return 0;
}

double sim_mob::MITSIM_CF_Model::accOfMixOfCFandFF(DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed) {
    if (p.space > p.distanceToNormalStop) {
        return accOfFreeFlowing(p, targetSpeed, maxLaneSpeed);
    } else {
        double v = p.v_lead + p.a_lead * p.elapsedSeconds;
        return brakeToTargetSpeed(p, p.space_star, v);
    }
}

double sim_mob::MITSIM_CF_Model::distanceToNormalStop(const DriverUpdateParams& p) {
    //	double minSpeed = 0.1;
    //	double minResponseDistance = 5;
    //	double DIS_EPSILON = 0.001;
    const ModelSpecification& model = ModelSpecificationSingleton::getInstance();
    double result = model.getMinResponseDistance();
    double fwdVelocityInMeters = Utils::cmToMeter(p.perceivedFwdVelocity);
    if (fwdVelocityInMeters > model.getMinSpeed()) {
        result = sim_mob::Math::DOUBLE_EPSILON -
                0.5 * (fwdVelocityInMeters) * (fwdVelocityInMeters) / normalDeceleration;
        if (result < model.getMinResponseDistance()) {
            result = model.getMinResponseDistance();
        }
    }
    return result;
}

void sim_mob::MITSIM_CF_Model::calcStateBasedVariables(DriverUpdateParams& p) {

    timeStep -= p.elapsedSeconds;

    // if time step >0 ,no need update variables
    if (timeStep > 0) {
        return;
    }

    p.distanceToNormalStop = distanceToNormalStop(p);

    // Acceleration rate for a vehicle (a function of vehicle type,
    // facility type, segment grade, current speed).
    maxAcceleration = getAccValue(ModelSpecification::MAX_ACCELERATION, p);
    // Deceleration rate for a vehicle (a function of vehicle type, and
    // segment grade).
    normalDeceleration = getAccValue(ModelSpecification::NORMAL_DECELERATION, p);
    // Maximum deceleration is function of speed and vehicle class
    maxDeceleration = getAccValue(ModelSpecification::MAX_DECELERATION, p);

    timeStep = calcNextStepSize();
}

double sim_mob::MITSIM_CF_Model::calcNextStepSize() {
    return 0.2;
}
