/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *         Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on October 21, 2013, 3:08 PM
 */
#pragma once
#include "Model.hpp"
#include "database/entity/Household.hpp"
#include "database/entity/Unit.hpp"
#include "database/entity/Individual.hpp"
#include "database/entity/Awakening.hpp"
#include "database/entity/VehicleOwnershipCoefficients.hpp"
#include "database/entity/TaxiAccessCoefficients.hpp"
#include "database/entity/HousingInterestRate.hpp"
#include "database/entity/Postcode.hpp"
#include "database/entity/Establishment.hpp"
#include "database/entity/Job.hpp"
#include "database/entity/LogSumVehicleOwnership.hpp"
#include "database/entity/DistanceMRT.hpp"
#include "database/entity/Taz.hpp"
#include "database/entity/HouseHoldHitsSample.hpp"
#include "database/entity/TazLogsumWeight.hpp"
#include "database/entity/LogsumMtzV2.hpp"
#include "database/entity/PlanningArea.hpp"
#include "database/entity/PlanningSubzone.hpp"
#include "database/entity/Mtz.hpp"
#include "database/entity/MtzTaz.hpp"
#include "database/entity/Alternative.hpp"
#include "database/entity/Hits2008ScreeningProb.hpp"
#include "database/entity/ZonalLanduseVariableValues.hpp"
#include "database/entity/PopulationPerPlanningArea.hpp"
#include "database/entity/HitsIndividualLogsum.hpp"
#include "database/entity/UnitSale.hpp"
#include "database/entity/VehicleOwnershipChanges.hpp"
#include "database/entity/IndvidualVehicleOwnershipLogsum.hpp"
#include "database/entity/AccessibilityFixedPzid.hpp"
#include "database/entity/ScreeningCostTime.hpp"
#include "database/entity/TenureTransitionRate.hpp"
#include "database/entity/OwnerTenantMovingRate.hpp"
#include "database/entity/HouseholdPlanningArea.hpp"
#include "database/entity/SchoolAssignmentCoefficients.hpp"
#include "database/entity/PrimarySchool.hpp"
#include "database/entity/PreSchool.hpp"
#include "database/entity/HHCoordinates.hpp"
#include "database/entity/AlternativeHedonicPrice.hpp"
#include "database/entity/ScreeningModelCoefficients.hpp"
#include "database/entity/HouseholdUnit.hpp"
#include "database/entity/IndvidualEmpSec.hpp"
#include "database/entity/LtVersion.hpp"
#include "core/HousingMarket.hpp"
#include "boost/unordered_map.hpp"
#include "DeveloperModel.hpp"

namespace sim_mob
{
    namespace long_term
    {
        /**
         * Class that contains Housing market model logic.
         */
        class HM_Model : public Model
        {
        public:
            typedef std::vector<Unit*> UnitList;
            typedef boost::unordered_map<BigSerial, Unit*> UnitMap;

            typedef std::vector<Household*> HouseholdList;
            typedef boost::unordered_map<BigSerial, Household*> HouseholdMap;

            typedef std::vector<Individual*> IndividualList;
            typedef boost::unordered_map<BigSerial, Individual*> IndividualMap;

            typedef std::vector<Postcode*> PostcodeList;
            typedef boost::unordered_map<BigSerial, Postcode*> PostcodeMap;

            typedef std::vector<Awakening*> AwakeningList;
            typedef boost::unordered_map<BigSerial, Awakening*> AwakeningMap;

            typedef std::vector<VehicleOwnershipCoefficients*> VehicleOwnershipCoeffList;
            typedef boost::unordered_map<BigSerial, VehicleOwnershipCoefficients*> VehicleOwnershipCoeffMap;

            typedef std::vector<TaxiAccessCoefficients*> TaxiAccessCoeffList;
            typedef boost::unordered_map<BigSerial, TaxiAccessCoefficients*> TaxiAccessCoeffMap;

            typedef std::vector<Establishment*> EstablishmentList;
            typedef boost::unordered_map<BigSerial, Establishment*> EstablishmentMap;

