//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "EnergyModelBase.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;

const double metersToMiles = 1/1609.34;
const double secondsToMinutes = 1/60.0;
const double joulesToGallons = 1/131760000.0;

namespace {
sim_mob::OneTimeFlag titleEnergy;
}

EnergyModelBase::EnergyModelBase() :
		modelType("simple"), energyModelOutputFile("energy_output.csv"), paramsMapping()
{
}

EnergyModelBase::~EnergyModelBase()
{
}


//void EnergyModelBase::computeEnergyWithSpeedHolder(const std::deque<double> speedCollector, struct0_T &vehicleStruct, const double timeStep)
void EnergyModelBase::computeEnergyWithSpeedHolder(const std::deque<double> speedCollector, struct0_T &vehicleStruct, const double timeStep, const int occupancy)
{
}

//std::pair<double,double> EnergyModelBase::computeEnergyWithVelocityVector( //const VehicleParams& vp, //jo Mar13
//		const std::vector<double> velocityVector, double timeStep, std::string vehicleTypeStr)
//{
//}
////}jo

void EnergyModelBase::computeTrainEnergyWithSpeed(const double trainMovement, struct0_T &vehicleStruct, const double timeStep)
{
}


struct0_T EnergyModelBase::initVehicleStruct(const std::string drivetrain)
{
}


// NOTE: jo Mar13
// Key difference between SimpleE and TripE is that, currently, SimpleE computes energy using entire velocity vector,
// while TripE does this every time step, using 3 speed elements...
// TODO: clean up; virtualize and overload in respective energy models! jo - Mar13
void EnergyModelBase::onTripCompletion(const DriverMovement* dM, const Person_MT* p,
		const std::vector<double>& velocityVector, double timeStep) // jo - Mar13
{
	const double gasolineGE = 1/33000.0;
	const Trip *trip = (static_cast<Trip*> (*p->currTripChainItem));
	//std::string
	std::string vehicleTypeStr; // = trip->getVehicleType();

	double tripD = dM->getTotalDistance();
	tripD = tripD*0.001; //metersToMiles;
	double tripT = dM->getTotalTime();
	tripT = tripT*secondsToMinutes;

	double tripE;
	tripE = p->getPersonInfo().getConstVehicleParams().getTotalEnergy();
	double tripEco;
	std::string outputHeader;

	//VehicleParams vp;

	sim_mob::SubTrip &st = *(p->currSubTrip);
	DailyTime starttime = st.startTime;
	std::string startTimeString =  starttime.getStrRepr();
	//std::cout << "NEW ENERGY: " << dM->getNewTotalEnergy() << std::endl;
	//std::cout << "Trip Completed. D = " << tripD << ", T = " << tripT*60 << ", E = " << tripE <<  ", MPG = " << tripMPG << std::endl;

	std::stringstream ret("");
	if (modelType == "simple")
	{
		vehicleTypeStr = getVehicleTypeString(p->getPersonInfo().getConstVehicleParams());
		ret <<
				p->getDatabaseId() << "," <<
				//vp.getVehicleId() << "," << // jo - Mar13 - until this can be fixed
				vehicleTypeStr << "," <<
				trip->getPersonID() << "," <<
				st.tripID << "," <<
				trip->originZoneCode << "," <<
				trip->destinationZoneCode << "," <<
				startTimeString << "," <<
				tripD << "," <<
				tripT << "," <<
				//dM->getTrajectoryInfo().totalTimeSlow << "," << dM->getTrajectoryInfo().totalTimeFast << "," <<
				tripE  << "," <<
				"0" << std::endl;
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,distance,time,kWh,GGE\n";
	}
	else if (modelType == "tripenergy")
	{
		vehicleTypeStr = trip->getVehicleType();
		ret <<
				p->getDatabaseId() << "," <<
				vehicleTypeStr << "," <<
				trip->getPersonID() << "," <<
				st.tripID << "," <<
				trip->originZoneCode << "," <<
				trip->destinationZoneCode << "," <<
				startTimeString << "," <<
				tripD << "," <<
				tripT << "," <<
				dM->getTrajectoryInfo().totalTimeSlow << "," <<
				dM->getTrajectoryInfo().totalTimeFast << "," <<
				tripE  << "," <<
				tripEco << std::endl;
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,D,T,T_lt_10,T_gt_25,energy,MPG\n";
	}
	sim_mob::BasicLogger& csv = sim_mob::Logger::log(getEnergyModelOutputFile());
	if (titleEnergy.check())
	{
		csv << outputHeader;
	}
	csv << ret.str();
}

