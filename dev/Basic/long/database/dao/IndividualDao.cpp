//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licenced under of the terms of the MIT licence, as described in the file:
//licence.txt (www.opensource.org\licences\MIT)

/*
 * IndividualDao.cpp
 *
 *  Created on: 3 Sep, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edU>
 */

#include "IndividualDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;


IndividualDao::IndividualDao(DB_Connection& connection): SqlAbstractDao<Individual>(connection, DB_TABLE_INDIVIDUAL, DB_INSERT_INDIVIDUAL, DB_UPDATE_INDIVIDUAL,
																					DB_DELETE_INDIVIDUAL, DB_GETALL_INDIVIDUAL, DB_GETBYID_INDIVIDUAL) {}

IndividualDao::~IndividualDao() {}

void IndividualDao::fromRow(Row& result, Individual& outObj)
{
	outObj.id  					= result.get<BigSerial>(	"id", 					INVALID_ID);
	outObj.jobId				= result.get<BigSerial>(	"job_id", 				INVALID_ID);
	outObj.studentId			= result.get<BigSerial>(	"student_id", 			INVALID_ID);
	outObj.individualTypeId		= result.get<BigSerial>(	"individual_type_id", 	INVALID_ID);
	outObj.householdId			= result.get<BigSerial>(	"household_id", 		INVALID_ID);
	outObj.ethnicityId			= result.get<BigSerial>(	"ethnicity_id", 		INVALID_ID);
	outObj.employmentStatusId	= result.get<BigSerial>(	"employment_status_id", INVALID_ID);
	outObj.genderId				= result.get<BigSerial>(	"gender_id", 			INVALID_ID);
	outObj.educationId			= result.get<BigSerial>(	"education_id", 		INVALID_ID);
	outObj.occupationId			= result.get<BigSerial>(	"occupation_id", 		INVALID_ID);
	outObj.industryId			= result.get<BigSerial>(	"industry_id", 			INVALID_ID);
	outObj.transitCategoryId	= result.get<BigSerial>(	"transit_category_id", 	INVALID_ID);
	outObj.ageCategoryId		= result.get<BigSerial>(	"age_category_id", 		INVALID_ID);
	outObj.residentialStatusId	= result.get<BigSerial>(	"residential_status_id",INVALID_ID);
	outObj.householdHead		= result.get<int>(			"household_head", 		0);
	outObj.income				= result.get<double>(		"income", 				0.0);
	outObj.memberId				= result.get<int>(			"member_id", 			0);
	outObj.carLicense			= result.get<int>(			"car_license", 			0);
	outObj.motorLicense			= result.get<int>(			"motor_license", 		0);
	outObj.vanbusLicense		= result.get<int>(			"vanbus_license", 		0);
	outObj.dateOfBirth			= result.get<std::tm>(		"date_of_birth", 		std::tm());
	outObj.ageDetailedCategory  = result.get<BigSerial>(	"age_detailed_category",INVALID_ID);
	outObj.taxiDriver			= result.get<int>(			"taxi_driver", 		0);
	outObj.fixed_workplace		= result.get<int>(			"fixed_workplace", 	0);
	outObj.fixed_hours			= result.get<int>(			"fixed_hours", 		0);

}

void IndividualDao::toRow(Individual& data, Parameters& outParams, bool update) {}

std::vector<Individual*> IndividualDao::getPrimarySchoolIndividual(std::tm currentSimYear)
{
	//const std::string DB_GETALL_PRIMARY_SCHOOL_INDIVIDUALS = "SELECT * FROM " + APPLY_SCHEMA( MAIN_SCHEMA, "individual") + " WHERE (date_part('year'::text, age(timestamp :v1, date_of_birth::timestamp with time zone))  >= 7 and date_part('year'::text, age(timestamp :v2, date_of_birth::timestamp with time zone)) <=12);";
	db::Parameters params;
	params.push_back(currentSimYear);
	//params.push_back(currentSimYear);
	std::vector<Individual*> primarySchoolIndList;
	getByQueryId(DB_GETALL_PRIMARY_SCHOOL_INDIVIDUALS,params,primarySchoolIndList);
	return primarySchoolIndList;
}

std::vector<Individual*> IndividualDao::getPreSchoolIndividual(std::tm currentSimYear)
{
	//const std::string DB_GETALL_PRE_SCHOOL_INDIVIDUALS = "SELECT * FROM " + APPLY_SCHEMA( MAIN_SCHEMA, "individual") + " WHERE date_part('year'::text, age(timestamp :v1, date_of_birth::timestamp with time zone))  >= 4 and date_part('year'::text, age(timestamp :v2, date_of_birth::timestamp with time zone)) <=6);";
	db::Parameters params;
	params.push_back(currentSimYear);
	//params.push_back(currentSimYear);
	std::vector<Individual*> preSchoolIndList;
	getByQueryId(DB_GETALL_PRE_SCHOOL_INDIVIDUALS,params,preSchoolIndList);
	return preSchoolIndList;

}
