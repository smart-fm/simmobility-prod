//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   DeveloperAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * Author: Gishara Premarathne <gishara@smart.mit.edu>
 * 
 * Created on Mar 5, 2014, 6:36 PM
 */
#include <boost/make_shared.hpp>

#include "DeveloperAgent.hpp"
#include "message/MessageBus.hpp"
#include "role/LT_Role.hpp"
#include "database/entity/Developer.hpp"
#include "database/entity/PotentialProject.hpp"
#include "database/entity/UnitType.hpp"
#include "model/DeveloperModel.hpp"
#include "model/lua/LuaProvider.hpp"
#include "message/LT_Message.hpp"
#include "core/DataManager.hpp"
#include "core/AgentsLookup.hpp"
#include "database/dao/BuildingDao.hpp"
#include "database/dao/UnitDao.hpp"
#include "database/dao/ProjectDao.hpp"
#include "database/dao/ParcelDao.hpp"
#include "database/entity/ROILimits.hpp"
#include "behavioral/PredayLT_Logsum.hpp"
#include "util/SharedFunctions.hpp"
#include "util/PrintLog.hpp"
#include <random>

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;
using sim_mob::event::EventArgs;

namespace
{
    const int INTERNAL_MESSAGE_PRIORITY = 5;

    class DEV_InternalMsg : public Message
    {
    public:

    	DEV_InternalMsg(Building &building) :unitId(INVALID_ID), unit(nullptr),building(&building),buildingId(building.getFmBuildingId()), buildingFutureDemolitionDate(std::tm())
    	{
    		priority = INTERNAL_MESSAGE_PRIORITY;
    	}

    	DEV_InternalMsg(Unit &unit) :unitId(unit.getId()), unit(&unit), building(nullptr),buildingId(unit.getBuildingId()), buildingFutureDemolitionDate(std::tm())
    	{
            priority = INTERNAL_MESSAGE_PRIORITY;
        }

    	DEV_InternalMsg(BigSerial unitId):unitId(unitId), unit(nullptr), building(nullptr),buildingId(INVALID_ID), buildingFutureDemolitionDate(std::tm())
    	{

    	}

    	DEV_InternalMsg():unitId(INVALID_ID), unit(nullptr), building(nullptr),buildingId(INVALID_ID), buildingFutureDemolitionDate(std::tm())
    	{

    	}

    	DEV_InternalMsg(BigSerial buildingId,std::tm buildingFutureDemolitionDate) :unitId(INVALID_ID), unit(nullptr),building(nullptr),buildingId(buildingId),buildingFutureDemolitionDate(buildingFutureDemolitionDate)
    	{
    	    priority = INTERNAL_MESSAGE_PRIORITY;
    	}

        virtual ~DEV_InternalMsg(){}
        /**
        * Gets the unit affected by the action.
        * @return
        */
        BigSerial getUnitId()const{
        	return unitId;
        }

        Unit  *getUnit() const{
        	return unit;
        }

        Building  *getBuilding() const{
            return building;
        }

        BigSerial getBuildingId()const{
        	return buildingId;
        }

        std::tm getFutureDemolitionDate() const{
        	return buildingFutureDemolitionDate;
        }