void EnergyModelBase::onBusTripCompletion(const DriverMovement* dM, const Person_MT* busDriver,
		const std::vector<double>& velocityVector, double timeStep) // jo - Mar13
{
	const BusTrip* busTrip = dynamic_cast<const BusTrip*>(*(busDriver->currTripChainItem));
	//const std::string& busLineID = busTrip->getBusLine()->getBusLineID();

	std::string vehicleTypeStr; // = trip->getVehicleType();

	double tripD = dM->getTotalDistance();
	tripD = tripD*0.001; //metersToMiles;
	double tripT = dM->getTotalTime();
	tripT = tripT*secondsToMinutes;

	//jo
	double tripE; //trip Energy consumption
	tripE = busDriver->getPersonInfo().getConstVehicleParams().getTotalEnergy();
	double tripEco; //trip Fuel Economy
	std::string outputHeader;

	DailyTime starttime = busTrip->startTime;
	std::string startTimeString =  starttime.getStrRepr();
	std::stringstream ret("");
	if (modelType == "simple")
	{
		vehicleTypeStr = getVehicleTypeString(busDriver->getPersonInfo().getConstVehicleParams());
		ret << //p->GetId() << "," <<	// ind_id
				busTrip->getBusLine()->getBusLineID() << "," << // ind_id_db
				"BUS" << "," << // veh_type
				busTrip->tripID  << "," << // trip_id
				busTrip->getVehicleID() << "," << // subtrip_id
				"0" << "," << // originzonecode (not implemented for bus)
				"0" << "," << // destinationzonecode (not implemented for bus)
				startTimeString << "," << // Trip Start Time (from sim start)
				tripD << "," << // Distance
				tripT << "," << // Time
				// dM->getTrajectoryInfo().totalTimeSlow << "," << // Time at low speed
				// dM->getTrajectoryInfo().totalTimeFast << "," << // Time at high speed
				tripE  << "," <<  // energy
				"0" << std::endl; // mpg
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,distance,time,Wh,GGE\n";
	}
	else if (modelType == "tripenergy")
	{
		tripEco = tripD/(tripE*joulesToGallons); // MPG for TripEnergy model
		ret << //p->GetId() << "," <<	// ind_id
				busTrip->getVehicleID() << "," << // ind_id_db
				"BUS" << "," << // veh_type
				busTrip->getMode() << "_" << "1" << "," << // trip_id
				"0" << "," << // subtrip_id
				"0" << "," << // originzonecode (not implemented for bus)
				"0" << "," << // destinationzonecode (not implemented for bus)
				"0:00:00" << "," << // Trip Start Time (from sim start)
				tripD << "," << // Distance
				tripT << "," << // Time
				dM->getTrajectoryInfo().totalTimeSlow << "," << // Time at low speed
				dM->getTrajectoryInfo().totalTimeFast << "," << // Time at high speed
				tripE  << "," <<  // energy
				tripEco << std::endl; // mpg
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,D,T,T_lt_10,T_gt_25,energy,MPG\n";
	}

	sim_mob::BasicLogger& csv = sim_mob::Logger::log(getEnergyModelOutputFile());
	if (titleEnergy.check())
	{
		csv << outputHeader ;
	}
	csv << ret.str();
}

