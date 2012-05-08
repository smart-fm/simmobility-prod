#include "GenConfig.h"

#include "PackageUtils.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "util/GeomHelpers.hpp"
#include "partitions/PartitionManager.hpp"
#include "util/DailyTime.hpp"
#include "util/DynamicVector.hpp"
#include "geospatial/Point2D.hpp"

namespace sim_mob {
std::string PackageUtils::getPackageData() {
	return buffer.str().data();
}

PackageUtils::PackageUtils()
{
	package = new boost::archive::text_oarchive(buffer);
}

PackageUtils::~PackageUtils()
{
	buffer.clear();
	if (package) {
		delete package;
		package = NULL;
	}
}

void PackageUtils::packFixedDelayedDPoint(const FixedDelayed<DPoint*>& one_delay) {
	//(*package) & (one_delay.delayMS);
	(*package) & (one_delay.reclaimPtrs);

	int list_size = one_delay.history.size();
	(*package) & list_size;

	std::list<FixedDelayed<DPoint*>::HistItem>::const_iterator itr = one_delay.history.begin();
	for (; itr != one_delay.history.end(); itr++) {
		FixedDelayed<DPoint*>::HistItem one = (*itr);
		DPoint* value = one.item;
		int value2 = one.observedTime;

		(*package) & (*value);
		(*package) & value2;
	}
}

void PackageUtils::packFixedDelayedDouble(const FixedDelayed<double>& one_delay) {
	//(*package) & (one_delay.delayMS);
	(*package) & (one_delay.reclaimPtrs);


	int list_size = one_delay.history.size();
	(*package) & list_size;

	std::list<FixedDelayed<double>::HistItem>::const_iterator itr = one_delay.history.begin();
	for (; itr != one_delay.history.end(); itr++) {
		FixedDelayed<double>::HistItem one = (*itr);
		double value = one.item;
		int value2 = one.observedTime;

		(*package) & (value);
		(*package) & value2;
	}
}

void PackageUtils::packFixedDelayedInt(const FixedDelayed<int>& one_delay) {
	//(*package) & (one_delay.delayMS);
	(*package) & (one_delay.reclaimPtrs);

	int list_size = one_delay.history.size();
	(*package) & list_size;

	std::list<FixedDelayed<int>::HistItem>::const_iterator itr = one_delay.history.begin();
	for (; itr != one_delay.history.end(); itr++) {
		FixedDelayed<int>::HistItem one = (*itr);
		int value = one.item;
		int value2 = one.observedTime;

		(*package) & (value);
		(*package) & value2;
	}
}

void PackageUtils::packPoint2D(const Point2D& one_point) {
	(*package) & (one_point);
}

void PackageUtils::packDailyTime(const DailyTime& time)
{
	(*package) & time;
}

void PackageUtils::packDPoint(const DPoint& point)
{
	(*package) & point;
}

void PackageUtils::packDynamicVector(const DynamicVector& vector)
{
	(*package) & vector;
}

void PackageUtils::packBasicData(double value) {
	double buffer = 0;
	if (value != value)
		(*package) & buffer;
	else
		(*package) & value;
}
void PackageUtils::operator<<(double value) {
	packBasicData(value);
}



}
#endif
