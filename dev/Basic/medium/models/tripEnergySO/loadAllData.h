#ifndef LOADALLDATA_H
#define LOADALLDATA_H
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "./rt_nonfinite.h"
#include "./rtGetNaN.h"
#include "./rtwtypes.h"
#include "./TripEnergy_SO_types.h"

extern void loadAllData(double moments[438976], double spdBins[38], struct0_T
  *vehs);

#endif
