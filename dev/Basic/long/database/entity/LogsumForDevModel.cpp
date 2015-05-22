/*
 * LogsumForDevModel.cpp
 *
 *  Created on: May 13, 2015
 *      Author: gishara
 */

#include "LogsumForDevModel.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

LogsumForDevModel::LogsumForDevModel(BigSerial fmParcelId, BigSerial tazId, double accessibility): fmParcelId(fmParcelId), tazId(tazId), accessibility(accessibility){}

LogsumForDevModel::~LogsumForDevModel() {}

BigSerial LogsumForDevModel::getFmParcelId() const
{
	return fmParcelId;
}

BigSerial LogsumForDevModel::getTazId() const
{
    return tazId;
}

double LogsumForDevModel::getAccessibility() const
{
    return accessibility;
}

//namespace sim_mob {
//    namespace long_term {
//
//        std::ostream& operator<<(std::ostream& strm, const LogsumForDevModel& data) {
//            return strm << "{"
//            		<< "\"fm_parcel_id\":\"" << data.fmParcelId << "\","
//                    << "\"taz_id\":\"" << data.tazId << "\","
//                    << "\"accessibility\":\"" << data.accessibility << "\","
//                    << "}";
//        }
//    }
//}



