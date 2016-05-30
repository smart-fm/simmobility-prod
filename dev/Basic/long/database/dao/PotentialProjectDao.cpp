/*
 * PotentialProjectDao.cpp
 *
 *  Created on: Dec 17, 2015
 *      Author: gishara
 */
#include "PotentialProjectDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

PotentialProjectDao::PotentialProjectDao(DB_Connection& connection)
: SqlAbstractDao<PotentialProject>(connection, EMPTY_STR,EMPTY_STR, EMPTY_STR, EMPTY_STR,EMPTY_STR, EMPTY_STR) {}

PotentialProjectDao::~PotentialProjectDao() {}

void PotentialProjectDao::fromRow(Row& result, PotentialProject& outObj)
{
    outObj.fmParcelId = result.get<BigSerial>( "fm_parcel_id", INVALID_ID );
    outObj.profit = result.get<double>( "profit", .0 );
    outObj.constructionCost = result.get<double>( "construction_cost", .0 );
    outObj.demolitionCost = result.get<double>("demolition_cost", .0 );
    outObj.grossArea = result.get<int>( "gross_area", .0 );
    outObj.investmentReturnRatio = result.get<double>( "investment_return_ratio", .0 );
    outObj.totalUnits = result.get<int>( "total_units", 0 );
    outObj.acquisitionCost = result.get<double>( "acquisition_cost", .0 );
    outObj.simulationDate = result.get<std::tm>("simulation_date", std::tm());
}

void PotentialProjectDao::toRow(PotentialProject& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getFmParcelId());
	outParams.push_back(data.getProfit());
	outParams.push_back(data.getConstructionCost());
	outParams.push_back(data.getDemolitionCost());
	outParams.push_back(data.getGrosArea());
	outParams.push_back(data.getInvestmentReturnRatio());
	outParams.push_back(data.getTotalUnits());
	outParams.push_back(data.getAcquisitionCost());
	outParams.push_back(data.getSimulationDate());
}

void PotentialProjectDao::insertPotentialProject(PotentialProject& potentialProject,std::string schema)
{

	const std::string DB_INSERT_POTENTIAL_PROJECT = "INSERT INTO " + APPLY_SCHEMA(schema, ".potential_project")
										+ " (" + "fm_parcel_id" + ", " + "profit" + ", " + "construction_cost"
				                		+ ", " + "demolition_cost" + ", " + "gross_area" + ", "
				                		+ "investment_return_ratio" + ", " + "total_units"  + ", "+ "acquisition_cost" + ", " + "land_value" + ", "
				                		+ "simulation_date"
				                		+ ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7, :v8, :v9, :v10)";
	insertViaQuery(potentialProject,DB_INSERT_POTENTIAL_PROJECT);

}