            typedef std::vector<Job*> JobList;
            typedef boost::unordered_map<BigSerial, Job*> JobMap;

            typedef std::vector<Taz*> TazList;
            typedef boost::unordered_map<BigSerial, Taz*> TazMap;

            typedef std::vector<HousingInterestRate*> HousingInterestRateList;
            typedef boost::unordered_map<BigSerial, HousingInterestRate*> HousingInterestRateMap;

            typedef std::vector<LogSumVehicleOwnership*> VehicleOwnershipLogsumList;
            typedef boost::unordered_map<BigSerial, LogSumVehicleOwnership*> VehicleOwnershipLogsumMap;

            typedef std::vector<DistanceMRT*> DistMRTList;
            typedef boost::unordered_map<BigSerial, DistanceMRT*> DistMRTMap;

            typedef std::vector<HouseHoldHitsSample*> HouseHoldHitsSampleList;
            typedef boost::unordered_map<BigSerial, HouseHoldHitsSample*> HouseHoldHitsSampleMap;

            typedef std::vector<TazLogsumWeight*> TazLogsumWeightList;
            typedef boost::unordered_map<BigSerial, TazLogsumWeight*> TazLogsumWeightMap;

            typedef std::vector<LogsumMtzV2*> LogsumMtzV2List;
            typedef boost::unordered_map<BigSerial, LogsumMtzV2*> LogsumMtzV2Map;
            //
            typedef std::vector<PlanningArea*> PlanningAreaList;
            typedef boost::unordered_map<BigSerial, PlanningArea*> PlanningAreaMap;

            typedef std::vector<PlanningSubzone*> PlanningSubzoneList;
            typedef boost::unordered_map<BigSerial, PlanningSubzone*> PlanningSubzoneMap;

            typedef std::vector<Mtz*> MtzList;
            typedef boost::unordered_map<BigSerial, Mtz*> MtzMap;

            typedef std::vector<MtzTaz*> MtzTazList;
            typedef boost::unordered_map<BigSerial, MtzTaz*> MtzTazMap;

            typedef std::vector<Alternative*> AlternativeList;
            typedef boost::unordered_map<BigSerial, Alternative*> AlternativeMap;

            typedef std::vector<Hits2008ScreeningProb*> Hits2008ScreeningProbList;
            typedef boost::unordered_map<BigSerial, Hits2008ScreeningProb*> Hits2008ScreeningProbMap;

            typedef std::vector<ZonalLanduseVariableValues*> ZonalLanduseVariableValuesList;
            typedef boost::unordered_map<BigSerial, ZonalLanduseVariableValues*> ZonalLanduseVariableValuesMap;

            typedef std::vector<PopulationPerPlanningArea*> PopulationPerPlanningAreaList;
            typedef boost::unordered_map<BigSerial, PopulationPerPlanningArea*> PopulationPerPlanningAreaMap;

            typedef std::vector<HitsIndividualLogsum*> HitsIndividualLogsumList;
            typedef boost::unordered_map<BigSerial, HitsIndividualLogsum*> HitsIndividualLogsumMap;


            typedef std::vector<IndvidualVehicleOwnershipLogsum*> IndvidualVehicleOwnershipLogsumList;
            typedef boost::unordered_map<BigSerial, IndvidualVehicleOwnershipLogsum*> IndvidualVehicleOwnershipLogsumMap;

            typedef std::vector<AccessibilityFixedPzid*> AccessibilityFixedPzidList;
            typedef boost::unordered_map<BigSerial, AccessibilityFixedPzid*> AccessibilityFixedPzidMap;

            typedef std::vector<ScreeningCostTime*> ScreeningCostTimeList;
            typedef boost::unordered_map<BigSerial, ScreeningCostTime*> ScreeningCostTimeMap;
            typedef boost::unordered_map<std::string, BigSerial> ScreeningCostTimeSuperMap;

            typedef std::vector<TenureTransitionRate*> TenureTransitionRateList;
            typedef boost::unordered_map<BigSerial, TenureTransitionRate*> TenureTransitionRateMap;

