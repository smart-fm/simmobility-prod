#include "UnPackageUtils.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "conf/simpleconf.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DynamicVector.hpp"
#include "util/DailyTime.hpp"
#include "geospatial/Point2D.hpp"

namespace sim_mob {

UnPackageUtils::UnPackageUtils(std::string data)
{
	buffer << data;
	package = new boost::archive::text_iarchive(buffer);
}

UnPackageUtils::~UnPackageUtils()
{
	buffer.clear();
	if (package) {
		delete package;
		package = NULL;
	}
}


FixedDelayed<DPoint*>& UnPackageUtils::unpackFixedDelayedDPoint() const {
	int delayMS;
	(*package) & delayMS;

	bool reclaimPtrs;
	(*package) & reclaimPtrs;

	FixedDelayed<DPoint*>* one_delay = new FixedDelayed<DPoint*> (delayMS, reclaimPtrs);

	int list_size;
	(*package) & list_size;

	for (int i = 0; i < list_size; i++) {
		DPoint value;
		int value2;

		(*package) & value;
		(*package) & value2;

		DPoint* buffer_value = new DPoint(value.x, value.y);
		uint32_t ut_value = value2;

		FixedDelayed<DPoint*>::HistItem one(buffer_value, ut_value);
		one_delay->history.push_back(one);
	}

	return (*one_delay);
}

FixedDelayed<double>& UnPackageUtils::unpackFixedDelayedDouble() const {
	int delayMS;
	(*package) & delayMS;

	bool reclaimPtrs;
	(*package) & reclaimPtrs;

	FixedDelayed<double>* one_delay = new FixedDelayed<double> (delayMS, reclaimPtrs);

	int list_size;
	(*package) & list_size;

	for (int i = 0; i < list_size; i++) {
		double value;
		int value2;

		(*package) & value;
		(*package) & value2;

		uint32_t ut_value = value2;

		FixedDelayed<double>::HistItem one(value, ut_value);
		one_delay->history.push_back(one);
	}

	return (*one_delay);
}

FixedDelayed<int>& UnPackageUtils::unpackFixedDelayedInt() const {
	int delayMS;
	(*package) & delayMS;

	bool reclaimPtrs;
	(*package) & reclaimPtrs;

	FixedDelayed<int>* one_delay = new FixedDelayed<int> (delayMS, reclaimPtrs);

	int list_size;
	(*package) & list_size;

	for (int i = 0; i < list_size; i++) {
		int value;
		int value2;

		(*package) & value;
		(*package) & value2;

		uint32_t ut_value = value2;

		FixedDelayed<int>::HistItem one(value, ut_value);
		one_delay->history.push_back(one);
	}

	return (*one_delay);
}

Point2D* UnPackageUtils::unpackPoint2D() const {
	Point2D* one_point = new Point2D(0, 0);
	(*package) & (*one_point);

	return one_point;
}


void UnPackageUtils::unpackDailyTime(DailyTime& time) const {
	(*package) & time;
}

void UnPackageUtils::unpackDPoint(DPoint& point) const
{
	(*package) & (point);
}

void UnPackageUtils::unpackDynamicVector(DynamicVector& vector) const
{
	(*package) & vector;
}

}
#endif
