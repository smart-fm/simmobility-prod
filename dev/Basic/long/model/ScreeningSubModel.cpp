//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ScreeningSubModel.cpp
 *
 *  Created on: 19 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <model/ScreeningSubModel.hpp>
#include <util/TimeCheck.hpp>

namespace sim_mob
{
	namespace long_term
	{

		ScreeningSubModel::ScreeningSubModel()
		{
			ln_popdwl		=  0.7190261605;	//1 logarithm of population by housing type in the zone 	persons
			den_respop_ha	=  0.2926656795;	//2 population density	persons per hectare (x10^-2)
			f_loc_com		= -0.2948563454;	//3 zonal average fraction of commercial land within a 500-meter buffer area from a residential postcode (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
			f_loc_res		=  0.6130340080;	//4 zonal average fraction of residential land within a 500-meter buffer area from a residential postcode  (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
			f_loc_open		= 2.2061274249;	//5 zonal average fraction of open space within a 500-meter buffer area from a residential postcode (weighted by residential unit within the buffer)	percentage point (x10^-1)
			odi10_loc		= -0.5323132576;	//6 zonal average local land use mix (opportunity diversity) index: 1-(|lu1/t-1/9|+|lu2/t-1/9|+|lu3/t-1/9|+|lu4/t-1/9|+|lu5/t-1/9|+|lu6/t-1/9|+|lu7/t-1/9|+|lu8/t-1/9|+|lu9/t-1/9|)/(16/9)	(x10)
			dis2mrt			= -0.1252826377;	//7 zonal average distance to the nearest MRT station	in kilometer
			dis2exp			= -0.0245811256;	//8 zonal average distance to the nearest express way	in kilometer
			accmanufact_jobs= -4.1227190422;	// manufacturing jobs
			accoffice_jobs	= -0.1068747459;	// office jobs
			pt_tt			= -0.7902179878;	// public transit total time
			pt_cost			= -1.2372340031;	// public transit total cost
			f_age4_n4		= 0.0000032028;	//10 zonal fraction of population younger than 4 years old x dummy if presence of kids younger than 4 years old in the household (=1, yes; =0, no)	percentage point (x10^-1)
			f_age19_n19		= 0.0000015216;	//11 zonal fraction of population between 5 and 19 years old x dummy if presence of children in the household  (=1, yes; =0, no)	percentage point (x10^-1)
			f_age65_n65		= 0.0000323330;	//12 zonal fraction of population older than 65 years old x dummy if presence of seniors in the household  (=1, yes; =0, no)	percentage point (x10^-1)
			f_chn_nchn		= 0.0000003178;	//13 zonal fraction of Chinese population x  dummy if household is Chinese (=1, yes; =0, no)	percentage point (x10^-1)
			f_mal_nmal		= 0.0000205347;	//14 zonal fraction of Malay population x  dummy if household is Malay (=1, yes; =0, no)	percentage point (x10^-1)
			f_indian_nind	= 0.0000238031;	//15 zonal fraction of Indian population x  dummy if household is Indian (=1, yes; =0, no)	percentage point (x10^-1)
			hhsize_diff		= -0.4025805306;	//16 absolute difference between zonal average household size by housing type and household size	persons
			log_hhinc_diff	= -0.4922419899;	//17 absolute difference between logarithm of the zonal median household montly income by housing type and logarithm of the household income	SGD
			log_price05tt_med= -0.2332194751;	//18 logarithm of the zonal median housing price by housing type	in (2005) SGD
			DWL600			= 0.8203584074;	//19 = 1, if household size is 1, living in private condo/apartment
			DWL700			= 1.6281946586;	//20 = 1, if household size is 1, living in landed property
			DWL800			= 0.9276536990; 	//21 = 1, if household size is 1, living in other types of housing units
			DWL400_500		= 0.1517336429;

			model 			= nullptr;
			headOfHousehold = nullptr;
		}

		ScreeningSubModel::~ScreeningSubModel(){}