            typedef std::vector<OwnerTenantMovingRate*>OwnerTenantMovingRateList;
            typedef boost::unordered_map<BigSerial, OwnerTenantMovingRate*> OwnerTenantMovingRateMap;

            typedef std::vector<AlternativeHedonicPrice*>AlternativeHedonicPriceList;
            typedef boost::unordered_multimap<BigSerial, AlternativeHedonicPrice*> AlternativeHedonicPriceMap;

            typedef std::vector<ScreeningModelCoefficients*>ScreeningModelCoefficientsList;
            typedef boost::unordered_map<BigSerial, ScreeningModelCoefficients*> ScreeningModelCoefficicientsMap;

            typedef std::vector<VehicleOwnershipChanges*> VehicleOwnershipChangesList;
            typedef boost::unordered_map<BigSerial, VehicleOwnershipChanges*> VehicleOwnershipChangesMap;


            typedef std::vector<HouseholdPlanningArea*> HouseholdPlanningAreaList;
            typedef boost::unordered_map<BigSerial, HouseholdPlanningArea*> HouseholdPlanningAreaMap;

            typedef std::vector<SchoolAssignmentCoefficients*> SchoolAssignmentCoefficientsList;
            typedef boost::unordered_map<BigSerial, SchoolAssignmentCoefficients*> SchoolAssignmentCoefficientsMap;

            typedef std::vector<PrimarySchool*> PrimarySchoolList;
            typedef boost::unordered_map<BigSerial, PrimarySchool*> PrimarySchoolMap;

            typedef std::vector<HHCoordinates*> HHCoordinatesList;
            typedef boost::unordered_map<BigSerial, HHCoordinates*> HHCoordinatesMap;

            typedef std::vector<PreSchool*> PreSchoolList;
            typedef boost::unordered_map<BigSerial, PreSchool*> PreSchoolMap;

            typedef std::vector<HouseholdUnit*> HouseholdUnitList;
            typedef boost::unordered_map<BigSerial, HouseholdUnit*> HouseholdUnitMap;

            typedef std::vector<IndvidualEmpSec*> IndvidualEmpSecList;
            typedef boost::unordered_map<BigSerial, IndvidualEmpSec*> IndvidualEmpSecMap;

            typedef std::vector<LtVersion*> LtVersionList;
            typedef boost::unordered_map<BigSerial, LtVersion*> LtVersionMap;

            /**
             * Taz statistics
             */
            class TazStats
            {
            public:
                TazStats(BigSerial tazId = INVALID_ID);
                virtual ~TazStats();
                
                /**
                 * Getters 
                 */
                BigSerial getTazId() const;
                long int getHH_Num() const;
                int getIndividuals() const;
                double getHH_TotalIncome() const;
                double getHH_AvgIncome() const;

                double getChinesePercentage() const;
                double getMalayPercentage() const;
                double getIndianPercentage() const;
                double getAvgHHSize() const;



            private:
                friend class HM_Model;
                void updateStats(const Household& household);

                BigSerial tazId;
                long int hhNum;
                int individuals;
                double hhTotalIncome;

                long int householdSize;
                long int numChinese;
                long int numMalay;
                long int numIndian;
            };
            
            typedef boost::unordered_map<BigSerial, HM_Model::TazStats*> StatsMap;
            

            /*
             *This function will contain groups of households who share the same logsum if they have the same hometaz
            */
            class HouseholdGroup
            {
            public:

            	HouseholdGroup(BigSerial groupId = 0, BigSerial homeTaz = 0, double logsum = .0);
            	~HouseholdGroup(){};

            	HouseholdGroup( HouseholdGroup& source);
            	HouseholdGroup(const HouseholdGroup& source);
            	HouseholdGroup& operator=(const HouseholdGroup& source);
            	HouseholdGroup& operator=( HouseholdGroup& source);


            	void	setLogsum(double value);
            	void	setGroupId(BigSerial value);
            	void	setHomeTaz( BigSerial value);