void EnergyModelBase::onTrainTripCompletion(//const TrainMovement* tM, //jo - Mar27 - taken out for now, until we can fix!
		const trajectory_info_t& trajectory, const Person_MT* p,
		const std::vector<double>& velocityVector, double timeStep, int occupancy) // jo - Mar13
{
	const TrainTrip* trainTrip = dynamic_cast<const TrainTrip*>(*(p->currTripChainItem));
	//const std::string& trainLineID = trainTrip->getLineId();
	std::string vehicleTypeStr; // = trip->getVehicleType();

	double tripD = trajectory.totalDistanceDriven; //tM->getTotalDistance(); //jo - Mar27 until we can fix
	tripD = tripD*0.001;
	double tripT = trajectory.totalTimeDriven; //tM->getTotalTime(); //jo - Mar27 until we can fix
	tripT = tripT*secondsToMinutes;

	//jo
	double tripE; //trip Energy consumption
	double tripEco =0; //trip Fuel Economy
	std::string outputHeader;
	tripE = p->getPersonInfo().getConstVehicleParams().getTotalEnergy();

//	else if (modelType == "tripenergy")
//	{
//		tripE = busDriver->getPersonInfo().getConstVehicleParams().getTotalEnergy();
//		tripEco = tripD/(tripE*joulesToGallons); // MPG for TripEnergy model
//	}
	//jo
	// std::cout << "TRAIN Trip Completed. D = " << tripD << ", T = " << tripT*60 << ", E = " << tripE <<  ", MPG = " << tripMPG << std::endl;
	//DailyTime starttime = trainTrip->getStartTime();
	std::string startTimeString = trainTrip->getStartTimeString(); //starttime.getStrRepr();
	std::stringstream ret("");
	if (modelType == "simple")
	{
		vehicleTypeStr = getVehicleTypeString(p->getPersonInfo().getConstVehicleParams());
		ret << //p->GetId() << "," <<	// ind_id
				trainTrip->getLineId() << "," << // ind_id_db
				vehicleTypeStr << "," << // veh_type
				trainTrip->getTripId() << "_" << "1" << "," << // trip_id
				trainTrip->getTrainId() << "," << // subtrip_id
				"0" << "," << // originzonecode (not implemented for bus)
				"0" << "," << // destinationzonecode (not implemented for bus)
				startTimeString << "," << // Trip Start Time (from sim start)
				tripD << "," << // Distance
				tripT << "," << // Time
				// dM->getTrajectoryInfo().totalTimeSlow << "," << // Time at low speed
				// dM->getTrajectoryInfo().totalTimeFast << "," << // Time at high speed
				tripE  << "," <<  // energy
				"0" << std::endl; // mpg
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,distance,time,kWh,GGE\n";
	}
	else if (modelType == "tripenergy")
	{
//		ret << //p->GetId() << "," <<	// ind_id
//				busTrip->getVehicleID() << "," << // ind_id_db
//				"BUS" << "," << // veh_type
//				busTrip->getMode() << "_" << "1" << "," << // trip_id
//				"0" << "," << // subtrip_id
//				"0" << "," << // originzonecode (not implemented for bus)
//				"0" << "," << // destinationzonecode (not implemented for bus)
//				"0:00:00" << "," << // Trip Start Time (from sim start)
//				tripD << "," << // Distance
//				tripT << "," << // Time
//				dM->getTrajectoryInfo().totalTimeSlow << "," << // Time at low speed
//				dM->getTrajectoryInfo().totalTimeFast << "," << // Time at high speed
//				tripE  << "," <<  // energy
//				tripEco << std::endl; // mpg
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,D,T,T_lt_10,T_gt_25,energy,MPG\n";
	}

	sim_mob::BasicLogger& csv = sim_mob::Logger::log(getEnergyModelOutputFile());
	if (titleEnergy.check())
	{
		csv << outputHeader ;
	}
	csv << ret.str();
}


