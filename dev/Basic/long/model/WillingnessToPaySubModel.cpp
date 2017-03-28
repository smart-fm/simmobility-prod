//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * WillingnessToPaySubModel.cpp
 *
 *  Created on: 29 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *  Modelling constants:    Roberto Lopez <rponcelo@mit.edu> 	
 */

#include "database/entity/PostcodeAmenities.hpp"
#include "database/entity/Individual.hpp"
#include "database/entity/Taz.hpp"
#include "database/entity/HouseHoldHitsSample.hpp"
#include "database/entity/Job.hpp"
#include <boost/random/mersenne_twister.hpp>
#include <model/WillingnessToPaySubModel.hpp>
#include "core/AgentsLookup.hpp"
#include "core/DataManager.hpp"
#include "behavioral/PredayLT_Logsum.hpp"

namespace sim_mob
{
	namespace long_term
	{
		namespace
		{
			inline void printHouseholdGroupLogsum( int homeTaz,  int group, BigSerial hhId, double logsum )
			{
				boost::format fmtr = boost::format("%1%, %2%, %3%, %4%") % homeTaz % group % hhId % logsum;
				AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLDGROUPLOGSUM,fmtr.str());
			}
		}

		WillingnessToPaySubModel::WillingnessToPaySubModel(){}

		WillingnessToPaySubModel::~WillingnessToPaySubModel(){}

		void WillingnessToPaySubModel::FindHDBType( int unitType)
		{
			if( unitType == ID_HDB1 || unitType == ID_HDB2 )
				HDB12 = 1;
			else
			if( unitType == ID_HDB3 )
				HDB3 = 1;
			else
			if( unitType == ID_HDB4 )
				HDB4 = 1;
			else
			if( unitType == ID_HDB5 )
				HDB5 = 1;
			else
			if( unitType >= ID_APARTM70 && unitType <= ID_APARTM159 )
				Apartment = 1;
			else
			if( unitType >= ID_CONDO60 && unitType <= ID_CONDO134 )
				Condo = 1;
			else
			if( unitType >= ID_TERRACE180 && unitType <= ID_TERRACE379 )
				Terrace = 1;
			else
			if( unitType >= ID_SEMID230 && unitType <= ID_DETACHED1199 )
				DetachedAndSemidetaced = 1;
			else
			if( unitType == 6 )
				HDB5 = 1;
			else
			if( unitType >= 32 && unitType <= 51 )
				Condo = 1;
			else
			if( unitType == 64 )
				Apartment = 1;
			else
			if( unitType == 65 )
				HDB5 = 1;
		}

		void WillingnessToPaySubModel::FindHouseholdSize(const Household *household)
		{
			if( household->getSize() == 1)
				HH_size1 = 1;
			else
			if( household->getSize() == 2)
				HH_size2 = 1;
			else
				HH_size3m = 1;
		}

		void WillingnessToPaySubModel::FindAgeOfUnit(const Unit *unit, int day)
		{
			ageOfUnitPrivate = HITS_SURVEY_YEAR  - 1900 + ( day / 365 ) - unit->getOccupancyFromDate().tm_year;

			ZZ_ageOfUnitPrivate	 = ageOfUnitPrivate;
			ZZ_missingAge    = 0;
			ZZ_freehold 	 = 0;

			if( ageOfUnitPrivate > 50 )
				ZZ_ageOfUnitPrivate = 50;

			if( ageOfUnitPrivate < 0 )
				ZZ_ageOfUnitPrivate = 0;

			ZZ_ageOfUnitPrivate = ZZ_ageOfUnitPrivate / 10.0;

			ageOfUnitHDB = HITS_SURVEY_YEAR - 1900 + ( day / 365 ) - unit->getOccupancyFromDate().tm_year;
			ZZ_ageOfUnitHDB	 = ageOfUnitHDB;

			if( ageOfUnitHDB > 40 )
				ZZ_ageOfUnitHDB = 40;

			if( ageOfUnitHDB  < 0 )
				ZZ_ageOfUnitHDB = 0;

			ZZ_ageOfUnitHDB = ZZ_ageOfUnitHDB / 10.0;

		}

