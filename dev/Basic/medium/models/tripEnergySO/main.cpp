#include "../tripEnergySO/main.h"

#include <iostream>
#include "./rt_nonfinite.h"
#include "./loadAllData.h"
#include "./so_script_struct.h"
#include "./TripEnergy_SO_initialize.h"
#include "./TripEnergy_SO_terminate.h"

//static void main_so_script_struct();


double main_so_script_struct(double spd[3], int vehtype)
{
  double output;
  static double moments[438976];
  static double spdBins[38];
  static struct0_T vehs;
  double totE = 0;
  double j2kwh = 2.77778e-7;// convert joules to kWh
  loadAllData(moments, spdBins, &vehs);
    for (int ii = 0; ii < 100000; ii++){
        output = so_script_struct(spd, vehtype, moments, spdBins, &vehs);
        totE += output*j2kwh;
    }
    return totE;
}

int main(int, const char * const [])
{
    double spd[3] = {16.0, 20.0, 22.4};
    int vehtype = 4;
    double output;
    TripEnergy_SO_initialize();
    output = main_so_script_struct(spd, vehtype);
    std::cout << "it worked!!\n";
    std::cout << output;
    std::cout << "\n";
    TripEnergy_SO_terminate();
    return 0;
}