            	double	  getLogsum() const;
            	BigSerial getGroupId() const;
            	BigSerial getHomeTaz() const;

            private:

            	double logsum;
            	BigSerial homeTaz;
            	BigSerial groupId;
            };

            std::vector<HouseholdGroup> householdGroupVec;
            boost::unordered_map<BigSerial, HouseholdGroup*> vehicleOwnerhipHHGroupByGroupId;

            HM_Model(WorkGroup& workGroup);
            virtual ~HM_Model();
            
            /**
             * Getters & Setters 
             */
            const Unit* getUnitById(BigSerial id) const;
            BigSerial getUnitTazId(BigSerial unitId) const;
            BigSerial getEstablishmentTazId(BigSerial establishmentId) const;
            const TazStats* getTazStats(BigSerial tazId) const;
            const TazStats* getTazStatsByUnitId(BigSerial unitId) const;


            Household* getHouseholdById( BigSerial id) const;
            Household* getHouseholdWithBidsById( BigSerial id) const;
			Individual* getIndividualById( BigSerial id) const;
			Individual* getPrimaySchoolIndById(BigSerial id) const;
			Individual* getPreSchoolIndById(BigSerial id) const;
            Awakening* getAwakeningById( BigSerial id) const;
            Postcode* getPostcodeById(BigSerial id) const;
            Job* getJobById(BigSerial id) const;
            Establishment* getEstablishmentById( BigSerial id) const;

            void hdbEligibilityTest(int );
            void unitsFiltering();
            void incrementAwakeningCounter();
            int  getAwakeningCounter() const;
            void getLogsumOfIndividuals(BigSerial id);
            void getLogsumOfVaryingHomeOrWork(BigSerial id);
            void getLogsumOfHouseholdVO(BigSerial householdId);

            HousingMarket* getMarket();

            HouseholdList* getHouseholdList();

            void setDeveloperModel(DeveloperModel *developerModel);
            DeveloperModel* getDeveloperModel() const;


            HousingInterestRateList* getHousingInterestRateList();

            double ComputeHedonicPriceLogsumFromMidterm(BigSerial taz);
            double ComputeHedonicPriceLogsumFromDatabase(BigSerial taz) const;

            int getBids();
            int getExits();
            int getSuccessfulBids();

            void incrementBids();
            void incrementExits();
            void incrementSuccessfulBids();
            void resetBAEStatistics(); //BAE is Bids, Awakenings and Exits

            void setNumberOfBidders(int number);
			void setNumberOfSellers(int number);
			void setNumberOfBTOAwakenings(int number);
			void incrementNumberOfSellers();
			void incrementNumberOfBidders();
			void incrementNumberOfBTOAwakenings();
			int getNumberOfSellers();
			int getNumberOfBidders();
			int getNumberOfBTOAwakenings();

