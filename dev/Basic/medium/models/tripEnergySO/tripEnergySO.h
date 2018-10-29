#pragma once

#include <iostream>

//#include "./rt_nonfinite.h"
//#include "./rt_nonfinite.h"
//#include "./rtGetNaN.h"
//#include "./rtwtypes.h"
#include "../../../shared/behavioral/params/PersonParams.hpp"
//#include "loadDrivingSO_types.h"

//#include "./loadVehicles.h"
//#include "./getEnergy_TS_3d.h"
//#include "./loadDrivingSO.h"
//#include "./loadBuses.h"
//#include "./rtGetInf.h"
//#include "./so_script.h"
//#include "./verifyTripEnergy.h"
//#include "./loadDrivingSO_initialize.h"
#include "./histc.h"
//#include "./loadDrivingSO_terminate.h"

//using namespace sim_mob;

extern void loadVehicles(struct0_T vehdatabase[10694]);

extern void loadDrivingSO(double moments[438976], double spdBins[38]);

extern double b_getEnergy_TS_3d(double spd[3], const double spdBins[38], const
  double momentMatrix[438976], double vehstruct_a, double vehstruct_b, double
  vehstruct_c, double vehstruct_m, double vehstruct_etaMax, double
  vehstruct_etaBrake, double vehstruct_P_idle, double p_aux);
extern void getEnergy_TS_3d(double spd[3], const double spdBins[38], const
  double momentMatrix[438976], double dt, const struct0_T *vehstruct, double
  p_aux, double *Euse, boolean_T *matched);

extern void so_script(const double speedVec[3], const double meanMomentMatrix3d
                      [438976], const double spdBins[38], struct0_T *vehstruct,
                      double temperature);

extern boolean_T verifyTripEnergy();