        BigSerial unitId;
        Unit *unit;
        Building *building;
        BigSerial buildingId;
        std::tm buildingFutureDemolitionDate;
    };
}
namespace {


inline void formatDate(int &month, int &year)
{
		month = month - 12;
		year = year + 1;
}

//assign default numeric values to character varying gpr values.
inline float getGpr(const Parcel *parcel)
{
		return atof(parcel->getGpr().c_str());
}

inline BigSerial getBuildingTypeFromUnitType(BigSerial unitTypeId)
{
	if (unitTypeId >= 12 and unitTypeId <= 16) //condo
	{
			return 1;
	}
	else if (unitTypeId >= 7 and unitTypeId <= 11)//Apartment
	{
		return 2;
	}
	else if (unitTypeId >= 17 and unitTypeId <= 21)//Terrace
	{
		return 3;
	}
	else if (unitTypeId >= 22 and unitTypeId <= 26) //Semi Detached
	{
		return 4;
	}
	else if (unitTypeId >= 27 and unitTypeId <= 31) //Detached
	{
		return 5;
	}
	else if (unitTypeId >= 32 and unitTypeId <= 36) //EC
	{
		return 6;
	}

	else return 0;
}

inline void calculateProjectProfit(PotentialProject& project,DeveloperModel* model,int &currTick, int &quarter, HM_Model *housingModel)
{
	std::vector<PotentialUnit>& units = project.getUnits();

	std::vector<PotentialUnit>::iterator unitsItr;

	double totalRevenue = 0;
	double totalConstructionCost = 0;
	double demolitionCostPerUnit = 0;
	double constructionCostPerUnit = 0;
	double totalDemolitionCost = 0;
	int unitAge = 0;
	const BigSerial fmParcelId = project.getParcel()->getId();
	bool isEmptyParcel = model->isEmptyParcel(fmParcelId);
	BigSerial buildingTypeId = 0;

	for (unitsItr = units.begin(); unitsItr != units.end(); unitsItr++) {
		int unitId = (*unitsItr).getUnitTypeId();
		const int lessThan60Apt = 7;
		const int largerThan134Condo = 16;
		if ((unitId >= lessThan60Apt) && (unitId <=largerThan134Condo))
		{
		const ParcelAmenities *amenities = model->getAmenitiesById(fmParcelId);
		//commented the below code in 2012 data as we are now getting logsum per taz id.
		//double logsum = model->getAccessibilityLogsumsByFmParcelId(project.getParcel()->getId())->getAccessibility();
//		double logsum = 0;
//		const LogsumForDevModel *logsumDev = model->getAccessibilityLogsumsByTAZId(project.getParcel()->getTazId());
//		if(logsumDev != nullptr)
//				{
//					logsum = logsumDev->getAccessibility();
//				}

		BigSerial tazId = project.getParcel()->getTazId();
		double logsum = 0;
		const HedonicLogsums *logsumPtr = model->getHedonicLogsumsByTazId(tazId);
		if(logsumPtr != nullptr)
		{
			logsum = logsumPtr->getLogsumWeighted();

		}

		if(model->getScenario().compare("ToaPayohScenario") == 0 && (housingModel->isStudyAreaTaz(tazId)))
		{
			logsum += hedonicLogsumStdDevForTpScenario;
		}

		if(isEmptyParcel)
		{
			unitAge = 0;
		}
		else
		{
			const BuildingAvgAgePerParcel *avgAgePerParcel = model->getBuildingAvgAgeByParcelId(fmParcelId);
			unitAge = avgAgePerParcel->getAge();
		}

		buildingTypeId = getBuildingTypeFromUnitType((*unitsItr).getUnitTypeId());
		std::string quarterStr = boost::lexical_cast<std::string>("2012")+"Q"+boost::lexical_cast<std::string>(quarter);
		const TAOByUnitType* taoByUT = model->getTaoUTByQuarter(quarterStr);

		int taoId = 0;
		if(taoByUT != nullptr)
		{
			taoId = taoByUT->getId();
		}

		//get the historical t values for current quarter - (4,5,6,7) quarters
		const int q4 = 4;
		const int q5 = 5;
		const int q6 = 6;
		const int q7 = 7;

		int taoQ4Id = taoId-q4;
		int taoQ5Id = taoId-q5;
		int taoQ6Id = taoId-q6;
		int taoQ7Id = taoId-q7;

		const TAOByUnitType* taoByUTQ4 = model->getTaoUTById(taoQ4Id);
		const TAOByUnitType* taoByUTQ5 = model->getTaoUTById(taoQ5Id);
		const TAOByUnitType* taoByUTQ6 = model->getTaoUTById(taoQ6Id);
		const TAOByUnitType* taoByUTQ7 = model->getTaoUTById(taoQ7Id);

		double taoValueQ4 = 0;
		double taoValueQ5 = 0;
		double taoValueQ6 = 0;
		double taoValueQ7 = 0;

		if(amenities != nullptr)
		{
			double taoValueQ1 = 0;

			const HedonicCoeffsByUnitType* hedonicCoeffObj = nullptr;
			const LagPrivate_TByUnitType* privateLagTObj = nullptr;
			double HPI = 0;

			switch((*unitsItr).getUnitTypeId())
			{
			case 7:
				taoValueQ1 = taoByUT->getTApartment7();
				taoValueQ4 = taoByUTQ4->getTApartment7();
				taoValueQ5 = taoByUTQ5->getTApartment7();
				taoValueQ6 = taoByUTQ6->getTApartment7();
				taoValueQ7 = taoByUTQ7->getTApartment7();
				break;
			case 8:
				taoValueQ1 = taoByUT->getTApartment8();
				taoValueQ4 = taoByUTQ4->getTApartment8();
				taoValueQ5 = taoByUTQ5->getTApartment8();
				taoValueQ6 = taoByUTQ6->getTApartment8();
				taoValueQ7 = taoByUTQ7->getTApartment8();
				break;
			case 9:
				taoValueQ1 = taoByUT->getTApartment9();
				taoValueQ4 = taoByUTQ4->getTApartment9();
				taoValueQ5 = taoByUTQ5->getTApartment9();
				taoValueQ6 = taoByUTQ6->getTApartment9();
				taoValueQ7 = taoByUTQ7->getTApartment9();
				break;
			case 10:
				taoValueQ1 = taoByUT->getTApartment10();
				taoValueQ4 = taoByUTQ4->getTApartment10();
				taoValueQ5 = taoByUTQ5->getTApartment10();
				taoValueQ6 = taoByUTQ6->getTApartment10();
				taoValueQ7 = taoByUTQ7->getTApartment10();
				break;
			case 11:
				taoValueQ1 = taoByUT->getTApartment11();
				taoValueQ4 = taoByUTQ4->getTApartment11();
				taoValueQ5 = taoByUTQ5->getTApartment11();
				taoValueQ6 = taoByUTQ6->getTApartment11();
				taoValueQ7 = taoByUTQ7->getTApartment11();
				break;
			case 12:
				taoValueQ1 = taoByUT->getTCondo12();
				taoValueQ4 = taoByUTQ4->getTCondo12();
				taoValueQ5 = taoByUTQ5->getTCondo12();
				taoValueQ6 = taoByUTQ6->getTCondo12();
				taoValueQ7 = taoByUTQ7->getTCondo12();
				break;
			case 13:
				taoValueQ1 = taoByUT->getTCondo13();
				taoValueQ1 = taoByUT->getTCondo13();
				taoValueQ4 = taoByUTQ4->getTCondo13();
				taoValueQ5 = taoByUTQ5->getTCondo13();
				taoValueQ6 = taoByUTQ6->getTCondo13();
				taoValueQ7 = taoByUTQ7->getTCondo13();
				break;
			case 14:
				taoValueQ1 = taoByUT->getTCondo14();
				taoValueQ1 = taoByUT->getTCondo14();
				taoValueQ4 = taoByUTQ4->getTCondo14();
				taoValueQ5 = taoByUTQ5->getTCondo14();
				taoValueQ6 = taoByUTQ6->getTCondo14();
				taoValueQ7 = taoByUTQ7->getTCondo14();
				break;
			case 15:
				taoValueQ1 = taoByUT->getTCondo15();
				taoValueQ1 = taoByUT->getTCondo15();
				taoValueQ4 = taoByUTQ4->getTCondo15();
				taoValueQ5 = taoByUTQ5->getTCondo15();
				taoValueQ6 = taoByUTQ6->getTCondo15();
				taoValueQ7 = taoByUTQ7->getTCondo15();
				break;
			case 16:
				taoValueQ1 = taoByUT->getTCondo16();
				taoValueQ4 = taoByUTQ4->getTCondo16();
				taoValueQ5 = taoByUTQ5->getTCondo16();
				taoValueQ6 = taoByUTQ6->getTCondo16();
				taoValueQ7 = taoByUTQ7->getTCondo16();
				break;
			Deafult:
			break;

			}

			hedonicCoeffObj = model->getHedonicCoeffsByUnitTypeId((*unitsItr).getUnitTypeId());
		    privateLagTObj = model->getLagPrivateTByUnitTypeId((*unitsItr).getUnitTypeId());

		    HPI = privateLagTObj->getIntercept() + ( privateLagTObj->getT4() * taoValueQ4) + ( privateLagTObj->getT5() * taoValueQ5) + ( privateLagTObj->getT6() * taoValueQ6) + ( privateLagTObj->getT7() * taoValueQ7)
		    		+ (privateLagTObj->getGdpRate() * taoByUT->getGdpRate()) + (taoByUT->getTreasuryBillYield1Year()- taoByUT->getInflation());

			int ageCapped = 0;
			if(unitAge > 50 )
			{
				ageCapped = 50;
			}
			else
			{
				ageCapped = unitAge;
			}

			double floorArea = (*unitsItr).getFloorArea();
			double revenue = 0;

			int freehold = model->isFreeholdParcel(fmParcelId);

			double distanceToMall = amenities->getDistanceToMall();
			double distanceToPMS30 = amenities->getDistanceToPMS30();
			double distanceToExpress = amenities->getDistanceToExpress();
			double distanceToBus = amenities->getDistanceToBus();
			double distanceToMRT = amenities->getDistanceToMRT();

			if(model->getScenario().compare("ToaPayohScenario") == 0 && (housingModel->isStudyAreaTaz(tazId)))
			{
				distanceToMall = distanceToMall/2.0;
				distanceToPMS30 = distanceToPMS30/2.0;
				distanceToBus = distanceToBus/2.0;
				distanceToMRT = distanceToMRT/2.0;
			}

			double isDistanceToPMS30 = 0;
			double isMRT_200m = 0;
			double isMRT_2_400m = 0;
			double isExpress_200m = 0;
			double isBus_2_400m = 0;
			double isBusGt_400m = 0;

			if(distanceToPMS30 < 1)
			{
				isDistanceToPMS30 = 1;
			}
			if( (distanceToMRT > 0.200) && (distanceToMRT < 0.400))
			{
				isMRT_2_400m = 1;
			}
			if(distanceToExpress < 0.200)
			{
				isExpress_200m = 1;
			}
			if((distanceToBus > 0.200) && (distanceToBus < 0.400))
			{
				isBus_2_400m = 1;
			}
			else if(distanceToBus > 0.400)
			{
				isBusGt_400m = 1;
			}

			if(hedonicCoeffObj!=nullptr)
			{
				revenue  = hedonicCoeffObj->getIntercept() + (hedonicCoeffObj->getLogSqrtArea() * log(floorArea))+ (hedonicCoeffObj->getFreehold() * freehold) +
					(hedonicCoeffObj->getLogsumWeighted() * logsum ) + (hedonicCoeffObj->getPms1km() * isDistanceToPMS30) + (hedonicCoeffObj->getDistanceMallKm() * distanceToMall) +
					(hedonicCoeffObj->getMrt200m()* isMRT_200m) + (hedonicCoeffObj->getMrt2400m() *isMRT_2_400m) + (hedonicCoeffObj->getExpress200m() * isExpress_200m) +
				    (hedonicCoeffObj->getBusGt400m() * isBus_2_400m) + (hedonicCoeffObj->getBusGt400m() * isBusGt_400m) + (hedonicCoeffObj->getAge() * ageCapped) + (hedonicCoeffObj->getAgeSquared() * (ageCapped*ageCapped));
			}

			double revenuePerUnit = exp(revenue+HPI);


			//used in 2008 only: pass futureYear variable as 0 as we are calculating the profit for current year only.
			//double revenuePerUnit = luaModel.calculateUnitRevenue((*unitsItr),*amenities,logsum, quarter,0,taoValue,unitAge);

			if (!isEmptyParcel)
			{
				demolitionCostPerUnit = (*unitsItr).getDemolitionCostPerUnit();
			}
			double demolitionCostPerUnitType = demolitionCostPerUnit * (*unitsItr).getNumUnits();
			totalDemolitionCost = totalDemolitionCost + demolitionCostPerUnitType;
			constructionCostPerUnit = model->getUnitTypeById((*unitsItr).getUnitTypeId())->getConstructionCostPerUnit();
			double profitPerUnit = revenuePerUnit - (constructionCostPerUnit + demolitionCostPerUnit);
			(*unitsItr).setUnitProfit(profitPerUnit);
			(*unitsItr).setDemolitionCostPerUnit(demolitionCostPerUnit);

			double totalRevenuePerUnitType = revenuePerUnit * ((*unitsItr).getNumUnits());
			totalRevenue = totalRevenue + totalRevenuePerUnitType;

			double constructionCostPerUnitType = constructionCostPerUnit * ((*unitsItr).getNumUnits());
			totalConstructionCost = totalConstructionCost + constructionCostPerUnitType;

			#ifdef VERBOSE_DEVELOPER
			PrintOutV("revenue calculated per unit type "<<revenuePerUnit<<"**"<<model->getUnitTypeById((*unitsItr).getUnitTypeId())->getName()<<" with logsum "<<logsum<<"with parcel id"<<project.getParcel()->getId()<<std::endl);
			PrintOutV("construction Cost Per Unit type"<<constructionCostPerUnit<<"**"<<model->getUnitTypeById((*unitsItr).getUnitTypeId())->getName()<<std::endl);
			if (!isEmptyParcel)
			{
				PrintOutV("demolition cost per unit type"<<demolitionCostPerUnit<<std::endl);

			}
			PrintOutV("profit per unit"<<profitPerUnit<<std::endl);
			PrintOutV("num units"<<(*unitsItr).getNumUnits()<<"in unit type id"<<unitsItr->getUnitTypeId()<<std::endl);
			PrintOutV("totalRevenue"<<totalRevenue<<std::endl);
			#endif



			if((currTick+1)%model->getOpSchemaloadingInterval() == 0)
			{
				boost::shared_ptr<PotentialProject> potentialPr = boost::make_shared<PotentialProject>(project);
				model->addPotentialProjects(potentialPr);
			}
		}
	}

	}

	//double demolitionCost = 0;
	double acqusitionCost = 0;
	double landCost = 0;
	if (!isEmptyParcel)
	{
		project.setDemolitionCost(totalDemolitionCost);
		const UnitPriceSum* unitPriceSum = model->getUnitPriceSumByParcelId(fmParcelId);
		if(unitPriceSum != nullptr)
		{
			acqusitionCost = unitPriceSum->getUnitPriceSum() * 1000000; // unit price in the table is in millions
		}
	}
	//else
	//{
		const TazLevelLandPrice* landPrice = model->getTazLevelLandPriceByTazId(project.getParcel()->getTazId());
		if(landPrice != nullptr)
		{
			landCost = project.getParcel()->getLotSize() * getGpr(project.getParcel()) * landPrice->getLandValue();
		}
	//}

	project.setAcquisitionCost(acqusitionCost);
	project.setLandValue(landCost);
	project.setBuildingTypeId(buildingTypeId);

	double totalCost = totalConstructionCost + totalDemolitionCost + acqusitionCost+ landCost;
	double profit = totalRevenue - totalCost;
	project.setProfit(profit);
	project.setConstructionCost(totalConstructionCost);
	double investmentReturnRatio = 0;
	if((totalRevenue>0) && (totalConstructionCost>0))
	{
		investmentReturnRatio = (totalRevenue - totalCost)/ (totalCost);
	}
	project.setInvestmentReturnRatio(investmentReturnRatio);

}
inline void createPotentialUnits(PotentialProject& project,const DeveloperModel* model)
    {

	const int checkUnitTypeStart = 17;
	const int checkUnitTypeEnd = 31;

	std::vector<TemplateUnitType>::const_iterator itr;
	double weightedAverage = 0.0;

	        for (itr = project.templateUnitTypes.begin(); itr != project.templateUnitTypes.end(); itr++)
	        	{
	        	if(itr->getProportion()>0)
	        	{
	        		double propotion = (itr->getProportion()/100.0);
	        		//add the minimum lot size constraint if the unit type is terrace, semi detached or detached
	        		if((itr->getUnitTypeId()>=checkUnitTypeStart) and (itr->getUnitTypeId() <= checkUnitTypeEnd))
	        		{
	        			weightedAverage = weightedAverage + (model->getUnitTypeById(itr->getUnitTypeId())->getTypicalArea()* model->getUnitTypeById(itr->getUnitTypeId())->getMinLosize() *(propotion));
	        		}
	        		else
	        		{
	        			weightedAverage = weightedAverage + (model->getUnitTypeById(itr->getUnitTypeId())->getTypicalArea()*(propotion));
	        		}
	        	}
	        	}

	        int totalUnits = 0;
	        if(weightedAverage>0)
	        {
	        		totalUnits = (getGpr(project.getParcel()) * project.getParcel()->getLotSize())/(weightedAverage);
	        		project.setTotalUnits(totalUnits);
	        }

	        double grossArea = 0;
	        for (itr = project.templateUnitTypes.begin(); itr != project.templateUnitTypes.end(); itr++)
	            {
	        	    double propotion = (itr->getProportion()/100.00);
	        		int numUnitsPerType = totalUnits * propotion;
	        		grossArea = grossArea + numUnitsPerType *  model->getUnitTypeById(itr->getUnitTypeId())->getTypicalArea();
	        		PotentialUnit potentialUnit(itr->getUnitTypeId(),numUnitsPerType,model->getUnitTypeById(itr->getUnitTypeId())->getTypicalArea(),0,0);
	        		project.addUnit(potentialUnit);
	            }

	        project.setGrossArea(grossArea);

    }

