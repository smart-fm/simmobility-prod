/*
 * ParcelsWithHDBDao.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: gishara
 */

#include "ParcelsWithHDBDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ParcelsWithHDBDao::ParcelsWithHDBDao(DB_Connection& connection)
: SqlAbstractDao<ParcelsWithHDB>(connection, "","", "", "","SELECT * FROM " + connection.getSchema()+"getParcelsWithHDB()", "") {}

ParcelsWithHDBDao::~ParcelsWithHDBDao() {}

void ParcelsWithHDBDao::fromRow(Row& result, ParcelsWithHDB& outObj)
{
    outObj.fmParcelId = result.get<BigSerial>( "fm_parcel_id", INVALID_ID );
    outObj.unitTypeId = result.get<int>("unit_type_id", 0);
}

void ParcelsWithHDBDao::toRow(ParcelsWithHDB & data, Parameters& outParams, bool update) {}




