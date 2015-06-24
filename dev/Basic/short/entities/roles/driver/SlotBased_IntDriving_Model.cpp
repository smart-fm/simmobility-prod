//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#include "models/IntersectionDrivingModel.hpp"
#include "DriverUpdateParams.hpp"
#include "Driver.hpp"

using namespace std;
using namespace sim_mob;

SlotBased_IntDriving_Model::SlotBased_IntDriving_Model(DriverUpdateParams& params) :
MITSIM_IntDriving_Model(params), tailgateSeparationTime(0), conflictSeparationTime(0)
{
	modelType = Int_Model_SlotBased;
	initParam(params);
}

SlotBased_IntDriving_Model::~SlotBased_IntDriving_Model()
{	
}

double SlotBased_IntDriving_Model::makeAcceleratingDecision(DriverUpdateParams& params)
{
	
}

void SlotBased_IntDriving_Model::initParam(DriverUpdateParams& params)
{
	string modelName = "general_driver_model";
	bool isAMOD = false;

	//Check if the vehicle is autonomous
	if (params.driver->getParent()->amodId != "-1")
	{
		isAMOD = true;
	}

	//Get the parameter manager instance for the respective type of vehicle (normal or AMOD)
	ParameterManager *parameterMgr = ParameterManager::Instance(isAMOD);

	//Read the parameter values
	parameterMgr->param(modelName, "tailgate_separation_time", tailgateSeparationTime, 1.0);
	parameterMgr->param(modelName, "conflict_separation_time", conflictSeparationTime, 2.5);
}