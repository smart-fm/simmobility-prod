//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * WillingnessToPaySubModel.cpp
 *
 *  Created on: 29 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
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

		double WillingnessToPaySubModel::CalculateWillingnessToPay(const Unit* unit, const Household* household, double& wtp_e, double day, HM_Model *model)
		{
			double V;

			//
			//These constants are extracted from Roberto Ponce's bidding model
			//
			/* willingness to pay in million of dollars*/
			double sde		=  0.1;
			double barea	=  0.6922591951;
			double blogsum	=  0.0184661069;
			double bchin	= -0.0727597459;
			double bmalay	= -0.3067308978;
			double bHighInc =  0.0558738418;
			const double bHIncChildApart  	=  0.085270578;
			const double bHIncChildCondo  	= -0.0496929496;
			const double bapartment 		= -3.1147976249;
			const double bcondo 			= -2.9582377947;
			const double bdetachedAndSemiDetached = -2.6753868759;
			const double terrace 			= -2.9801756451;
			const double bageOfUnit25		= -0.0432841653;
			const double bageOfUnit25Squared= -0.0164360119;
			const double bageGreaterT25LessT50 =  0.1883170202;
			const double bageGreaterT50 	=  0.3565907423;
			const double bmissingAge 		= -0.1679748285;
			const double bfreeholdAppartm 	=  0.599136353;
			const double bfreeholdCondo 	=  0.4300148333;
			const double fbreeholdTerrace 	=  0.3999045196;

			const double midIncChildHDB3 = -0.0044485643;
			const double midIncChildHDB4 = -0.0068614137;
			const double midIncChildHDB5 = -0.0090473027;

			const double bhdb12	=	-3.7770973415;
			const double bhdb3  =  	-3.4905971667;
			const double bhdb4 	=	-3.4851295051;
			const double bhdb5	=	-3.5070548459;
			const double bageOfUnit30 = -0.7012864149;
			const double bageOfUnit30Squared = 0.1939266362;
			const double bageOfUnitGreater30 = 0.0521622428;

			const PostcodeAmenities* pcAmenities = DataManagerSingleton::getInstance().getAmenitiesById( unit->getSlaAddressId() );

			double Apartment	= 0;
			double Condo		= 0;
			double DetachedAndSemidetaced	= 0;
			double Terrace		= 0;
			double HDB12 		= 0;
			double HDB3			= 0;
			double HDB4			= 0;
			double HDB5			= 0;
			double HH_size1		= 0;
			double HH_size2		= 0;
			double HH_size3m	= 0;
			double DD_area		= 0;
			double ZZ_logsumhh	=-1;
			double ZZ_hhchinese = 0;
			double ZZ_hhmalay	= 0;
			double ZZ_hhindian	= 0;
			double ZZ_hhinc		= 0;
			double ZZ_hhsize	= 0;

			int unitType = unit->getUnitType();

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
			else
				return 0.0;


			if( unitType <= 6  || unitType == 65 )
			{
				sde 	 = 0.05;
				barea 	 = 0.8095874824;
				blogsum	 = 0.0035517989;
				bchin 	 = 0.0555546991;
				bmalay 	 = -0.0056135472;
				bHighInc = 0.0229342784;
			}

			if( household->getSize() == 1)
				HH_size1 = 1;
			else
			if( household->getSize() == 2)
				HH_size2 = 1;
			else
				HH_size3m = 1;

			DD_area = log( unit->getFloorArea() );

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

			const int ageOfUnitPrivate = HITS_SURVEY_YEAR  - 1900 + ( day / 365 ) - unit->getPhysicalFromDate().tm_year;


			double ZZ_ageOfUnitPrivate	 = ageOfUnitPrivate;
			int ZZ_ageBet25And50 = 0;
			int ZZ_ageGreater50  = 0;
			int ZZ_missingAge    = 0;
			int ZZ_freehold 	 = 0;

			if( ageOfUnitPrivate > 25 )
				ZZ_ageOfUnitPrivate = 25;

			if( ageOfUnitPrivate < 0 )
				ZZ_ageOfUnitPrivate = 0;

			ZZ_ageOfUnitPrivate = ZZ_ageOfUnitPrivate / 10.0;

			if( ageOfUnitPrivate > 25 && ageOfUnitPrivate < 50)
				ZZ_ageBet25And50 = 1;

			if( ageOfUnitPrivate > 50 )
				ZZ_ageGreater50 = 1;


			const int ageOfUnitHDB = HITS_SURVEY_YEAR - 1900 + ( day / 365 ) - unit->getPhysicalFromDate().tm_year;
			double ZZ_ageOfUnitHDB	 = ageOfUnitHDB;
			int ZZ_ageGreater30  = 0;

			if( ageOfUnitHDB > 30 )
				ZZ_ageOfUnitHDB = 30;

			if( ageOfUnitHDB  < 0 )
				ZZ_ageOfUnitHDB = 0;

			ZZ_ageOfUnitHDB = ZZ_ageOfUnitHDB / 10.0;

			if( ageOfUnitHDB > 30 )
				ZZ_ageGreater30 = 1;

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
				return 0;
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
					PredayPersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( headOfHousehold->getId(), homeTaz, workTaz );
					ZZ_logsumhh = personParam.getDpbLogsum();

					BigSerial groupId = hitssample->getGroupId();
					boost::shared_ptr<HM_Model::HouseholdGroup> thisHHGroup(new HM_Model::HouseholdGroup(groupId, homeTaz, ZZ_logsumhh ));

					model->householdGroupVec.push_back( *(thisHHGroup.get()));

					printHouseholdGroupLogsum( homeTaz, hitssample->getGroupId(), headOfHousehold->getId(), ZZ_logsumhh );
				}

				Household* householdT = const_cast<Household*>(household);
				householdT->setLogsum(ZZ_logsumhh);

			}

			const HM_Model::TazStats *tazstats  = model->getTazStatsByUnitId(unit->getId());

			if( tazstats->getChinesePercentage() > 0.76 ) //chetan TODO: add to xml file
				ZZ_hhchinese = 1;

			if( tazstats->getMalayPercentage() > 0.10 )
				ZZ_hhmalay 	 = 1;

			double ZZ_highInc = household->getIncome();
			double ZZ_middleInc = household->getIncome();
			double ZZ_lowInc  =  household->getIncome();

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

			int ZZ_children = 0;

			if( household->getChildUnder15() > 0 )
				ZZ_children = 1;

			int chineseHousehold = 0;
			int malayHousehold   = 0;

			if( household->getEthnicityId() == 1 )
				chineseHousehold = 1;

			if( household->getEthnicityId() == 2 )
				malayHousehold = 1;



			double Vpriv = 	(barea		*  DD_area 		) +
							(blogsum	* ZZ_logsumhh 	) +
							(bchin	  	* ZZ_hhchinese 	* chineseHousehold ) +
							(bmalay		* ZZ_hhmalay 	* malayHousehold   ) +
							(bHighInc   * ZZ_highInc 	) +
							(bHIncChildApart * ZZ_children * ZZ_highInc	* Apartment 	) +
							(bHIncChildCondo * ZZ_children * ZZ_highInc	* Condo 		) +
							(bapartment  * Apartment ) +
							(bcondo 	 * Condo 	 ) +
							(bdetachedAndSemiDetached * DetachedAndSemidetaced ) +
							(terrace	* Terrace		) +
							(bageOfUnit25 * ZZ_ageOfUnitPrivate 	) +
							(bageOfUnit25Squared 	* ZZ_ageOfUnitPrivate * ZZ_ageOfUnitPrivate ) +
							(bageGreaterT25LessT50  * ZZ_ageBet25And50 	) +
							(bageGreaterT50  		* ZZ_ageGreater50 	) +
							(bmissingAge  			* ZZ_missingAge 	) +
							(bfreeholdAppartm  		* ZZ_freehold * Apartment 	) +
							(bfreeholdCondo  		* ZZ_freehold * Condo 		) +
							(fbreeholdTerrace  		* ZZ_freehold * Terrace 	);


			double Vhdb = 	(barea		*  DD_area 		) +
							(blogsum	* ZZ_logsumhh 	) +
							(bchin	  	* ZZ_hhchinese 	* chineseHousehold ) +
							(bmalay		* ZZ_hhmalay 	* malayHousehold   ) +
							(bHighInc   * ZZ_highInc 	) +
							(midIncChildHDB3 * ZZ_children * ZZ_middleInc 	* HDB3	) +
							(midIncChildHDB4 * ZZ_children * ZZ_middleInc 	* HDB4	) +
							(midIncChildHDB5 * ZZ_children * ZZ_middleInc 	* HDB5	) +
							(bhdb12  * HDB12 ) +
							(bhdb3   * HDB3  ) +
							(bhdb4 	 * HDB4	 ) +
							(bhdb5 	 * HDB5	 ) +
							(bageOfUnit30 * ZZ_ageOfUnitHDB ) +
							(bageOfUnit30Squared * ZZ_ageOfUnitHDB * ZZ_ageOfUnitHDB ) +
							(bageOfUnitGreater30 * ZZ_ageGreater30 );

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