            void incrementLifestyle1HHs();
            void incrementLifestyle2HHs();
            void incrementLifestyle3HHs();
            int getLifestyle1HHs() const;
            int getLifestyle2HHs() const;
            int getLifestyle3HHs() const;
            VehicleOwnershipCoeffList getVehicleOwnershipCoeffs()const;
            VehicleOwnershipCoefficients* getVehicleOwnershipCoeffsById( BigSerial id) const;
            TaxiAccessCoeffList getTaxiAccessCoeffs()const;
            TaxiAccessCoefficients* getTaxiAccessCoeffsById( BigSerial id) const;
            Taz* getTazById( BigSerial id) const;
            void addUnit(Unit* unit);
            std::vector<BigSerial> getRealEstateAgentIds();
            VehicleOwnershipLogsumList getVehicleOwnershipLosums()const;
            LogSumVehicleOwnership* getVehicleOwnershipLogsumsById( BigSerial id) const;
            void setTaxiAccess2008(const Household *household);
            void setTaxiAccess2012(const Household *household);
            DistMRTList getDistanceMRT()const;
            DistanceMRT* getDistanceMRTById( BigSerial id) const;
            HouseHoldHitsSampleList getHouseHoldHits()const;
            HouseHoldHitsSample* getHouseHoldHitsById( BigSerial id) const;
            HouseholdGroup* getHouseholdGroupByGroupId(BigSerial id)const;
            void addHouseholdGroupByGroupId(HouseholdGroup* hhGroup);
            void getScreeningProbabilities(std::string hitsId, std::vector<double> &householdScreeningProbabilties );
            AlternativeList& getAlternatives();
            Alternative* getAlternativeById(int zoneHousingType);
            PlanningArea* getPlanningAreaById( int id );
            std::vector<PlanningSubzone*> getPlanningSubZoneByPlanningAreaId(int id);
            std::vector<Mtz*> getMtzBySubzoneVec(std::vector<PlanningSubzone*> vecPlanningSubzone );
            std::vector <BigSerial> getTazByMtzVec( std::vector<Mtz*> vecMtz);
            int getMtzIdByTazId(int tazId);
            Mtz* getMtzById( int id);
            PlanningSubzone* getPlanningSubzoneById(int id);
            ZonalLanduseVariableValues* getZonalLandUseByAlternativeId(int id) const;
            Alternative* getAlternativeByPlanningAreaId(int id) const;
            std::vector<PopulationPerPlanningArea*> getPopulationByPlanningAreaId(BigSerial id)const;
            HitsIndividualLogsumList getHitsIndividualLogsumVec() const;
            void setStartDay(int day);
            int getStartDay() const;
            void addNewBids(boost::shared_ptr<Bid> &newBid);
            void addHouseholdUnits(boost::shared_ptr<HouseholdUnit> &newHouseholdUnit);
            BigSerial getBidId();
            BigSerial getUnitSaleId();
            std::vector<boost::shared_ptr<Bid> > getNewBids();
            std::vector<boost::shared_ptr<HouseholdUnit> > getNewHouseholdUnits();
            void addUnitSales(boost::shared_ptr<UnitSale> &unitSale);
            std::vector<boost::shared_ptr<UnitSale> > getUnitSales();
            void addHouseholdsTo_OPSchema(boost::shared_ptr<Household> &houseHold);
            std::vector<boost::shared_ptr<Household> > getHouseholdsWithBids();
            void addVehicleOwnershipChanges(boost::shared_ptr<VehicleOwnershipChanges> &vehicleOwnershipChange);
            std::vector<boost::shared_ptr<VehicleOwnershipChanges> > getVehicleOwnershipChanges();
            ScreeningModelCoefficientsList getScreeningModelCoefficientsList();

            IndvidualVehicleOwnershipLogsumList getIndvidualVehicleOwnershipLogsums() const;
            IndvidualVehicleOwnershipLogsum* getIndvidualVehicleOwnershipLogsumsByHHId(BigSerial householdId) const;

            ScreeningCostTimeList getScreeningCostTime();
            ScreeningCostTime* getScreeningCostTimeInst(std::string key);
            AccessibilityFixedPzidList getAccessibilityFixedPzid();


            static const BigSerial FAKE_IDS_START = 9999900;

            std::multimap<BigSerial, Unit*> getUnitsByZoneHousingType();
            std::vector<Bid*> getResumptionBids();
            Household* getResumptionHouseholdById( BigSerial id) const;
            VehicleOwnershipChanges* getVehicleOwnershipChangesByHHId(BigSerial houseHoldId) const;
            void setLastStoppedDay(int stopDay);
            int getLastStoppedDay();
            HouseholdPlanningAreaList getHouseholdPlanningAreaList() const;
            HouseholdPlanningArea* getHouseholdPlanningAreaByHHId(BigSerial houseHoldId) const;
            SchoolAssignmentCoefficientsList getSchoolAssignmentCoefficientsList() const;
            SchoolAssignmentCoefficients* getSchoolAssignmentCoefficientsById( BigSerial id) const;
            PrimarySchoolList getPrimarySchoolList() const;
            PrimarySchool* getPrimarySchoolById( BigSerial id) const;
            HHCoordinatesList getHHCoordinatesList() const;
            HHCoordinates* getHHCoordinateByHHId(BigSerial houseHoldId) const;
            PreSchoolList getPreSchoolList() const;
            PreSchool* getPreSchoolById( BigSerial id) const;