    /**
     * Creates and adds units to the given project.
     * @param project to create the units.
     * @param templates available unit type templates.
     */
    inline void addUnitTemplates(PotentialProject& project, const DeveloperModel::TemplateUnitTypeList& unitTemplates)
    {
    	DeveloperModel::TemplateUnitTypeList::const_iterator itr;

        for (itr = unitTemplates.begin(); itr != unitTemplates.end(); itr++)
        {
            if ((*itr)->getTemplateId() == project.getDevTemplate()->getTemplateId())
            {
            	project.addTemplateUnitType(*(*itr));

            }
        }

    }

    /**
     * Create all potential projects.
     * @param parcelsToProcess parcel Ids to process.
     * @param model Developer model.
     * @param outProjects (out parameter) list to receive all projects;
     */
    inline void createPotentialProjects(BigSerial parcelId, DeveloperModel* model, PotentialProject& outProject,int quarter,std::tm &currentDate, int currentTick, HM_Model *housingModel)
    {
        const DeveloperModel::DevelopmentTypeTemplateList& devTemplates = model->getDevelopmentTypeTemplates();
        const DeveloperModel::TemplateUnitTypeList& unitTemplates = model->getTemplateUnitType();
        const int condoType = 2;
        const int apartmentType = 1;
        const int minLotSizeForCondo = 4000;
        /**
         *  Iterates over all development type templates and
         *  get all potential projects which have a density <= GPR.
         */
            const Parcel* parcel = model->getParcelById(parcelId);
            if (parcel)
            {
            	std::vector<PotentialProject> projects;
                DeveloperModel::DevelopmentTypeTemplateList::const_iterator it;

                for (it = devTemplates.begin(); it != devTemplates.end(); it++)
                {
                	//limit the condo to parcels with min lot size of 4000 sqm and apartments to parcels less than 4000 sqm.
                	if( ((*it)->getDevelopmentTypeId() == condoType && parcel->getLotSize() >= minLotSizeForCondo) || ((*it)->getDevelopmentTypeId() == apartmentType && parcel->getLotSize() < minLotSizeForCondo))
                	{
                	if ((*it)->getLandUseTypeId() == parcel->getLandUseTypeId())

                    {
                		PotentialProject project((*it), parcel,parcel->getId(),currentDate);
                		addUnitTemplates(project, unitTemplates);
                		createPotentialUnits(project,model);

                        calculateProjectProfit(project,model,currentTick,quarter,housingModel);

                        int newDevelopment = 0;
                        if(model->isEmptyParcel(parcel->getId()))
                        	{
                        		newDevelopment = 1;
                        	}

                        const ROILimits *roiLimit = model->getROILimitsByDevelopmentTypeId(project.getDevTemplate()->getDevelopmentTypeId());

                        double thresholdInvestmentReturnRatio = 0;
                        if(roiLimit != nullptr)
                        {
                        	thresholdInvestmentReturnRatio = roiLimit->getRoiLimit();
                        }

                        writeROIDataToFile(*parcel,newDevelopment,project.getProfit(),project.getDevTemplate()->getTemplateId(), thresholdInvestmentReturnRatio, project.getInvestmentReturnRatio());

                        if(project.getInvestmentReturnRatio()> thresholdInvestmentReturnRatio)
                        {

                        	if(&project != nullptr)
                        	{
                        		projects.push_back(project);
                        	}
                        }
                    }
                }

                }


                if(projects.size()>0)
                {
                	std::vector<PotentialProject>::iterator projectIt;
                	//calculate the probability of being selected for each project
                	double totalExpRatio = 0.0;
                	for (projectIt = projects.begin(); projectIt != projects.end(); projectIt++)
                	{
                		double expRatio = exp((projectIt)->getInvestmentReturnRatio());
                		(projectIt)->setExpRatio(expRatio);
                		totalExpRatio = totalExpRatio + expRatio;
                	}

                	for (projectIt = projects.begin(); projectIt != projects.end(); projectIt++)
                	{
                		const double probability = (projectIt)->getExpRatio() / (totalExpRatio);
                		(projectIt)->setTempSelectProbability(probability);
                	}

                	//generate a uniformly distributed random number
                	std::random_device rd;
                	std::mt19937 gen(rd());
                	std::uniform_real_distribution<> dis(0.0, 1.0);
                	const double randomNum = dis(gen);
                	double pTemp = 0.0;

                	if(projects.size()>0)
                	{
                		for (projectIt = projects.begin(); projectIt != projects.end(); projectIt++)
                		{
                			if( (pTemp < randomNum) && ( randomNum < ((projectIt)->getTempSelectProbability() + pTemp)))
                			{

                					outProject = (*projectIt);

                			}
                			else
                			{
                				pTemp = pTemp + (projectIt)->getTempSelectProbability();
                			}

                		}

                	}
                }

            }
        }
}


