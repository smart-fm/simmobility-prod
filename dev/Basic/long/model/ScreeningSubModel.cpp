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

namespace sim_mob
{
	namespace long_term
	{

		ScreeningSubModel::ScreeningSubModel() {}

		ScreeningSubModel::~ScreeningSubModel() {}


		void ScreeningSubModel::getScreeningProbabilities(int hhId, std::vector<double> &probabilities, HM_Model *model, int day )
		{
			double ln_popdwl		= 0.9701;	//1 logarithm of population by housing type in the zone 	persons
			double den_respop_ha	= 0.0257;	//2 population density	persons per hectare (x10^-2)
			double f_loc_com		= 0.0758;	//3 zonal average fraction of commercial land within a 500-meter buffer area from a residential postcode (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
			double f_loc_res		= 0.0676;	//4 zonal average fraction of residential land within a 500-meter buffer area from a residential postcode  (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
			double f_loc_open		= 0.0841;	//5 zonal average fraction of open space within a 500-meter buffer area from a residential postcode (weighted by residential unit within the buffer)	percentage point (x10^-1)
			double odi10_loc		= 0.0928;	//6 zonal average local land use mix (opportunity diversity) index: 1-(|lu1/t-1/9|+|lu2/t-1/9|+|lu3/t-1/9|+|lu4/t-1/9|+|lu5/t-1/9|+|lu6/t-1/9|+|lu7/t-1/9|+|lu8/t-1/9|+|lu9/t-1/9|)/(16/9)	(x10)
			double dis2mrt			=-0.3063;	//7 zonal average distance to the nearest MRT station	in kilometer
			double dis2exp			= 0.0062;	//8 zonal average distance to the nearest express way	in kilometer
			double hh_dgp_w_lgsm1	= 0.8204;	//9 average of workers' logsum of a household (at the DGP level) x dummy if household has at least a worker with fixed workplace (=1, yes; =0, otherwise)	utils
			double f_age4_n4		= 1.5187;	//10 zonal fraction of population younger than 4 years old x dummy if presence of kids younger than 4 years old in the household (=1, yes; =0, no)	percentage point (x10^-1)
			double f_age19_n19		= 0.3068;	//11 zonal fraction of population between 5 and 19 years old x dummy if presence of children in the household  (=1, yes; =0, no)	percentage point (x10^-1)
			double f_age65_n65		= 0.7503;	//12 zonal fraction of population older than 65 years old x dummy if presence of seniors in the household  (=1, yes; =0, no)	percentage point (x10^-1)
			double f_chn_nchn		= 0.1689;	//13 zonal fraction of Chinese population x  dummy if household is Chinese (=1, yes; =0, no)	percentage point (x10^-1)
			double f_mal_nmal		= 0.4890;	//14 zonal fraction of Malay population x  dummy if household is Malay (=1, yes; =0, no)	percentage point (x10^-1)
			double f_indian_nind	= 0.8273;	//15 zonal fraction of Indian population x  dummy if household is Indian (=1, yes; =0, no)	percentage point (x10^-1)
			double hhsize_diff		=-0.5926;	//16 absolute difference between zonal average household size by housing type and household size	persons
			double log_hhinc_diff	=-1.5749;	//17 absolute difference between logarithm of the zonal median household montly income by housing type and logarithm of the household income	SGD
			double log_price05tt_med=-0.1473;	//18 logarithm of the zonal median housing price by housing type	in (2005) SGD
			double DWL600			= 0.3940;	//19 = 1, if household size is 1, living in private condo/apartment
			double DWL700			= 0.3254;	//20 = 1, if household size is 1, living in landed property
			double DWL800			= 0.3394; 	//21 = 1, if household size is 1, living in other types of housing units

			//HM_Model *model = getParent()->getModel();
			Household* household = model->getHouseholdById(hhId);
			const Unit* unit = model->getUnitById( household->getUnitId() );
			int tazId = model->getUnitTazId( household->getUnitId() );
			Taz *taz  = model->getTazById(tazId);
			int mtzId = model->getMtzIdByTazId(tazId);
			Mtz *mtz  = model->getMtzById(mtzId);

			PlanningSubzone *planningSubzone = nullptr;
			PlanningArea *planningArea = nullptr;
			Alternative* alternative = nullptr;

			if(mtz)
				planningSubzone = model->getPlanningSubzoneById( mtz->getPlanningSubzoneId() );

			if(planningSubzone)
				planningArea = model->getPlanningAreaById(planningSubzone->getPlanningAreaId() );

			if(planningArea)
				alternative = model->getAlternativeByPlanningAreaId(planningArea->getId());

			int dwellingId = 0;

			if(alternative)
				dwellingId = alternative->getDwellingTypeId();

			if(!planningArea)
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

				if( populationPerPlanningArea[n]->getUnitType() == unit->getUnitType() )
				{
					avgHouseholdSize += populationPerPlanningArea[n]->getAvgHhSize();
					avgHouseholdIncome += populationPerPlanningArea[n]->getAvgIncome();
					unitTypeCounter++;
					populationByunitType += populationPerPlanningArea[n]->getPopulation();
				}
			}