            std::vector<OwnerTenantMovingRate*> getOwnerTenantMovingRates();
            std::vector<TenureTransitionRate*> getTenureTransitionRates();
            std::vector<AlternativeHedonicPrice*> getAlternativeHedonicPrice();
            boost::unordered_multimap<BigSerial, AlternativeHedonicPrice*>& getAlternativeHedonicPriceById();

            HouseholdUnit* getHouseholdUnitByHHId(BigSerial hhId) const;
            IndvidualEmpSecList getIndvidualEmpSecList() const;
            IndvidualEmpSec* getIndvidualEmpSecByIndId(BigSerial indId) const;

			vector<double> getlogSqrtFloorAreahdb() const{ return logSqrtFloorAreahdb;}
            vector<double> getlogSqrtFloorAreacondo() const { return logSqrtFloorAreacondo;}


            set<string> logsumUniqueCounter;

            void  loadLTVersion(DB_Connection &conn);


        protected:
            /**
             * Inherited from Model.
             */
            void startImpl();
            void stopImpl();
            void update(int day);

        private:
            // Data
            HousingMarket market;

            HouseholdList households;
            HouseholdMap householdsById;
            HouseholdMap householdWithBidsById;

            UnitList units; //residential only.
            UnitMap unitsById;
            std::multimap<BigSerial, Unit*> unitsByZoneHousingType;

            StatsMap stats;

            IndividualList individuals;
            IndividualMap individualsById;
            IndividualList primarySchoolIndList;
            IndividualList preSchoolIndList;
            IndividualMap primarySchoolIndById;
            IndividualMap preSchoolIndById;

            PostcodeList postcodes;
            PostcodeMap postcodesById;

            EstablishmentList establishments;
            EstablishmentMap establishmentsById;

            HousingInterestRateList housingInterestRates;
            HousingInterestRateMap housingInterestRatesById;

            JobList jobs;
            JobMap jobsById;

            TazList tazs;
            TazMap tazById;

            TazLogsumWeightList tazLogsumWeights;
            TazLogsumWeightMap tazLogsumWeightById;

            LogsumMtzV2List logsumMtzV2;
            LogsumMtzV2Map logsumMtzV2ById;

            PlanningAreaList planningArea;
            PlanningAreaMap  planningAreaById;

            PlanningSubzoneList planningSubzone;
            PlanningSubzoneMap planningSubzoneById;

            MtzList mtz;
            MtzMap mtzById;

            MtzTazList mtzTaz;
            MtzTazMap mtzTazById;

            Hits2008ScreeningProbList hits2008ScreeningProb;
            Hits2008ScreeningProbMap hits2008ScreeningProbById;
            AlternativeList alternative;
            AlternativeMap alternativeById;

            PopulationPerPlanningAreaList populationPerPlanningArea;
            PopulationPerPlanningAreaMap populationPerPlanningAreaById;

            HitsIndividualLogsumList hitsIndividualLogsum;
            HitsIndividualLogsumMap  hitsIndividualLogsumById;

            AccessibilityFixedPzidList accessibilityFixedPzid;
            AccessibilityFixedPzidMap accessibilityFixedPzidById;

            ScreeningCostTimeList screeningCostTime;
            ScreeningCostTimeMap screeningCostTimeById;
            ScreeningCostTimeSuperMap screeningCostTimeSuperMap;

            boost::mutex mtx;
            boost::mutex mtx2;
            boost::mutex mtx3;
            boost::mutex mtx4;
            boost::mutex idLock;
            boost::mutex DBLock;
            boost::unordered_map<BigSerial, double>tazLevelLogsum;
            boost::unordered_map<BigSerial, double>vehicleOwnershipLogsum;

            vector<double> logSqrtFloorAreahdb;
            vector<double> logSqrtFloorAreacondo;