DeveloperAgent::DeveloperAgent(boost::shared_ptr<Parcel> parcel, DeveloperModel* model)
: Agent_LT(ConfigManager::GetInstance().FullConfig().mutexStategy(), (parcel) ? parcel->getId() : INVALID_ID), devModel(model),parcel(parcel),active(false),monthlyUnitCount(0),unitsRemain(true),realEstateAgent(nullptr),postcode(INVALID_ID),housingMarketModel(housingMarketModel),simYear(simYear),currentTick(currentTick),parcelDBStatus(false),hasBTO(false),onGoingProjectOnDay0(false)
{

}

DeveloperAgent::~DeveloperAgent() {
}

void DeveloperAgent::assignParcel(BigSerial parcelId) {
    if (parcelId != INVALID_ID) {
        parcelsToProcess.push_back(parcelId);
    }
}

bool DeveloperAgent::onFrameInit(timeslice now) {

    return true;
}

Entity::UpdateStatus DeveloperAgent::onFrameTick(timeslice now) {

    if (devModel && isActive())
    {
    	currentTick = now.ms();
    	std::tm currentDate = getDateBySimDay(simYear,now.ms());

    	if(!hasBTO)
    	{
    	if(this->parcel->getStatus()== 0)
    	{
    		int quarter = ((currentDate.tm_mon)/4) + 1;
    		PotentialProject project;
    		createPotentialProjects(this->parcel->getId(),devModel,project,quarter,currentDate,currentTick,housingMarketModel);
    		if(project.getUnits().size()>0)
    		{
    			std::vector<PotentialUnit>::iterator unitsItr;
    			std::vector<PotentialUnit>& potentialUnits = project.getUnits();
    			for (unitsItr = potentialUnits.begin(); unitsItr != potentialUnits.end(); ++unitsItr)
    			{
    				std::tm constructionStartDate = getDateBySimDay(simYear,(now.ms()+60));
    				//TODO:: construction start date and launch date is temporary. gishara
    				std::tm launchDate = getDateBySimDay(simYear,(now.ms()+180));
    				boost::shared_ptr <DevelopmentPlan> devPlan(new DevelopmentPlan(this->parcel->getId(),project.getDevTemplate()->getTemplateId(),(*unitsItr).getUnitTypeId(),
    						(*unitsItr).getNumUnits(),currentDate,constructionStartDate,launchDate));
    				devModel->addDevelopmentPlans(devPlan);
    			}


    			BigSerial projectId = devModel->getProjectIdForDeveloperAgent();
    			createUnitsAndBuildings(project,projectId);
    			createProject(project,projectId);
    		}

    	}
    	else
    	{
    		int currTick = this->fmProject->getCurrTick();
    		currTick++;
    		this->fmProject->setCurrTick(currTick);
    		if(onGoingProjectOnDay0)
    		{
    			launchOnGoingUnitsOnDay0();
    		}
    		else
    		{
    			processExistingProjects();
    		}
    	}
    	}
    	else if(hasBTO)
    	{
    		launchBTOUnits(currentDate);
    	}

    }

    //setActive(false);
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void DeveloperAgent::createUnitsAndBuildings(PotentialProject &project,BigSerial projectId)
{
	std::tm currentDate = getDateBySimDay(simYear,currentTick);

	parcel->setStatus(1); //set the status to 1 from 0 to indicate that the parcel is already associated with an ongoing project.
	parcel->setDevelopmentAllowed(3);// 3 = "development not currently allowed because of endogenous constraints"
	//next available date of the parcel for the consideration of a new development is assumed to be one year after.
	std::tm nextAvailableDate = getDateBySimDay((simYear+1),currentTick);
	parcel->setNextAvailableDate(nextAvailableDate);
	parcel->setLastChangedDate(currentDate);
	int newDevelopment = 0;
	if(devModel->isEmptyParcel(parcel->getId()))
	{
		newDevelopment = 1;
	}

	boost::shared_ptr<Parcel> profitableParcel = boost::make_shared<Parcel>(*parcel);
	if(!parcelDBStatus)
	{
		devModel->addProfitableParcels(profitableParcel);
	}
	writeParcelDataToFile(*parcel,newDevelopment,project.getProfit());

	//check whether the parcel is empty; if not send a message to HM model with building id and future demolition date about the units that are going to be demolished.
	if (!(devModel->isEmptyParcel(parcel->getId()))) {
		DeveloperModel::BuildingList buildings = devModel->getBuildings();
		DeveloperModel::BuildingList::iterator itr;

		for (itr = buildings.begin(); itr != buildings.end(); itr++) {
			std::tm futureDemolitionDate = currentDate;
			//set the future demolition date of the building to 3 months ahead.
			int futureDemolitionMonth = futureDemolitionDate.tm_mon + 3;
			int futureDemolitionYear = futureDemolitionDate.tm_year;
			if(futureDemolitionMonth > 11)
			{
				formatDate(futureDemolitionMonth,futureDemolitionYear);
			}
			futureDemolitionDate.tm_mon = futureDemolitionMonth;
			futureDemolitionDate.tm_year = futureDemolitionYear;
			if ((*itr)->getFmParcelId() == parcel->getId()) {
				BigSerial buildingId = (*itr)->getFmBuildingId();
				toBeDemolishedBuildingIds.push_back((*itr)->getFmBuildingId());
				MessageBus::PostMessage(this, LT_STATUS_ID_DEV_BUILDING_DEMOLISHED, MessageBus::MessagePtr(new DEV_InternalMsg(buildingId,futureDemolitionDate)), true);
				//TODO- add demolished building id's to each agent and use it within the agent

			}
		}

	}

	//create a new building
	BigSerial buildingId = devModel->getBuildingIdForDeveloperAgent();
	//building construction start date; assumed to be the first day of the project created.
	//building construction finish date ; assumed to be 6 months after
	std::tm toDate = getDateBySimDay(simYear,(currentTick+180));
	boost::shared_ptr<Building>building(new Building(buildingId,projectId,parcel->getId(),0,0,currentDate,toDate,BUILDING_UNCOMPLETED_WITHOUT_PREREQUISITES,project.getGrosArea(),0,0,0,0,0,0,toDate,0,0,std::string()));
	newBuildings.push_back(building);
	MessageBus::PostMessage(this, LT_DEV_BUILDING_ADDED, MessageBus::MessagePtr(new DEV_InternalMsg(*building.get())), true);

	//create new units and add all the units to the newly created building.
	std::vector<PotentialUnit> units = project.getUnits();
	std::vector<PotentialUnit>::iterator unitsItr;

	for (unitsItr = units.begin(); unitsItr != units.end(); ++unitsItr) {
		for(size_t i=0; i< unitsItr->getNumUnits();i++)
		{
			//TODO: Add the BTO unit sla address to building_match and sla_building. 15 Feb 2017. Chetan/Gishara
			boost::shared_ptr<Unit>unit(new Unit( devModel->getUnitIdForDeveloperAgent(), buildingId, (*unitsItr).getUnitTypeId(), 0, DeveloperAgent::UNIT_PLANNED, (*unitsItr).getFloorArea(), 0, 0,toDate, currentDate,DeveloperAgent::UNIT_NOT_LAUNCHED, DeveloperAgent::UNIT_NOT_READY_FOR_OCCUPANCY, currentDate, 0, currentDate,0));
			newUnits.push_back(unit);
			double profit = (*unitsItr).getUnitProfit();
			double demolitionCost = (*unitsItr).getDemolitionCostPerUnit();
			int quarter = ((currentDate.tm_mon)/3) + 1; //get the current month of the simulation and divide it by 3 to determine the quarter
			writeUnitDataToFile(*unit, profit,parcel->getId(),demolitionCost,quarter);
			MessageBus::PostMessage(this, LT_DEV_UNIT_ADDED, MessageBus::MessagePtr(new DEV_InternalMsg(*unit.get())), true);
		}

	}
	#ifdef VERBOSE_DEVELOPER
	PrintOutV(newUnits.size()<<" number of units generated by developer agent with id "<<this->GetId()<<std::endl);
    #endif
}

void DeveloperAgent::createProject(PotentialProject &project, BigSerial projectId)
{

	std::tm constructionDate = getDateBySimDay(simYear,currentTick);
	std::tm completionDate = getDateBySimDay((simYear+1),currentTick);
	double constructionCost = project.getConstructionCost();
	double demolitionCost = 0; //demolition cost is not calculated by the model yet.
	double totalCost = constructionCost + demolitionCost;
	double fmLotSize = parcel->getLotSize();
	double grossArea = project.getGrosArea();
	std::string grossRatio = boost::lexical_cast<std::string>(grossArea / fmLotSize);
	std::string projectStatus = "Active";
	boost::shared_ptr<Project>fmProject(new Project(projectId,parcel->getId(),INVALID_ID,project.getDevTemplate()->getTemplateId(),EMPTY_STR,constructionDate,completionDate,constructionCost,demolitionCost,totalCost,fmLotSize,grossRatio,grossArea,0,constructionDate,projectStatus));
	writeProjectDataToFile(fmProject);
	this->fmProject = fmProject;
	MessageBus::PostMessage(this, LT_DEV_PROJECT_ADDED, MessageBus::MessagePtr(new DEV_InternalMsg()), true);

}

void DeveloperAgent::processExistingProjects()
{
	int projectDuration = this->fmProject->getCurrTick();
	std::vector<boost::shared_ptr<Building> >::iterator buildingsItr;
	std::vector<boost::shared_ptr<Unit> >::iterator unitsItr;
	const int secondMonth = 59;
	const int fourthMonth = 119;
	const int sixthMonth = 179;
	switch(projectDuration)
	{
	case (secondMonth):
	{
		for(buildingsItr = this->newBuildings.begin(); buildingsItr != this->newBuildings.end(); buildingsItr++)
		{
			(*buildingsItr)->setBuildingStatus(BUILDING_UNCOMPLETED_WITH_PREREQUISITES);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_BUILDING_UNCOMPLETED_WITH_PREREQUISITES,MessageBus::MessagePtr(new DEV_InternalMsg((*buildingsItr)->getFmBuildingId(),std::tm())), true);
		}

		for(unitsItr = this->newUnits.begin(); unitsItr != this->newUnits.end(); unitsItr++)
		{
			(*unitsItr)->setConstructionStatus(UNIT_UNDER_CONSTRUCTION);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_UNIT_UNDER_CONSTRUCTION,MessageBus::MessagePtr(new DEV_InternalMsg((*unitsItr)->getId())), true);
		}
		break;
	}
	case (fourthMonth):
	{
		for(buildingsItr = this->newBuildings.begin(); buildingsItr != this->newBuildings.end(); buildingsItr++)
		{
			(*buildingsItr)->setBuildingStatus(BUILDING_NOT_LAUNCHED);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_BUILDING_NOT_LAUNCHED,MessageBus::MessagePtr(new DEV_InternalMsg((*buildingsItr)->getFmBuildingId(),std::tm())), true);
		}

		for(unitsItr = this->newUnits.begin(); unitsItr != this->newUnits.end(); unitsItr++)
		{
			(*unitsItr)->setConstructionStatus(UNIT_CONSTRUCTION_COMPLETED);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_UNIT_CONSTRUCTION_COMPLETED,MessageBus::MessagePtr(new DEV_InternalMsg((*unitsItr)->getId())), true);
		}
		break;
	}
	case(sixthMonth):
	{
		for(buildingsItr = this->newBuildings.begin(); buildingsItr != this->newBuildings.end(); buildingsItr++)
		{
			//building launched message will be sent when at least a single unit of the building is launched for sale.
			(*buildingsItr)->setBuildingStatus(BUILDING_LAUNCHED_BUT_UNSOLD);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_BUILDING_LAUNCHED_BUT_UNSOLD,MessageBus::MessagePtr(new DEV_InternalMsg((*buildingsItr)->getFmBuildingId(),std::tm())), true);
		}
			if(unitsRemain)
			{
				std::vector<boost::shared_ptr<Unit> >::iterator first;
				std::vector<boost::shared_ptr<Unit> >::iterator last;
				setUnitsForHM(first,last);
				std::vector<boost::shared_ptr<Unit> > unitsToSale(first,last);

				#ifdef VERBOSE_DEVELOPER
				PrintOutV(unitsToSale.size()<<" number of units launched for selling by developer agent "<<this->GetId()<<"on day "<<currentTick<<std::endl);
				#endif

				//comment this to disable private unit launches by developer model, when running BTO.
				for(unitsItr = unitsToSale.begin(); unitsItr != unitsToSale.end(); unitsItr++)
				{
					(*unitsItr)->setSaleStatus(UNIT_LAUNCHED_BUT_UNSOLD);
					(*unitsItr)->setOccupancyStatus(UNIT_READY_FOR_OCCUPANCY_AND_VACANT);
					MessageBus::PostMessage(this, LT_STATUS_ID_DEV_NEW_UNIT_LAUNCHED_BUT_UNSOLD,MessageBus::MessagePtr(new DEV_InternalMsg((*unitsItr)->getId())), true);
					MessageBus::PostMessage(this, LT_STATUS_ID_DEV_UNIT_READY_FOR_OCCUPANCY_AND_VACANT,MessageBus::MessagePtr(new DEV_InternalMsg((*unitsItr)->getId())), true);
				}

			}
		break;
	}

	}

}

