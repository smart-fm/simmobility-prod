//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PopulationMongoDao.cpp
 *
 *  Created on: Nov 28, 2013
 *      Author: Harish Loganathan
 */

#include "PopulationMongoDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob;
using namespace medium;

sim_mob::medium::PopulationMongoDao::PopulationMongoDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection)
: db::MongoDao(dbConfig, database, collection)
{}

sim_mob::medium::PopulationMongoDao::~PopulationMongoDao()
{}

void PopulationMongoDao::fromRow(mongo::BSONObj document, PersonParams& outParam) {
  	outParam.setPersonId(document.getField(MONGO_FIELD_ID).String());
  	outParam.setHhId(document.getField(MONGO_FIELD_HOUSEHOLD_ID).String());
   	outParam.setIncomeId(document.getField(MONGO_FIELD_INCOME_ID).Int());
   	outParam.setMissingIncome(document.getField(MONGO_FIELD_MISSING_INCOME).Int());
   	outParam.setPersonTypeId(document.getField(MONGO_FIELD_PERSON_TYPE_ID).Int());
   	outParam.setAgeId(document.getField(MONGO_FIELD_AGE_CATEGORY_ID).Int());
   	outParam.setWorksAtHome(document.getField(MONGO_FIELD_WORK_AT_HOME).Int());
   	outParam.setHasDrivingLicence(document.getField(MONGO_FIELD_DRIVER_LICENCE).Int());
  	outParam.setStudentTypeId(document.getField(MONGO_FIELD_STUDENT_TYPE_ID).Int());
  	outParam.setIsUniversityStudent(document.getField(MONGO_FIELD_UNIVERSITY_STUDENT).Int());
   	outParam.setIsFemale(document.getField(MONGO_FIELD_FEMALE).Int());
   	outParam.setHomeLocation(document.getField(MONGO_FIELD_HOME_MTZ).Int());
   	outParam.setFixedWorkLocation(document.getField(MONGO_FIELD_WORK_MTZ).Int());
   	outParam.setFixedSchoolLocation(document.getField(MONGO_FIELD_SCHOOL_MTZ).Int());
   	outParam.setHH_OnlyAdults(document.getField(MONGO_FIELD_HH_ONLY_ADULTS).Int());
  	outParam.setHH_OnlyWorkers(document.getField(MONGO_FIELD_HH_ONLY_WORKERS).Int());
   	outParam.setHH_NumUnder4(document.getField(MONGO_FIELD_HH_NUM_UNDER_4).Int());
   	outParam.setHH_HasUnder15(document.getField(MONGO_FIELD_HH_UNDER_15).Int());
   	outParam.setCarOwn(document.getField(MONGO_FIELD_CAR_OWN).Int());
   	outParam.setCarOwnNormal(document.getField(MONGO_FIELD_CAR_OWN_NORMAL).Int());
   	outParam.setCarOwnOffpeak(document.getField(MONGO_FIELD_CAR_OWN_OFFPEAK).Int());
   	outParam.setMotorOwn(document.getField(MONGO_FIELD_MOTOR_OWN).Int());
   	outParam.setWorkLogSum(document.getField(MONGO_FIELD_WORK_LOGSUM).Number());
   	outParam.setEduLogSum(document.getField(MONGO_FIELD_EDU_LOGSUM).Number());
   	outParam.setShopLogSum(document.getField(MONGO_FIELD_SHOP_LOGSUM).Number());
   	outParam.setOtherLogSum(document.getField(MONGO_FIELD_OTHER_LOGSUM).Number());
   	outParam.setDptLogsum(document.getField(MONGO_FIELD_DPT_LOGSUM).Number());
   	outParam.setDpsLogsum(document.getField(MONGO_FIELD_DPS_LOGSUM).Number());
   	outParam.setHouseholdFactor(document.getField(MONGO_FIELD_HH_FACTOR).Number());
   	outParam.setHasFixedWorkTiming(document.getField(MONGO_FIELD_WORK_TIME_FLEX).Int());
}

std::string PopulationMongoDao::getIdFromRow(mongo::BSONObj document)
{
	return document.getField(MONGO_FIELD_ID).String();
}

bool sim_mob::medium::PopulationMongoDao::getOneById(const std::string& id, PersonParams& outParam)
{
	mongo::BSONObj qry = BSON("_id" << id);
	mongo::BSONObj fetchedDocument;
	getOne(qry, fetchedDocument);
	fromRow(fetchedDocument, outParam);
	return true;
}