			avgHouseholdSize 	= avgHouseholdSize 	  / unitTypeCounter;
			avgHouseholdIncome 	= avgHouseholdIncome  / unitTypeCounter;

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

		 	double probabilitySum = 0;

			for( int n = 1; n <= model->getAlternatives().size(); n++ )
			{
				ZonalLanduseVariableValues *zonalLanduseVariableValues = model->getZonalLandUseByAlternativeId(n);

				double logPopulationByHousingType	= log((double)populationByunitType);	//1 logarithm of population by housing type in the zone 	persons
				double populationDensity			= (double)populationByunitType / planningArea_size;	//2 population density	persons per hectare (x10^-2)
				double commercialLandFraction		= zonalLanduseVariableValues->getFLocCom();	//3 zonal average fraction of commercial land within a 500-meter buffer area from a residential postcode (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
				double residentialLandFraction		= zonalLanduseVariableValues->getFLocRes();	//4 zonal average fraction of residential land within a 500-meter buffer area from a residential postcode  (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
				double openSpaceFraction			= zonalLanduseVariableValues->getFLocOpen();	//5 zonal average fraction of open space within a 500-meter buffer area from a residential postcode (weighted by residential unit within the buffer)	percentage point (x10^-1)
				double oppurtunityDiversityIndex	= zonalLanduseVariableValues->getOdi10Loc();	//6 zonal average local land use mix (opportunity diversity) index: 1-(|lu1/t-1/9|+|lu2/t-1/9|+|lu3/t-1/9|+|lu4/t-1/9|+|lu5/t-1/9|+|lu6/t-1/9|+|lu7/t-1/9|+|lu8/t-1/9|+|lu9/t-1/9|)/(16/9)	(x10)
				double distanceToMrt				= zonalLanduseVariableValues->getDis2mrt();	//7 zonal average distance to the nearest MRT station	in kilometer
				double distanceToExp				= zonalLanduseVariableValues->getDis2exp();	//8 zonal average distance to the nearest express way	in kilometer
				double householdWorkerLogsumAverage	= 0.0;	//9 average of workers' logsum of a household (at the DGP level) x dummy if household has at least a worker with fixed workplace (=1, yes; =0, otherwise)	utils
				double fractionYoungerThan4			= ( populationYoungerThan4 / populationTotal ) * bHouseholdMemberYoungerThan4;	//10 zonal fraction of population younger than 4 years old x dummy if presence of kids younger than 4 years old in the household (=1, yes; =0, no)	percentage point (x10^-1)
				double fractionBetween5And19		= ( population5To19 / populationTotal ) * bHouseholdMember5To19;	//11 zonal fraction of population between 5 and 19 years old x dummy if presence of children in the household  (=1, yes; =0, no)	percentage point (x10^-1)
				double fractionOlderThan65			= ( populationGreaterThan65 / populationTotal ) * bHouseholdMemberGreaterThan65;	//12 zonal fraction of population older than 65 years old x dummy if presence of seniors in the household  (=1, yes; =0, no)	percentage point (x10^-1)
				double fractionOfChinese			= ( populationChinese / populationTotal ) * bHouseholdEthnicityChinese;	//13 zonal fraction of Chinese population x  dummy if household is Chinese (=1, yes; =0, no)	percentage point (x10^-1)
				double fractionOfMalay				= ( populationChinese / populationTotal ) * bHouseholdEthnicityMalay;	//14 zonal fraction of Malay population x  dummy if household is Malay (=1, yes; =0, no)	percentage point (x10^-1)
				double fractionOfIndian				= ( populationChinese / populationTotal ) * bHouseholdEthnicityIndian;	//15 zonal fraction of Indian population x  dummy if household is Indian (=1, yes; =0, no)	percentage point (x10^-1)
				double householdSizeMinusZoneAvg	= fabs( avgHouseholdSize - household->getSize());	//16 absolute difference between zonal average household size by housing type
				double logHouseholdInconeMinusZoneAvg= fabs( log(avgHouseholdIncome ) - log(household->getIncome() ) );	//17 absolute difference between logarithm of the zonal median household montly income by housing type and logarithm of the household income	SGD
				double logZonalMedianHousingPrice	= 0.0;	//18 logarithm of the zonal median housing price by housing type	in (2005) SGD
				double privateCondoHhSizeOne		= 0.0;	//19 = 1, if household size is 1, living in private condo/apartment
				double landedPropertyHhSizeOne		= 0.0;	//20 = 1, if household size is 1, living in landed property
				double otherHousingHhSizeOne		= 0.0; 	//21 = 1, if household size is 1, living in other types of housing units

				if( household->getSize() == 1 )
				{
					if( unit->getUnitType() >= 12 && unit->getUnitType() <= 16 )
						privateCondoHhSizeOne = 1.0;
					else
					if( unit->getUnitType() >= 17 && unit->getUnitType() <= 31 )
						landedPropertyHhSizeOne = 1.0;
					else
						otherHousingHhSizeOne = 1.0;
				}

				double costTime = 0;
				double accessbility_fixed_pzid = 0;

				if(household->getWorkers() != 0 )
				{
					std::vector<double> workerLogsumAtPlanningAreaLevel;
					std::vector<BigSerial> individuals = household->getIndividuals();
					int tazPopulation = 0;

					for(int m = 0; m < individuals.size(); m++)
					{
						Individual *individual = model->getIndividualById(individuals[m]);
						double logsum = 0;

						if( individual->getEmploymentStatusId() <= 3) //1:fulltime. 2:partime 3:self-employed
						{
							tazPopulation = 0;
							int patSize = planningAreaTazs.size();
							for( int p = 0; p < patSize; p++)
							{
								Taz *thisTaz = model->getTazById(planningAreaTazs[n]);

								if( thisTaz )
								{
									const HM_Model::TazStats *tazStats = model->getTazStats(thisTaz->getId());

									if( tazStats )
									{
										tazPopulation += tazStats->getIndividuals();

										HouseHoldHitsSample *hitsSample = model->getHouseHoldHitsById( household->getId() );
										int tazH = atoi(thisTaz->getName().c_str());

										int p = 0;
										int tazIdW = -1;
										for(p = 0; p < model->getHitsIndividualLogsumVec().size(); p++ )
										{
											if ( model->getHitsIndividualLogsumVec()[p]->getHitsId().compare( hitsSample->getHouseholdHitsId() ) == 0 )
											{
												tazIdW = model->getHitsIndividualLogsumVec()[p]->getWorkTaz();
												break;
											}
										}

										Taz *tazObjW = model->getTazById( tazIdW );
									    std::string tazStrW;
										if( tazObjW != NULL )
											tazStrW = tazObjW->getName();
										BigSerial tazW = std::atoi( tazStrW.c_str() );

										double lg =  0;
										int vehicleOwnership = 0;

										if( individual->getVehicleCategoryId() > 0 )
											vehicleOwnership = 1;

										PredayPersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( individuals[m] , tazH, tazW, vehicleOwnership );
										lg = personParam.getDpbLogsum(); //2.71 use this value as an average for testing purposes


										logsum = logsum + lg * (double)(tazStats->getIndividuals());
									}
								}
							}


							if( tazPopulation && patSize )
							{
								logsum = logsum / tazPopulation / patSize; //TODO: check the avg logsum computation. might not need tazpopulation.
							}

							workerLogsumAtPlanningAreaLevel.push_back(logsum);
						}
					}

					for( int m = 0; m < workerLogsumAtPlanningAreaLevel.size(); m++ )
					{
						householdWorkerLogsumAverage = householdWorkerLogsumAverage + ( workerLogsumAtPlanningAreaLevel[m] / workerLogsumAtPlanningAreaLevel.size() );
					}
				}

				HedonicPrice_SubModel hpSubmodel(day, model, const_cast<Unit*>(unit));

				std::vector<ExpectationEntry> expectations;
				hpSubmodel.ComputeExpectation(1, expectations);

				logZonalMedianHousingPrice = log(expectations[0].hedonicPrice);




				double probability =( logPopulationByHousingType* ln_popdwl 		) +
									( populationDensity			* den_respop_ha 	) +
									( commercialLandFraction	* f_loc_com 		) +
									( residentialLandFraction	* f_loc_res		 	) +
									( openSpaceFraction			* f_loc_open	 	) +
									( oppurtunityDiversityIndex	* odi10_loc		 	) +
									( distanceToMrt				* dis2mrt		 	) +
									( distanceToExp				* dis2exp		 	) +
									( householdWorkerLogsumAverage* hh_dgp_w_lgsm1 	) +
									( fractionYoungerThan4		* f_age4_n4		 	) +
									( fractionBetween5And19		* f_age19_n19	 	) +
									( fractionOlderThan65		* f_age65_n65	 	) +
									( fractionOfChinese			* f_chn_nchn	 	) +
									( fractionOfMalay			* f_mal_nmal	 	) +
									( fractionOfIndian			* f_indian_nind	 	) +
									( householdSizeMinusZoneAvg	* hhsize_diff	 	) +
									( logHouseholdInconeMinusZoneAvg * log_hhinc_diff ) +
									( logZonalMedianHousingPrice* log_price05tt_med ) +
									( privateCondoHhSizeOne		* DWL600 ) +
									( landedPropertyHhSizeOne	* DWL700 ) +
									( otherHousingHhSizeOne		* DWL800 );

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

			//PrintOut(std::endl);

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