void DeveloperAgent::setUnitsForHM(std::vector<boost::shared_ptr<Unit> >::iterator &first,std::vector<boost::shared_ptr<Unit> >::iterator &last)
{
	const int totalUnits = newUnits.size();
	const double monthlyUnitsFraction = 0.2;
	int monthlyUnits = totalUnits * monthlyUnitsFraction;

	if(monthlyUnits <=0)
	{
		monthlyUnits = totalUnits;
	}
	first = newUnits.begin()+ monthlyUnitCount;
	if(monthlyUnitCount < totalUnits)
	{
		monthlyUnitCount = monthlyUnitCount + monthlyUnits;
	}

	last = newUnits.begin()+ monthlyUnitCount;
	if(monthlyUnitCount >= totalUnits)
	{
		setUnitsRemain(false);
	}
}

void DeveloperAgent::setUnitsRemain (bool unitRemain)
{
	unitsRemain = unitRemain;
}

void DeveloperAgent::launchBTOUnits(std::tm currentDate)
{
	std::vector<BigSerial>  btoUnits = devModel->getBTOUnits(currentDate);
	if(btoUnits.size()>0)
	{
		MessageBus::PostMessage(realEstateAgent, LT_DEV_BTO_UNIT_ADDED, MessageBus::MessagePtr(new HM_ActionMessage((btoUnits))), true);
	}
}