            boost::unordered_map<BigSerial, BigSerial> assignedUnits;
            VehicleOwnershipCoeffList vehicleOwnershipCoeffs;
            VehicleOwnershipCoeffMap vehicleOwnershipCoeffsById;
            TaxiAccessCoeffList taxiAccessCoeffs;
            TaxiAccessCoeffMap taxiAccessCoeffsById;
            AwakeningList awakening;
            AwakeningMap awakeningById;
            HouseholdStatistics household_stats;
            VehicleOwnershipLogsumList vehicleOwnershipLogsums;
            VehicleOwnershipLogsumMap vehicleOwnershipLogsumById;
            DistMRTList mrtDistances;
            DistMRTMap mrtDistancesById;
            HouseHoldHitsSampleList houseHoldHits;
            HouseHoldHitsSampleMap houseHoldHitsById;

            ZonalLanduseVariableValuesList zonalLanduseVariableValues;
            ZonalLanduseVariableValuesMap zonalLanduseVariableValuesById;

            TenureTransitionRateList tenureTransitionRate;
            TenureTransitionRateMap tenureTransitionRateById;
            OwnerTenantMovingRateList ownerTenantMovingRate;
            OwnerTenantMovingRateMap ownerTenantMovingRateById;

            int	initialHHAwakeningCounter;
            int numberOfBidders;
            int numberOfSellers;
            int numLifestyle1HHs;
            int numLifestyle2HHs;
            int numLifestyle3HHs;
            std::vector<BigSerial> realEstateAgentIds;
            bool hasTaxiAccess;
            int householdLogsumCounter;
            int simulationStopCounter;

            int numberOfBids;
            int numberOfExits;
            int numberOfSuccessfulBids;
            int numberOfBTOAwakenings;

            DeveloperModel *developerModel;
            int startDay; //start tick of the simulation
            std::vector<boost::shared_ptr<Bid> > newBids;
            std::vector<boost::shared_ptr<UnitSale> > unitSales;
            std::vector<boost::shared_ptr<HouseholdUnit> > newHouseholdUnits;
            BigSerial bidId;
            BigSerial unitSaleId;
            std::vector<boost::shared_ptr<Household> > hhWithBidsVector;
            std::vector<boost::shared_ptr<VehicleOwnershipChanges> > vehicleOwnershipChangesVector;
            IndvidualVehicleOwnershipLogsumList IndvidualVehicleOwnershipLogsums;
            IndvidualVehicleOwnershipLogsumMap IndvidualVehicleOwnershipLogsumById;

            AlternativeHedonicPriceList alternativeHedonicPrice;
            AlternativeHedonicPriceMap alternativeHedonicPriceById;

            ScreeningModelCoefficientsList screeningModelCoefficientsList;
            ScreeningModelCoefficicientsMap screeningModelCoefficicientsMap;

            std::vector<SimulationStoppedPoint*> simStoppedPointList;
            std::vector<Bid*> resumptionBids;
            HouseholdList resumptionHouseholds;
            HouseholdMap resumptionHHById;
            VehicleOwnershipChangesList vehOwnershipChangesList;
            VehicleOwnershipChangesMap vehicleOwnershipChangesById;
            HouseholdUnitList householdUnits;
            HouseholdUnitMap householdUnitByHHId;
            int lastStoppedDay;
            HouseholdPlanningAreaList hhPlanningAreaList;
            HouseholdPlanningAreaMap hhPlanningAreaMap;
            SchoolAssignmentCoefficientsList schoolAssignmentCoefficients;
            SchoolAssignmentCoefficientsMap SchoolAssignmentCoefficientsById;
            PrimarySchoolList primarySchools;
            PrimarySchoolMap primarySchoolById;
            HHCoordinatesList hhCoordinates;
            HHCoordinatesMap hhCoordinatesById;
            PreSchoolList preSchools;
            PreSchoolMap preSchoolById;
            bool resume ;
            IndvidualEmpSecList indEmpSecList;
            IndvidualEmpSecMap indEmpSecbyIndId;

            LtVersionList ltVersionList;
            LtVersionMap ltVersionById;



        };
    }
}

