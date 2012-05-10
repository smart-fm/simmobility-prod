#include "GenConfig.h"

#include "PackageUtils.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "util/GeomHelpers.hpp"
#include "partitions/PartitionManager.hpp"
#include "util/DailyTime.hpp"
#include "util/DynamicVector.hpp"
#include "geospatial/Point2D.hpp"

using namespace sim_mob;

std::string sim_mob::PackageUtils::getPackageData() {
	return buffer.str().data();
}

sim_mob::PackageUtils::PackageUtils()
{
	package = new boost::archive::text_oarchive(buffer);
}

sim_mob::PackageUtils::~PackageUtils()
{
	buffer.clear();
	safe_delete_item(package);
	//if (package) {
		//delete package;
		//package = nullptr;
	//}
}

void sim_mob::PackageUtils::packFixedDelayedDPoint(const FixedDelayed<DPoint*>& one_delay) {
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

void sim_mob::PackageUtils::packFixedDelayedDouble(const FixedDelayed<double>& one_delay) {
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

void sim_mob::PackageUtils::packFixedDelayedInt(const FixedDelayed<int>& one_delay) {
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

void sim_mob::PackageUtils::packPoint2D(const Point2D& one_point) {
	(*package) & (one_point);
}

void sim_mob::PackageUtils::packDailyTime(const DailyTime& time)
{
	(*package) & time;
}

void sim_mob::PackageUtils::packDPoint(const DPoint& point)
{
	(*package) & point;
}

void sim_mob::PackageUtils::packDynamicVector(const DynamicVector& vector)
{
	(*package) & vector;
}

void sim_mob::PackageUtils::packBasicData(double value) {
	double buffer = 0;
	if (value != value) {
		(*package) & buffer;
	} else {
		(*package) & value;
	}
}
void sim_mob::PackageUtils::operator<<(double value) {
	packBasicData(value);
}


#endif