bool DeveloperAgent::isHasBto() const
{
		return hasBTO;
}

void DeveloperAgent::setHasBto(bool hasBto)
{
		hasBTO = hasBto;
}

void DeveloperAgent::onFrameOutput(timeslice now) {
}

void DeveloperAgent::onEvent(EventId eventId, Context ctxId, EventPublisher*, const EventArgs& args) {

	processEvent(eventId, ctxId, args);
}

void DeveloperAgent::processEvent(EventId eventId, Context ctxId, const EventArgs& args)
{
	switch (eventId) {
	        case LTEID_EXT_ZONING_RULE_CHANGE:
	        {
	        	devModel->reLoadZonesOnRuleChangeEvent();
	        	devModel->processParcels();
	            break;
	        }

	        default:break;
	    };
}

void DeveloperAgent::onWorkerEnter() {

	MessageBus::SubscribeEvent(LTEID_EXT_ZONING_RULE_CHANGE, this, this);
}

void DeveloperAgent::onWorkerExit() {

	MessageBus::UnSubscribeEvent(LTEID_EXT_ZONING_RULE_CHANGE, this, this);
}

void DeveloperAgent::HandleMessage(Message::MessageType type, const Message& message) {

	switch (type) {

		        case LT_DEV_UNIT_ADDED:
		        {
		            const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		            boost::shared_ptr<Unit> newUnit = boost::make_shared<Unit>(*(devArgs.getUnit()));
		            devModel->addNewUnits(newUnit);
		            MessageBus::PostMessage(realEstateAgent, LTEID_HM_UNIT_ADDED, MessageBus::MessagePtr(new HM_ActionMessage((*devArgs.getUnit()))), true);
		            break;
		        }
		        case LT_DEV_PROJECT_ADDED:
		       	{
		       	    devModel->addNewProjects(fmProject);
		       	    break;
		       	}
		        case LT_STATUS_ID_DEV_UNIT_UNDER_CONSTRUCTION:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PostMessage(realEstateAgent, LT_STATUS_ID_HM_UNIT_UNDER_CONSTRUCTION, MessageBus::MessagePtr(new HM_ActionMessage((devArgs.getUnitId()))), true);
		        	break;
		        }
		        case LT_STATUS_ID_DEV_UNIT_CONSTRUCTION_COMPLETED:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PostMessage(realEstateAgent, LT_STATUS_ID_HM_UNIT_CONSTRUCTION_COMPLETED, MessageBus::MessagePtr(new HM_ActionMessage((devArgs.getUnitId()))), true);
		        	break;
		        }
		        case LT_STATUS_ID_DEV_NEW_UNIT_LAUNCHED_BUT_UNSOLD:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PostMessage(realEstateAgent, LT_STATUS_ID_HM_NEW_UNIT_LAUNCHED_BUT_UNSOLD, MessageBus::MessagePtr(new HM_ActionMessage((devArgs.getUnitId()))), true);
		        	break;
		        }
		        case LT_STATUS_ID_DEV_ONGOING_UNIT_LAUNCHED_BUT_UNSOLD:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PostMessage(realEstateAgent, LT_STATUS_ID_HM_ONGOING_UNIT_LAUNCHED_BUT_UNSOLD, MessageBus::MessagePtr(new HM_ActionMessage((devArgs.getUnitId()))), true);
		        	break;
		        }
		        case LT_STATUS_ID_DEV_UNIT_READY_FOR_OCCUPANCY_AND_VACANT:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PostMessage(realEstateAgent, LT_STATUS_ID_HM_UNIT_READY_FOR_OCCUPANCY_AND_VACANT, MessageBus::MessagePtr(new HM_ActionMessage((devArgs.getUnitId()))), true);
		        	break;
		        }
		        case LT_DEV_BUILDING_ADDED:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	boost::shared_ptr<Building> newBuilding = boost::make_shared<Building>(*(devArgs.getBuilding()));
		        	devModel->addNewBuildings(newBuilding);
		        	MessageBus::PostMessage(realEstateAgent, LTEID_HM_BUILDING_ADDED, MessageBus::MessagePtr(new HM_ActionMessage((*devArgs.getBuilding()))), true);
		        	break;
		        }
		        case LT_STATUS_ID_DEV_BUILDING_DEMOLISHED:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PostMessage(realEstateAgent, LT_STATUS_ID_HM_BUILDING_DEMOLISHED, MessageBus::MessagePtr(new HM_ActionMessage((devArgs.getBuildingId()),(devArgs.getFutureDemolitionDate()))), true);
		        	break;
		        }
		        case LT_STATUS_ID_DEV_BUILDING_UNCOMPLETED_WITH_PREREQUISITES:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PostMessage(realEstateAgent, LT_STATUS_ID_HM_BUILDING_UNCOMPLETED_WITH_PREREQUISITES, MessageBus::MessagePtr(new HM_ActionMessage((devArgs.getBuildingId()),(std::tm()))), true);
		        	break;
		        }
		        case LT_STATUS_ID_DEV_BUILDING_NOT_LAUNCHED:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PostMessage(realEstateAgent, LT_STATUS_ID_HM_BUILDING_NOT_LAUNCHED, MessageBus::MessagePtr(new HM_ActionMessage((devArgs.getBuildingId()),(std::tm()))), true);
		        	break;
		        }
		        case LT_STATUS_ID_DEV_BUILDING_LAUNCHED_BUT_UNSOLD:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PostMessage(realEstateAgent, LT_STATUS_ID_HM_BUILDING_LAUNCHED_BUT_UNSOLD, MessageBus::MessagePtr(new HM_ActionMessage((devArgs.getBuildingId()),(std::tm()))), true);
		        	break;
		        }
		        default:break;
		    };
}

