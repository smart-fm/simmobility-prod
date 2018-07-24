//Copyright (c) 2018 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT

/*
 * ScreeningModelCoefficients.cpp
 *
 *  Created on: 23 July 2018
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/ScreeningModelFactors.hpp>

using namespace sim_mob::long_term;


ScreeningModelFactors::ScreeningModelFactors(  BigSerial _id, int _dgpid, int _alt_id, int _dwl, double _f_loc_com, double _f_loc_res,
                        double _f_loc_open, double _odi10_loc, double _dis2mrt, double _dis2exp, double _time_ave,
                        double _cost_ave, double _acc_t_mfg, double _acc_t_off):  id(_id),  dgpid(_dgpid),  alt_id(_alt_id),  dwl(_dwl),  f_loc_com(_f_loc_com),  
                        f_loc_res(_f_loc_res), f_loc_open(_f_loc_open),  odi10_loc(_odi10_loc),  dis2mrt(_dis2mrt),  dis2exp(_dis2exp),  time_ave(_time_ave),
                        cost_ave(_cost_ave),  acc_t_mfg(_acc_t_mfg), acc_t_off(_acc_t_off) {}


ScreeningModelFactors::~ScreeningModelFactors(){}


BigSerial ScreeningModelFactors::getId()
{
    return id;
}

int ScreeningModelFactors::getDgpid()
{
    return dgpid;
}


int ScreeningModelFactors::getAlt_id()
{
    return alt_id;
}

int ScreeningModelFactors::getDwl()
{
    return dwl;
}

double ScreeningModelFactors::getF_loc_com()
{
    return f_loc_com;
}

double ScreeningModelFactors::getF_loc_res()
{
    return f_loc_res;
}

double ScreeningModelFactors::getF_loc_open()
{
    return f_loc_open;
}

double ScreeningModelFactors::getOdi10_loc()
{
    return odi10_loc;
}

double ScreeningModelFactors::getDis2mrt()
{
    return dis2mrt;
}

double ScreeningModelFactors::getDis2exp()
{
    return dis2exp;
}

double ScreeningModelFactors::getTime_ave()
{
    return time_ave;
}

double ScreeningModelFactors::getCost_ave()
{
    return cost_ave;
}

double ScreeningModelFactors::getAcc_t_mfg()
{
    return acc_t_mfg;
}

double ScreeningModelFactors::getAcc_t_off()
{
    return acc_t_off;
}

