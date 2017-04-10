//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * Alternative.cpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/Alternative.hpp>

namespace sim_mob
{
	namespace long_term
	{
		Alternative::Alternative( BigSerial id, BigSerial planAreaId, std::string planAreaName, BigSerial dwellingTypeId, std::string dwellingTypeName,
								  double avgHouseholdSize, double avgHouseholdIncome, int unitTypeCounter, int populationByUnitType, double medianHedonicPrice,
								  double sumFloorArea, BigSerial mapId)
								 :id(id), planAreaId(planAreaId), planAreaName(planAreaName), dwellingTypeId(dwellingTypeId), dwellingTypeName(dwellingTypeName),
								  avgHouseholdSize(avgHouseholdSize), avgHouseholdIncome(avgHouseholdIncome), unitTypeCounter(unitTypeCounter),
								  populationByUnitType(populationByUnitType), medianHedonicPrice(medianHedonicPrice), sumFloorArea(sumFloorArea), mapId(mapId){}

		Alternative::~Alternative() {}

		Alternative::Alternative(const Alternative& source)
		{
			this->id = source.id;
			this->planAreaId = source.planAreaId;
			this->planAreaName = source.planAreaName;
			this->dwellingTypeId = source.dwellingTypeId;
			this->dwellingTypeName = source.dwellingTypeName;
			this->avgHouseholdSize = source.avgHouseholdSize;
			this->avgHouseholdIncome = source.avgHouseholdIncome;
			this->unitTypeCounter = source.unitTypeCounter;
			this->populationByUnitType = source.populationByUnitType;
			this->medianHedonicPrice = source.medianHedonicPrice;
			this->sumFloorArea = source.sumFloorArea;
			this->mapId = source.mapId;
		}

		Alternative& Alternative::operator=(const Alternative& source)
		{
			this->id = source.id;
			this->planAreaId = source.planAreaId;
			this->planAreaName = source.planAreaName;
			this->dwellingTypeId = source.dwellingTypeId;
			this->dwellingTypeName = source.dwellingTypeName;
			this->avgHouseholdSize = source.avgHouseholdSize;
			this->avgHouseholdIncome = source.avgHouseholdIncome;
			this->unitTypeCounter = source.unitTypeCounter;
			this->populationByUnitType = source.populationByUnitType;
			this->medianHedonicPrice = source.medianHedonicPrice;
			this->sumFloorArea = source.sumFloorArea;

			return *this;
		}


		BigSerial Alternative::getMapId() const
		{
			return mapId;
		}

		BigSerial Alternative::getId() const
		{
			return id;
		}

		BigSerial Alternative::getPlanAreaId() const
		{
			return planAreaId;
		}

		std::string Alternative::getPlanAreaName() const
		{
			return planAreaName;
		}

		BigSerial Alternative::getDwellingTypeId() const
		{
			return dwellingTypeId;
		}

		std::string Alternative::getDwellingTypeName() const
		{
			return dwellingTypeName;
		}

		double Alternative::getAvgHouseholdSize()const
		{
			return avgHouseholdSize;
		}

		double Alternative::getAvgHouseholdIncome()const
		{
			return avgHouseholdIncome;
		}

		int Alternative::getUnitTypeCounter()const
		{
			return unitTypeCounter;
		}

		int Alternative::getPopulationByUnitType()const
		{
			return populationByUnitType;
		}

		int Alternative::getMedianHedonicPrice() const
		{
			return medianHedonicPrice;
		}

		void Alternative::setAvgHouseholdSize( double value )
		{
				avgHouseholdSize = value;
		}

		void Alternative::setAvgHouseholdIncome( double value )
		{
			avgHouseholdIncome = value;
		}

		void Alternative::setUnitTypeCounter( int value )
		{
			unitTypeCounter = value;
		}

		void Alternative::setPopulationByUnitType( int value )
		{
			populationByUnitType = value;
		}

		void Alternative::setSumFloorArea( double value)
		{
			sumFloorArea = value;
		}

		double Alternative::getSumFloorArea()
		{
			return sumFloorArea;
		}

		void Alternative::setMedianHedonicPrice(double value)
		{
			medianHedonicPrice = value;
		}

		std::ostream& operator<<(std::ostream& strm, const Alternative& data)
		{
			return strm << "{"
						<< "\"id\":\"" << data.id << "\","
						<< "\"planAreaId\":\"" << data.planAreaId << "\","
						<< "\"planAreaName\":\"" << data.planAreaName << "\","
						<< "\"dwellingTypeid\":\"" << data.dwellingTypeId << "\","
						<< "\"dwellingTypeName\":\"" << data.dwellingTypeName << "\","
						<< "\" avgHouseholdSize \":\"" << data.avgHouseholdSize << "\","
						<< "\"avgHouseholdIncome \":\"" << data.avgHouseholdIncome << "\","
						<< "\"unitTypeCounter \":\"" << data.unitTypeCounter << "\","
						<< "\"populationByUnitType \":\"" << data.populationByUnitType << "\""
						<< "\"medianHedonicPrice \":\"" << data.medianHedonicPrice<< "\""
						<< "\"mapId \":\"" << data.mapId<< "\""
						<< "}";
		}
	}
} /* namespace sim_mob */