void DeveloperAgent::setRealEstateAgent(RealEstateAgent* realEstAgent)
{
	this->realEstateAgent = realEstAgent;
}

void DeveloperAgent::setPostcode(int postCode)
{
	this->postcode = postCode;
}

void DeveloperAgent::setHousingMarketModel(HM_Model *housingModel)
{

	this->housingMarketModel = housingModel;
}

void DeveloperAgent::setSimYear(int simulationYear)
{
	this->simYear = simulationYear;
}

void DeveloperAgent::setProject(boost::shared_ptr<Project> project)
{
	this->fmProject = project;
}

boost::shared_ptr<Parcel> DeveloperAgent::getParcel()
{
	return this->parcel;
}

void DeveloperAgent::setParcelDBStatus(bool status)
{
	this->parcelDBStatus = status;
}

bool DeveloperAgent::getParcelDBStatus()
{
	return this->parcelDBStatus;
}

void DeveloperAgent::setNewBuildings(std::vector<boost::shared_ptr<Building> > buildings)
{
	this->newBuildings = buildings;
}

void DeveloperAgent::setNewUnits(std::vector<boost::shared_ptr<Unit> > units)
{
	this->newUnits = units;
}

bool  DeveloperAgent::isIsDay0Project() const
{
		return onGoingProjectOnDay0;
}

void  DeveloperAgent::setIsDay0Project(bool isDay0Project)
{
		this->onGoingProjectOnDay0 = isDay0Project;
}

void DeveloperAgent::launchOnGoingUnitsOnDay0()
{
	std::tm currentDate = getDateBySimDay(simYear,currentTick);
	boost::gregorian::date currentDateGreg = boost::gregorian::date_from_tm(currentDate);
	for(boost::shared_ptr<Unit> unit:this->newUnits)
	{
		boost::gregorian::date saleFromDate = boost::gregorian::date_from_tm(unit->getSaleFromDate());
		if(currentDateGreg == saleFromDate)
		{
			MessageBus::PostMessage(realEstateAgent, LTEID_HM_UNIT_ADDED, MessageBus::MessagePtr(new HM_ActionMessage(*unit.get())), true);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_ONGOING_UNIT_LAUNCHED_BUT_UNSOLD,MessageBus::MessagePtr(new DEV_InternalMsg(unit->getId())), true);
		}
	}
}