		void WillingnessToPaySubModel::GetLogsum(HM_Model *model, const Household *household, int day)
		{
			BigSerial homeTaz = 0;
			BigSerial workTaz = 0;
			Individual* headOfHousehold = NULL;

			std::vector<BigSerial> householdOccupants = household->getIndividuals();

			for( int n = 0; n < householdOccupants.size(); n++ )
			{
				Individual * householdIndividual = model->getIndividualById( householdOccupants[n] );

				if( householdIndividual->getHouseholdHead() )
				{
					headOfHousehold = householdIndividual;
				}
			}

			//This household does not seem to have an head of household, let's just assign one.
			if(headOfHousehold == NULL)
			{
				int eldestHouseholdMemberAge = 0;
				for( int n = 0; n < householdOccupants.size(); n++ )
				{
					Individual * householdIndividual = model->getIndividualById( householdOccupants[n] );
					std::tm dob = householdIndividual->getDateOfBirth();

					int age = HITS_SURVEY_YEAR  - 1900 + ( day / 365 ) - dob.tm_year;

					if( age > eldestHouseholdMemberAge )
					{
						age =  eldestHouseholdMemberAge;
						headOfHousehold = householdIndividual;
					}
				}
			}

			Job *job = model->getJobById(headOfHousehold->getJobId());

			BigSerial hometazId = model->getUnitTazId( household->getUnitId() );
			Taz *homeTazObj = model->getTazById( hometazId );

			std::string homeTazStr;
			if( homeTazObj != NULL )
				homeTazStr = homeTazObj->getName();

			homeTaz = std::atoi( homeTazStr.c_str() );

			BigSerial worktazId = model->getEstablishmentTazId( job->getEstablishmentId() );
			Taz *workTazObj = model->getTazById( worktazId );

			std::string workTazStr;
			if( workTazObj != NULL )
				workTazStr =  workTazObj->getName();

			workTaz = std::atoi( workTazStr.c_str());

			if( workTazStr.size() == 0 )
			{
				AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "workTaz is empty for person:  %1%.") %  headOfHousehold->getId()).str());
				workTaz = homeTaz;
			}

			if( homeTazStr.size() == 0 )
			{
				AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "homeTaz is empty for person:  %1%.") %  headOfHousehold->getId()).str());
				homeTaz = -1;
				workTaz = -1;
			}

			if( homeTaz == -1 || workTaz == -1 )
			{
				ZZ_logsumhh = 0;
			}
			else
			{
				HouseHoldHitsSample *hitssample = model->getHouseHoldHitsById( household->getId() );

				for(int n = 0; n < model->householdGroupVec.size(); n++ )
				{
					BigSerial thisGroupId = model->householdGroupVec[n].getGroupId();
					BigSerial thisHomeTaz = model->householdGroupVec[n].getHomeTaz();

					if( thisGroupId == hitssample->getGroupId() &&  thisHomeTaz == homeTaz )
					{
						ZZ_logsumhh = model->householdGroupVec[n].getLogsum();
						break;
					}
				}

				if( ZZ_logsumhh == -1 )
				{
					PersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( headOfHousehold->getId(), homeTaz, workTaz, household->getVehicleOwnershipOptionId() );
					ZZ_logsumhh = personParam.getDpbLogsum();

					BigSerial groupId = hitssample->getGroupId();
					boost::shared_ptr<HM_Model::HouseholdGroup> thisHHGroup(new HM_Model::HouseholdGroup(groupId, homeTaz, ZZ_logsumhh ));

					model->householdGroupVec.push_back( *(thisHHGroup.get()));

					printHouseholdGroupLogsum( homeTaz, hitssample->getGroupId(), headOfHousehold->getId(), ZZ_logsumhh );
				}

				Household* householdT = const_cast<Household*>(household);
				householdT->setLogsum(ZZ_logsumhh);

			}

		}

		void WillingnessToPaySubModel::GetIncomeAndEthnicity(HM_Model *model, const Household *household, const Unit *unit)
		{
			const HM_Model::TazStats *tazstats  = model->getTazStatsByUnitId(unit->getId());

			if( tazstats->getChinesePercentage() > 0.76 ) //chetan TODO: add to xml file
				ZZ_hhchinese = 1;

			if( tazstats->getMalayPercentage() > 0.10 )
				ZZ_hhmalay 	 = 1;

			ZZ_highInc = household->getIncome();
			ZZ_middleInc = household->getIncome();
			ZZ_lowInc  =  household->getIncome();

			if( ZZ_highInc >= 11000 )
				ZZ_highInc = 1;
			else
				ZZ_highInc = 0;


			if( ZZ_middleInc >= 2750 && ZZ_middleInc < 11000)
				ZZ_middleInc = 1;
			else
				ZZ_middleInc = 0;


			if( ZZ_lowInc < 2750 )
				ZZ_lowInc = 1;
			else
				ZZ_lowInc = 0;

			ZZ_children = 0;

			if( household->getChildUnder15() > 0 )
				ZZ_children = 1;


			if( household->getEthnicityId() == 1 )
				chineseHousehold = 1;

			if( household->getEthnicityId() == 2 )
				malayHousehold = 1;

			if( household->getVehicleOwnershipOptionId() > 0)
				carOwnershipBoolean = 1;
		}


		double WillingnessToPaySubModel::CalculateWillingnessToPay(const Unit* unit, const Household* household, double& wtp_e, double day, HM_Model *model)
		{
			double V;

			const PostcodeAmenities *pcAmenities = DataManagerSingleton::getInstance().getAmenitiesById( model->getUnitSlaAddressId( unit->getId() ) );

			int unitType = unit->getUnitType();

			int sizeAreaQuantileHDB = 0;
			int sizeAreaQuantileCondo = 0;
			double lgsqrtArea = log(sqrt(unit->getFloorArea()));
			double lowerQuantileCondo = model->getlogSqrtFloorAreahdb()[ model->getlogSqrtFloorAreahdb().size() * 0.3 ];
			double upperQuantileCondo = model->getlogSqrtFloorAreahdb()[ model->getlogSqrtFloorAreahdb().size() * 0.5 ];
			double lowerQuantileHDB = model->getlogSqrtFloorAreahdb()[ model->getlogSqrtFloorAreahdb().size() * 0.4 ];
			double upperQuantileHDB = model->getlogSqrtFloorAreahdb()[ model->getlogSqrtFloorAreahdb().size() * 0.6 ];

			if( lgsqrtArea >=  lowerQuantileCondo && lgsqrtArea < upperQuantileCondo )
			{
				sizeAreaQuantileCondo = 0;
			}

			if( lgsqrtArea >=  lowerQuantileHDB && lgsqrtArea < upperQuantileHDB )
			{
				sizeAreaQuantileHDB = 0;
			}

			//We use a separate list of coefficients for HDB units.
			if( unitType <= 6  || unitType == 65 )
			{
				sde			=  0.2079816511;
				barea		=  1.3174741336;
				blogsum		=  5.6119278112;
				bsizearea	=  0.0182733682;
				bcar		= -2.9867669658;
				bcarlgs		=  0.5766951069;
			}

			FindHDBType(unitType);
			FindHouseholdSize(household);

			DD_area = log(unit->getFloorArea()/10);

			FindAgeOfUnit( unit, day);

			//GetLogsum(model, household, day);
			Postcode *unitPostcode = model->getPostcodeById( model->getUnitSlaAddressId( unit->getId() ) );
			ZZ_logsumhh = model->ComputeHedonicPriceLogsumFromDatabase( unitPostcode->getTazId() );


			vector<BigSerial>ind_hh = household->getIndividuals();

			Individual *thisIndividual;
			for(int n = 0; n < ind_hh.size();n++)
			{
				Individual *tempIndividual = model->getIndividualById(ind_hh[n]);

				if( tempIndividual->getHouseholdHead() )
					thisIndividual = tempIndividual;
			}

			Job *job = model->getJobById(thisIndividual->getJobId());
			Establishment *establishment = model->getEstablishmentById(	job->getEstablishmentId());
			int work_tazId = model->getEstablishmentTazId( establishment->getId() );

			if(work_tazId==682||work_tazId==683||work_tazId==684||work_tazId==697||work_tazId==698||work_tazId==699||work_tazId==700||work_tazId==702||work_tazId==703||work_tazId==927||work_tazId==928||work_tazId==929||work_tazId==930||work_tazId==931||work_tazId==932||work_tazId==255||work_tazId==1256)
				ZZ_logsumhh += 0.07808;

		    int tazId = model->getUnitTazId( unit->getId() );

		    if(tazId==682||tazId==683||tazId==684||tazId==697||tazId==698||tazId==699||tazId==700||tazId==702||tazId==703||tazId==927||tazId==928||tazId==929||tazId==930||tazId==931||tazId==932||tazId==1255||tazId==1256)
				ZZ_logsumhh += 0.07808;



			Household* householdT = const_cast<Household*>(household);
			householdT->setLogsum(ZZ_logsumhh);

			GetIncomeAndEthnicity(model, household, unit);

			const PostcodeAmenities *amenities = DataManagerSingleton::getInstance().getAmenitiesById( unitPostcode->getAddressId() );

			int busDistanceBool = 0;
			if( amenities->getDistanceToBus() > 200 && amenities->getDistanceToBus() < 400 )
				busDistanceBool = 1;

			double mallDistance = amenities->getDistanceToMall();

			int mallDistanceBool = 0;

			if( amenities->getDistanceToMall() > 200 && amenities->getDistanceToMall() < 400 )
				mallDistanceBool = 1;

			double Vpriv = 	(barea		*  DD_area 		) +
							(blogsum	* ZZ_logsumhh 	) +
							(bsizearea	 * sizeAreaQuantileCondo) +
							(bcar * carOwnershipBoolean ) +
							(bcarlgs * carOwnershipBoolean  * ZZ_logsumhh ) +
							(bbus2400 * busDistanceBool) +

							(bapartment  * Apartment ) +
							(bcondo 	 * Condo 	 ) +
							(bdetachedAndSemiDetached * DetachedAndSemidetaced ) +
							(terrace	* Terrace		) +
							(bmissingAge  			* ZZ_missingAge 	) +
							(bfreeholdAppartm  		* ZZ_freehold * Apartment 	) +
							(bfreeholdCondo  		* ZZ_freehold * Condo 		) +
							(fbreeholdTerrace  		* ZZ_freehold * Terrace 	);


			double Vhdb = 	(barea		*  DD_area 		) +
							(blogsum	* ZZ_logsumhh 	) +
							(bsizearea	 * sizeAreaQuantileHDB) +
							(bcar * carOwnershipBoolean ) +
							(bcarlgs * carOwnershipBoolean  * ZZ_logsumhh ) +
							(bmall * mallDistance) +
							(bmrt2400m * mallDistanceBool ) +
							(bhdb12  * HDB12 ) +
							(bhdb3   * HDB3  ) +
							(bhdb4 	 * HDB4	 ) +
							(bhdb5 	 * HDB5	 ) +
							(bageOfUnit30 * ZZ_ageOfUnitHDB ) +
							(bageOfUnit30Squared * ZZ_ageOfUnitHDB * ZZ_ageOfUnitHDB );

			if( unit->getUnitType() <= 6 || unitType == 65 )
				V = Vhdb;
			else
				V = Vpriv;

			boost::mt19937 rng( clock() );
			boost::normal_distribution<> nd( 0.0, sde);
			boost::variate_generator<boost::mt19937&,  boost::normal_distribution<> > var_nor(rng, nd);
			wtp_e  = var_nor();

			//needed when wtp model is expressed as log wtp
			V = exp(V);

			return V;
		}
	}

}
