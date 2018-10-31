//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: getEnergy_TS_3d.h
//
// MATLAB Coder version            : 3.2
// C/C++ source code generated on  : 06-Jan-2018 13:15:08
//
#ifndef GETENERGY_TS_3D_H
#define GETENERGY_TS_3D_H

// Include Files
#include <cmath>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
//#include "rtGetInf.h"
//#include "rtGetNaN.h"
//#include "rt_nonfinite.h"
#include "rtwtypes.h"
#include "loadDrivingSO_types.h"

// Function Declarations
extern double b_getEnergy_TS_3d(double spd[3], const double spdBins[38], const
  double momentMatrix[438976], double vehstruct_a, double vehstruct_b, double
  vehstruct_c, double vehstruct_m, double vehstruct_etaMax, double
  vehstruct_etaBrake, double vehstruct_P_idle, double p_aux);
extern void getEnergy_TS_3d(double spd[3], const double spdBins[38], const
  double momentMatrix[438976], double dt, const struct0_T *vehstruct, double
  p_aux, double *Euse, boolean_T *matched);

#endif

//
// File trailer for getEnergy_TS_3d.h
//
// [EOF]
//