// NOTE: jo Apr4
// TODO: clean up; virtualize and overload in respective energy models! jo -
void EnergyModelBase::onOnCallTripCompletion(const DriverMovement* dM, const Person_MT* onCallDriver,
		const std::vector<double>& velocityVector, double timeStep) // jo - Mar13
{
	const Trip *trip = (static_cast<Trip*> (*onCallDriver->currTripChainItem));
	std::string vehicleTypeStr; // = trip->getVehicleType();

	double tripD = dM->getTotalDistance();
	tripD = tripD*0.001; //meters to km;
	double tripT = dM->getTotalTime();
	tripT = tripT*secondsToMinutes;

	double tripE;
	double tripEco;
	tripEco = tripD/(tripE*joulesToGallons); // MPG for TripEnergy model
	tripE = onCallDriver->getPersonInfo().getConstVehicleParams().getTotalEnergy(); // in joules for TripE

	std::string outputHeader;

	if (modelType == "simple")
	{
		vehicleTypeStr = getVehicleTypeString(onCallDriver->getPersonInfo().getConstVehicleParams());
	}

	//sim_mob::SubTrip &st = *(onCallDriver->currSubTrip);
	DailyTime starttime = trip->startTime;
	std::string startTimeString =  starttime.getStrRepr();

	std::stringstream ret("");
	if (modelType == "simple")
	{
		ret <<
				onCallDriver->getDatabaseId() << "," <<
				// vp.getVehicleId() << "," << // jo - Mar13 - until this can be fixed
				"SMS" << "," <<
				onCallDriver->getDatabaseId() << "," <<
				"0" << "," <<
				"0" << "," <<
				"0" << "," <<
				startTimeString << "," <<
				tripD << "," <<
				tripT << "," <<
				//dM->getTrajectoryInfo().totalTimeSlow << "," << dM->getTrajectoryInfo().totalTimeFast << "," <<
				tripE  << "," <<
				"0" << std::endl;
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,distance,time,kWh,GGE\n";
	}
	else if (modelType == "tripenergy")
	{
		ret <<
				onCallDriver->getDatabaseId() << "," <<
				vehicleTypeStr << "," <<
				trip->getPersonID() << "," <<
				"0" << "," <<
				trip->originZoneCode << "," <<
				trip->destinationZoneCode << "," <<
				"00:00:00" << "," <<
				tripD << "," <<
				tripT << "," <<
				dM->getTrajectoryInfo().totalTimeSlow << "," <<
				dM->getTrajectoryInfo().totalTimeFast << "," <<
				tripE  << "," <<
				tripEco << std::endl;
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,D,T,T_lt_10,T_gt_25,energy,MPG\n";
	}
	sim_mob::BasicLogger& csv = sim_mob::Logger::log(getEnergyModelOutputFile());
	if (titleEnergy.check())
	{
		csv << outputHeader;
	}
	csv << ret.str();
}


void EnergyModelBase::onOnHailTripCompletion(const DriverMovement* dM, const Person_MT* taxiDriver,
		const std::vector<double>& velocityVector, double timeStep) // jo - Mar13
{
	const Trip *trip = (static_cast<Trip*> (*taxiDriver->currTripChainItem));
	std::string vehicleTypeStr; // = trip->getVehicleType();

	double tripD = dM->getTotalDistance();
	tripD = tripD*0.001; //meters to km
	double tripT = dM->getTotalTime();
	tripT = tripT/60.0; // seconds to minutes

	double tripE;
	tripE = taxiDriver->getPersonInfo().getConstVehicleParams().getTotalEnergy(); // in joules for TripE

	double tripEco;
	//tripEco = tripD/(tripE*joulesToGallons); // MPG for TripEnergy model

	std::string outputHeader;

	DailyTime starttime = trip->startTime;
	std::string startTimeString =  starttime.getStrRepr();
	std::stringstream ret("");
	if (modelType == "simple")
	{
		vehicleTypeStr = getVehicleTypeString(taxiDriver->getPersonInfo().getConstVehicleParams());
		ret <<
				taxiDriver->getDatabaseId() << "," <<
				// vp.getVehicleId() << "," << // jo - Mar13 - until this can be fixed
				"TAXI" << "," <<
				taxiDriver->getDatabaseId() << "," <<
				"0" << "," <<
				trip->originZoneCode << "," <<
				trip->destinationZoneCode << "," <<
				startTimeString << "," <<
				tripD << "," <<
				tripT << "," <<
				//dM->getTrajectoryInfo().totalTimeSlow << "," << dM->getTrajectoryInfo().totalTimeFast << "," <<
				tripE  << "," <<
				"0" << std::endl;
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,distance,time,kWh,GGE\n";
	}
	else if (modelType == "tripenergy")
	{
		ret <<
				taxiDriver->getDatabaseId() << "," <<
				vehicleTypeStr << "," <<
				trip->getPersonID() << "," <<
				"0" << "," <<
				trip->originZoneCode << "," <<
				trip->destinationZoneCode << "," <<
				"00:00:00" << "," <<
				tripD << "," <<
				tripT << "," <<
				dM->getTrajectoryInfo().totalTimeSlow << "," <<
				dM->getTrajectoryInfo().totalTimeFast << "," <<
				tripE  << "," <<
				"0" << std::endl; //tripEco formerly
		outputHeader = "ind_id,veh_type,trip_id,subtrip_id,fromNode,toNode,start_time,D,T,T_lt_10,T_gt_25,energy,MPG\n";
	}
	sim_mob::BasicLogger& csv = sim_mob::Logger::log(getEnergyModelOutputFile());
	if (titleEnergy.check())
	{
		csv << outputHeader;
	}
	csv << ret.str();
}