		BigSerial ScreeningSubModel::ComputeWorkPlanningArea(PlanningArea *planningAreaWork)
		{
				Job  *headOfHhJob = model->getJobById( (headOfHousehold)->getJobId());
				Establishment *headOfHhEstablishment = model->getEstablishmentById(headOfHhJob->getEstablishmentId());
				Postcode *slaAddressWork = model->getPostcodeById( headOfHhEstablishment->getSlaAddressId());

				int tazIdWork = slaAddressWork->getTazId();
				Taz *tazWork  = model->getTazById(tazIdWork);
				int mtzIdWork = model->getMtzIdByTazId(tazIdWork);
				Mtz *mtzWork  = model->getMtzById(mtzIdWork);

				PlanningSubzone *planningSubzoneWork = nullptr;

				if(mtzWork)
					planningSubzoneWork = model->getPlanningSubzoneById( mtzWork->getPlanningSubzoneId() );

				if(planningSubzoneWork)
					planningAreaWork = (model->getPlanningAreaById(planningSubzoneWork->getPlanningAreaId()));

				if(!planningAreaWork)
				{
					AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "Planning Area null for Taz id  %1%.") %  tazIdWork).str());
					return 0;
				}

				return planningSubzoneWork->getPlanningAreaId();

		}

		BigSerial ScreeningSubModel::ComputeHomePlanningArea(PlanningArea* planningArea, Household *household)
		{
			PlanningSubzone *planningSubzone = nullptr;

			int tazId = model->getUnitTazId( household->getUnitId() );
			Taz *taz  = model->getTazById(tazId);
			int mtzId = model->getMtzIdByTazId(tazId);
			Mtz *mtz  = model->getMtzById(mtzId);

			if(mtz)
				planningSubzone = model->getPlanningSubzoneById( mtz->getPlanningSubzoneId() );

			if(planningSubzone)
				planningArea = model->getPlanningAreaById(planningSubzone->getPlanningAreaId());

			if(!planningArea)
			{
				AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "Planning Area null for Taz id  %1%.") %  tazId).str());
				return 0;
			}

			return planningSubzone->getPlanningAreaId();
		}

		void ScreeningSubModel::ComputeHeadOfHousehold(Household* household)
		{
			std::vector<BigSerial> individuals = household->getIndividuals();
			for(int n = 0; n < individuals.size(); n++)
			{
				Individual *tempIndividual = model->getIndividualById(individuals[n]);

				if( tempIndividual->getHouseholdHead() == true )
				{
					headOfHousehold = tempIndividual;
					break;
				}
			}
		}

		int ScreeningSubModel::GetDwellingType(int unitType)
		{
			int dwellingType = 0;

			if( unitType < 3 )
				dwellingType = 100;
			else
			if( unitType == 3 )
				dwellingType = 300;
			else
			if( unitType == 4 )
				dwellingType = 400;
			else
			if( unitType == 5 || unitType == 6 )
				dwellingType = 500;
			else
			if(( unitType >=7 && unitType <=16 ) || ( unitType >= 32 && unitType <= 36 ) )
				dwellingType = 600;
			else
			if( unitType >= 17 && unitType <= 31 )
				dwellingType = 700;
			else
				dwellingType = 800;

			return dwellingType;
		}

		void ScreeningSubModel::getScreeningProbabilities( int hhId, std::vector<double> &probabilities, HM_Model *modelArg, int day )
		{
			model = modelArg;
			Household* household = model->getHouseholdById(hhId);
			const Unit* unit = model->getUnitById( household->getUnitId() );

			ComputeHeadOfHousehold(household);

			PlanningArea *planningAreaWork = nullptr;
			BigSerial id1 = ComputeWorkPlanningArea(planningAreaWork );

			planningAreaWork = model->getPlanningAreaById( id1 );

			PlanningArea* planningArea = nullptr;
			BigSerial id2 = ComputeHomePlanningArea(planningArea, household);

			planningArea = model->getPlanningAreaById(id2);

			if(!planningArea || !planningAreaWork)
				return;

			std::vector<PopulationPerPlanningArea*> populationPerPlanningArea = model->getPopulationByPlanningAreaId(planningArea->getId());

			double populationTotal 	 = 0;

			double populationChinese = 0;
			double populationMalay	 = 0;
			double populationIndian	 = 0;
			double populationOther	 = 0;

			bool  bHouseholdEthnicityChinese = false;
			bool  bHouseholdEthnicityMalay	 = false;
			bool  bHouseholdEthnicityIndian	 = false;

			double populationYoungerThan4 	= 0;
			double population5To19 			= 0;
			double populationGreaterThan65 	= 0;

			double bHouseholdMemberYoungerThan4  = false;
			double bHouseholdMember5To19		 = false;
			double bHouseholdMemberGreaterThan65 = false;

			double 	avgHouseholdSize = 0;
			double 	avgHouseholdIncome = 0;
			int	   	unitTypeCounter = 0;
			int 	populationByunitType = 0;
			int		sumFloorArea = 0;

			if( household->getEthnicityId() == 1 )
				bHouseholdEthnicityChinese = true;

			if( household->getEthnicityId() == 2 )
				bHouseholdEthnicityMalay = true;

			if( household->getEthnicityId() == 3 )
				bHouseholdEthnicityIndian = true;

			std::vector<BigSerial> individualIds = household->getIndividuals();

			for( int n = 0; n < individualIds.size(); n++ )
			{
				Individual* thisMember = model->getIndividualById(individualIds[n]);

				if( thisMember->getAgeCategoryId()  == 0 )
					bHouseholdMemberYoungerThan4 = true;

				if( thisMember->getAgeCategoryId() > 0 && thisMember->getAgeCategoryId() < 4 )
					bHouseholdMember5To19 = true;

				if( thisMember->getAgeCategoryId() > 12 )
					bHouseholdMemberGreaterThan65 = true;
			}

			for(int n = 0; n < populationPerPlanningArea.size(); n++)
			{
				populationTotal += populationPerPlanningArea[n]->getPopulation();

				if(populationPerPlanningArea[n]->getEthnicityId() == 1 )
					populationChinese = populationChinese + populationPerPlanningArea[n]->getPopulation();

				if(populationPerPlanningArea[n]->getEthnicityId() == 2 )
					populationMalay = populationMalay + populationPerPlanningArea[n]->getPopulation();

				if(populationPerPlanningArea[n]->getEthnicityId() == 3 )
					populationIndian = populationIndian + populationPerPlanningArea[n]->getPopulation();

				if(populationPerPlanningArea[n]->getEthnicityId() == 4 )
					populationOther = populationOther + populationPerPlanningArea[n]->getPopulation();

				if(populationPerPlanningArea[n]->getAgeCategoryId() == 0 )
					 populationYoungerThan4 = populationYoungerThan4 + populationPerPlanningArea[n]->getPopulation();

				if(populationPerPlanningArea[n]->getAgeCategoryId() > 0 && populationPerPlanningArea[n]->getAgeCategoryId()< 4 )
					 population5To19 = population5To19 + populationPerPlanningArea[n]->getPopulation();

				if(populationPerPlanningArea[n]->getAgeCategoryId() > 12 )
					 populationGreaterThan65 = populationGreaterThan65 + populationPerPlanningArea[n]->getPopulation();
			}

			/*
			std::vector<PlanningSubzone*>  planningSubzones = model->getPlanningSubZoneByPlanningAreaId(planningArea->getId());
			std::vector<Mtz*> mtzs = model->getMtzBySubzoneVec(planningSubzones);
			std::vector<BigSerial> planningAreaTazs = model->getTazByMtzVec(mtzs);

			double planningArea_size = 0;
			for( int n = 0; n < planningAreaTazs.size();n++)
			{
				Taz *thisTaz = model->getTazById(planningAreaTazs[n]);

				planningArea_size += thisTaz->getArea();
			}

			//convert sqm into hectares
			planningArea_size = planningArea_size / 10000.0;
			*/

		 	double probabilitySum = 0;


			for( int n = 0; n < model->getAlternatives().size(); n++ )
			{
			 	if( model->getAlternatives()[n]->getUnitTypeCounter() == 0 )
			 	{
					for(int m = 0; m < populationPerPlanningArea.size(); m++)
					{
						int unitType = populationPerPlanningArea[m]->getUnitType();

						int dwellingType = GetDwellingType(unitType);

						if( dwellingType == model->getAlternatives()[n]->getDwellingTypeId() )
						{
							avgHouseholdSize += populationPerPlanningArea[m]->getAvgHhSize();
							avgHouseholdIncome += populationPerPlanningArea[m]->getAvgIncome();
							unitTypeCounter++;
							populationByunitType += populationPerPlanningArea[m]->getPopulation();
							sumFloorArea += populationPerPlanningArea[m]->getFloorArea();
						}
					}

			 		model->getAlternatives()[n]->setAvgHouseholdSize(avgHouseholdSize );
					model->getAlternatives()[n]->setAvgHouseholdIncome(avgHouseholdIncome );
					model->getAlternatives()[n]->setUnitTypeCounter(unitTypeCounter );
					model->getAlternatives()[n]->setPopulationByUnitType(populationByunitType );
					model->getAlternatives()[n]->setSumFloorArea(sumFloorArea);
			 	}
			 	else
			 	{
			 		avgHouseholdSize  = model->getAlternatives()[n]->getAvgHouseholdSize();
			 		avgHouseholdIncome = model->getAlternatives()[n]->getAvgHouseholdIncome();
			 		unitTypeCounter = model->getAlternatives()[n]->getUnitTypeCounter();
			 		populationByunitType = model->getAlternatives()[n]->getPopulationByUnitType();
			 		sumFloorArea = model->getAlternatives()[n]->getSumFloorArea();
			 	}

				avgHouseholdSize 	= avgHouseholdSize 	  / unitTypeCounter;
				avgHouseholdIncome 	= avgHouseholdIncome  / unitTypeCounter;

				int bHighInc = 0;

				if(household->getIncome() > 7000 )
					bHighInc = 1;

				ZonalLanduseVariableValues *zonalLanduseVariableValues = model->getZonalLandUseByAlternativeId(n + 1);

				double logPopulationByHousingType	= log((double)unitTypeCounter);	//1 logarithm of population by housing type in the zone 	persons
				double populationDensity			= (double)unitTypeCounter / (double)sumFloorArea * 100.0;	//2 population density	persons per hectare
				double commercialLandFraction		= zonalLanduseVariableValues->getFLocCom();	//3 zonal average fraction of commercial land within a 500-meter buffer area from a residential postcode (weighted by no. of residential unit within the buffer)	percentage point
				double residentialLandFraction		= zonalLanduseVariableValues->getFLocRes();	//4 zonal average fraction of residential land within a 500-meter buffer area from a residential postcode  (weighted by no. of residential unit within the buffer)	percentage point
				double openSpaceFraction			= zonalLanduseVariableValues->getFLocOpen();	//5 zonal average fraction of open space within a 500-meter buffer area from a residential postcode (weighted by residential unit within the buffer)	percentage point
				double oppurtunityDiversityIndex	= zonalLanduseVariableValues->getOdi10Loc();	//6 zonal average local land use mix (opportunity diversity) index: 1-(|lu1/t-1/9|+|lu2/t-1/9|+|lu3/t-1/9|+|lu4/t-1/9|+|lu5/t-1/9|+|lu6/t-1/9|+|lu7/t-1/9|+|lu8/t-1/9|+|lu9/t-1/9|)/(16/9)
				double distanceToMrt				= zonalLanduseVariableValues->getDis2mrt();	//7 zonal average distance to the nearest MRT station	in kilometer
				double distanceToExp				= zonalLanduseVariableValues->getDis2exp();	//8 zonal average distance to the nearest express way	in kilometer
				double fractionYoungerThan4			= ( populationYoungerThan4 / populationTotal ) * bHouseholdMemberYoungerThan4;	//10 zonal fraction of population younger than 4 years old x dummy if presence of kids younger than 4 years old in the household (=1, yes; =0, no)	percentage point
				double fractionBetween5And19		= ( population5To19 / populationTotal ) * bHouseholdMember5To19;	//11 zonal fraction of population between 5 and 19 years old x dummy if presence of children in the household  (=1, yes; =0, no)	percentage point
				double fractionOlderThan65			= ( populationGreaterThan65 / populationTotal ) * bHouseholdMemberGreaterThan65;	//12 zonal fraction of population older than 65 years old x dummy if presence of seniors in the household  (=1, yes; =0, no)	percentage point
				double fractionOfChinese			= ( populationChinese / populationTotal ) * bHouseholdEthnicityChinese;	//13 zonal fraction of Chinese population x  dummy if household is Chinese (=1, yes; =0, no)	percentage point
				double fractionOfMalay				= ( populationChinese / populationTotal ) * bHouseholdEthnicityMalay;	//14 zonal fraction of Malay population x  dummy if household is Malay (=1, yes; =0, no)	percentage point
				double fractionOfIndian				= ( populationChinese / populationTotal ) * bHouseholdEthnicityIndian;	//15 zonal fraction of Indian population x  dummy if household is Indian (=1, yes; =0, no)	percentage point
				double householdSizeMinusZoneAvg	= fabs( avgHouseholdSize - household->getSize());	//16 absolute difference between zonal average household size by housing type
				double logHouseholdInconeMinusZoneAvg= fabs( log(avgHouseholdIncome ) - log(household->getIncome() ) );	//17 absolute difference between logarithm of the zonal median household montly income by housing type and logarithm of the household income	SGD
				double logZonalMedianHousingPrice	= 0.0;	//18 logarithm of the zonal median housing price by housing type	in (2005) SGD
				double privateCondoHhSizeOne		= 0.0;	//19 = 1, if household size is 1, living in private condo/apartment
				double landedPropertyHhSizeOne		= 0.0;	//20 = 1, if household size is 1, living in landed property
				double otherHousingHhSizeOne		= 0.0; 	//21 = 1, if household size is 1, living in other types of housing units
				double publicTransitTime 			= 0.0;
				double publicTransitCost 			= 0.0;
				double accessbilityManufacturJobs	= 0.0;
				double accessibilityOfficeJobs 		= 0.0;
				double hdb45						= 0.0;



				if( model->getAlternatives()[n]->getDwellingTypeId() == 600 )
				{
					if( household->getSize() == 1 )
						privateCondoHhSizeOne = 1.0;
				}
				else
				if( model->getAlternatives()[n]->getDwellingTypeId() == 700 )
				{
					if( household->getSize() == 1 )
						landedPropertyHhSizeOne = 1.0;
				}
				else
				if(model->getAlternatives()[n]->getDwellingTypeId() == 400 || model->getAlternatives()[n]->getDwellingTypeId() == 500 )
				{
					hdb45 = 1.0;
				}
				else
				{
					if( household->getSize() == 1 )
						otherHousingHhSizeOne = 1.0;
				}

				BigSerial id = 0;

				if( planningAreaWork )
					(*planningAreaWork).getId();

				std::string key = std::to_string( model->getAlternatives()[n]->getPlanAreaId() ) + "-" + std::to_string( (*planningAreaWork).getId());
				ScreeningCostTime* costTime = model->getScreeningCostTimeInst(key);

				if(costTime)
				{
					publicTransitCost = costTime->getCost();
					publicTransitTime = costTime->getTime();
				}
				else
				{
					for(int m = 0; m < model->getScreeningCostTime().size(); m++ )
					{
						if( model->getScreeningCostTime()[m]->getPlanningAreaOrigin() 	   == model->getAlternatives()[n]->getPlanAreaId() &&
							model->getScreeningCostTime()[m]->getPlanningAreaDestination() == (*planningAreaWork).getId() )
						{
							publicTransitCost = model->getScreeningCostTime()[m]->getCost();
							publicTransitTime = model->getScreeningCostTime()[m]->getTime();
						}
					}
				}

				if( !planningAreaWork )
				{
					publicTransitCost = 0;
					publicTransitTime = 0;
				}

				for( int m = 0; m < model->getAccessibilityFixedPzid().size(); m++)
				{
					if( model->getAlternatives()[n]->getPlanAreaId() == model->getAccessibilityFixedPzid()[m]->getPlanningAreaId() )
					{
						accessbilityManufacturJobs	= model->getAccessibilityFixedPzid()[m]->getAccTMfg();
						accessibilityOfficeJobs 	= model->getAccessibilityFixedPzid()[m]->getAccTOff();
					}
				}

				if( model->getAlternatives()[n]->getMedianHedonicPrice() > 0.000001 )
				{
					logZonalMedianHousingPrice =  model->getAlternatives()[n]->getMedianHedonicPrice();// / sumFloorArea / 1000;

					PrintOutV("1. n " << n << " oldmp: " <<  model->getAlternatives()[n]->getMedianHedonicPrice() << " mp: " << logZonalMedianHousingPrice << std::endl);
				}
				else
				{

					string strId = to_string(model->getAlternatives()[n]->getPlanAreaId()) + to_string(model->getAlternatives()[n]->getDwellingTypeId());
					int key = std::atoi( strId.c_str());

					typedef boost::unordered_multimap<BigSerial, AlternativeHedonicPrice*>::iterator altmm_itr;
					typedef boost::unordered_multimap<BigSerial, AlternativeHedonicPrice*> altmmap;

					std::pair< altmm_itr,altmm_itr > its = model->getAlternativeHedonicPriceById().equal_range(key);
					int count = 0;

					for (altmm_itr it = its.first; it != its.second; it++)
					{
						count++;
					}

					auto it = its.first;
					std::advance(it, count/2);

					logZonalMedianHousingPrice = it->second->getTotalPrice();

					PrintOutV("2. n " << n << " oldmp: " <<  model->getAlternatives()[n]->getMedianHedonicPrice() << " mp: " << logZonalMedianHousingPrice << std::endl);

					model->getAlternatives()[n]->setMedianHedonicPrice( logZonalMedianHousingPrice );
				}

				double probability =( logPopulationByHousingType* ln_popdwl 		) +
									( populationDensity			* den_respop_ha 	) +
									( commercialLandFraction	* f_loc_com 		) +
									( residentialLandFraction	* f_loc_res		 	) +
									( openSpaceFraction			* f_loc_open	 	) +
									( oppurtunityDiversityIndex	* odi10_loc		 	) +
									( distanceToMrt				* dis2mrt		 	) +
									( distanceToExp				* dis2exp		 	) +
									( accessbilityManufacturJobs* accmanufact_jobs 	) +
									( accessibilityOfficeJobs	* accoffice_jobs	) +
									( publicTransitTime			* pt_tt 			) +
									( publicTransitCost 		* pt_cost			) +
									( fractionYoungerThan4		* f_age4_n4		 	) +
									( fractionBetween5And19		* f_age19_n19	 	) +
									( fractionOlderThan65		* f_age65_n65	 	) +
									( fractionOfChinese			* f_chn_nchn	 	) +
									( fractionOfMalay			* f_mal_nmal	 	) +
									( fractionOfIndian			* f_indian_nind	 	) +
									( householdSizeMinusZoneAvg	* hhsize_diff	 	) +
									( logHouseholdInconeMinusZoneAvg * log_hhinc_diff ) +
									( logZonalMedianHousingPrice* log_price05tt_med ) +
									( privateCondoHhSizeOne		* DWL600 * bHighInc ) +
									( landedPropertyHhSizeOne	* DWL700 * bHighInc ) +
									( otherHousingHhSizeOne		* DWL800 * bHighInc ) +
									( hdb45						* DWL400_500 );

				/*
				PrintOut("n: " <<    populationByunitType 		<< " 0 " << planningArea->getId()  << " hhid:  " << hhId << " " <<
									 logPopulationByHousingType << " 1 " << ln_popdwl 		 << " " <<
									 populationDensity			<< " 2 " << den_respop_ha 	 << " " <<
									 commercialLandFraction	    << " 3 " << f_loc_com 	 	 << " " <<
									 residentialLandFraction	<< " 4 " << f_loc_res		 << " " <<
									 openSpaceFraction			<< " 5 " << f_loc_open	 	 << " " <<
									 oppurtunityDiversityIndex	<< " 6 " << odi10_loc		 << " " <<
									 distanceToMrt				<< " 7 " << dis2mrt		 	 << " " <<
									 distanceToExp				<< " 8 " << dis2exp		 	 << " " <<
									 householdWorkerLogsumAverage << " 9 " << hh_dgp_w_lgsm1 << " " <<
									 fractionYoungerThan4		<< " a " << f_age4_n4		 << " " <<
									 fractionBetween5And19		<< " b " << f_age19_n19	 	 << " " <<
									 fractionOlderThan65		<< " c " << f_age65_n65	 	 << " " <<
									 fractionOfChinese			<< " d " << f_chn_nchn	 	 << " " <<
									 fractionOfMalay			<< " e " << f_mal_nmal	 	 << " " <<
									 fractionOfIndian			<< " f " << f_indian_nind	 << " " <<
									 householdSizeMinusZoneAvg	<< " g " << hhsize_diff	 	 << " " <<
									 logHouseholdInconeMinusZoneAvg << " h " << log_hhinc_diff << " " <<
									 logZonalMedianHousingPrice << " i " << log_price05tt_med  << " " <<
									 privateCondoHhSizeOne		<< " j " << DWL600  << " " <<
									 landedPropertyHhSizeOne	<< " k " << DWL700  << " " <<
									 otherHousingHhSizeOne		<< " l " << DWL800  << std::endl);
				*/

				if( std::isnan(probability) )
					probability = 0.0;

				probabilities.push_back(probability);

				probabilitySum += exp(probability);
			}

			for( int n = 0; n < probabilities.size(); n++)
			{
				probabilities[n] = exp(probabilities[n])/ probabilitySum;
			}


			/*
			// NOTE: dgp is the planning area
			//1. population grouped by planning area (55 areas)
			//2. start with taz. Cummulate area and population up to planning area.
			//3. grab from shan's excel
			//4. grab from shan's excel
			//5. grab from shan's excel
			//6. grab from shan's excel
			//7. grab from shan's excel
			//8. grab from shan's excel
			//9.a) Check if there are any workers in the household. If not, skip the steps below.
				b) Get number of hhs in each taz within the dgp.
				c) Compute the logsum for each worker in candidate household if workplace and vehicle are unchanged and residents is moved to each taz within the dgp.
				d) Compute the weighted average of the logsum rtp the number of households per taz.
				e) Do a simple average if the number of workers > 1.
			//10.	a) Check if there are any kids under 4 in the hh. If yes, go to b)
				b) NUmber of people under 4 divided by total number of people in that dgp
			//11.	a) Check if there are any kids 5 - 19 in the hh. If yes, go to b)
				b) NUmber of people 5 - 19 divided by total number of people in that dgp
			//12.	a) Check if there are people > 65 in the hh. If yes, go to b)
				b) NUmber of people >65 divided by total number of people in that dgp
			//13. If hh is chinese, find the fraction of chinese people in the planning area
			//14. If hh is malay, find the fraction of chinese people in the planning area
			//15. If hh is indian, find the fraction of chinese people in the planning area
			//16. absolute difference between zonal average household size by housing type and household size	persons
			//17. abs diff in hh median income
			//18. Check with Yi about sale price. Else use the hedonic price for now
			//19. 1 if condo
			//20. 1 if landed
			//21. 1 if other
			 *
			 *
			 * 22. Multiply each coeff with values computed above a sum them up. This is refered to as R
			 * 23. Probability = eR/(1+eR)
			*/
		}

	}
}
