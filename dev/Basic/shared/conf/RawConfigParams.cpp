//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RawConfigParams.hpp"

using namespace sim_mob;

sim_mob::RawConfigParams::RawConfigParams() : mergeLogFiles(false), generateBusRoutes(false), simMobRunMode(RawConfigParams::UNKNOWN_RUN_MODE),
		subTripLevelTravelTimeOutput(std::string()), subTripTravelTimeEnabled(false)
{}

sim_mob::SimulationParams::SimulationParams() :
    baseGranMS(0), baseGranSecond(0), totalRuntimeMS(0), totalWarmupMS(0), inSimulationTTUsage(0),
    workGroupAssigmentStrategy(WorkGroup::ASSIGN_ROUNDROBIN), startingAutoAgentID(0),
    mutexStategy(MtxStrat_Buffered)
{}


sim_mob::LongTermParams::LongTermParams(): enabled(false), workers(0), days(0), tickStep(0), maxIterations(0),year(0),simulationScenario(std::string()),resume(false),currentOutputSchema(std::string()),mainSchemaVersion(std::string()),configSchemaVersion(std::string()),calibrationSchemaVersion(std::string()),geometrySchemaVersion(std::string()),opSchemaloadingInterval(0){}
sim_mob::LongTermParams::DeveloperModel::DeveloperModel(): enabled(false), timeInterval(0), initialPostcode(0),initialUnitId(0),initialBuildingId(0),initialProjectId(0),minLotSize(0) {}
sim_mob::LongTermParams::HousingModel::HousingModel(): enabled(false), timeInterval(0), timeOnMarket(0), timeOffMarket(0), vacantUnitActivationProbability(0),
													   housingMarketSearchPercentage(0), housingMoveInDaysInterval(0), offsetBetweenUnitBuyingAndSelling(0),
													   bidderUnitsChoiceSet(0),bidderBTOUnitsChoiceSet(0),householdBiddingWindow(0), householdAwakeningPercentageByBTO(0),offsetBetweenUnitBuyingAndSellingAdvancedPurchase(0){}

sim_mob::LongTermParams::HousingModel::AwakeningModel::AwakeningModel(): initialHouseholdsOnMarket(0), dailyHouseholdAwakenings(0), awakenModelJingsi(false), awakenModelShan(false), awakenModelRandom(false), awakeningOffMarketSuccessfulBid(0), awakeningOffMarketUnsuccessfulBid(0){}

sim_mob::LongTermParams::OutputHouseholdLogsums::OutputHouseholdLogsums():enabled(false), fixedHomeVariableWork(false), fixedWorkVariableHome(false), vehicleOwnership(false){}

sim_mob::LongTermParams::VehicleOwnershipModel::VehicleOwnershipModel():enabled(false), vehicleBuyingWaitingTimeInDays(0){}
sim_mob::LongTermParams::TaxiAccessModel::TaxiAccessModel():enabled(false){}
sim_mob::LongTermParams::SchoolAssignmentModel::SchoolAssignmentModel():enabled(false), schoolChangeWaitingTimeInDays(0){}

ModelScriptsMap::ModelScriptsMap(const std::string& scriptFilesPath, const std::string& scriptsLang) : path(scriptFilesPath), scriptLanguage(scriptsLang) {}
