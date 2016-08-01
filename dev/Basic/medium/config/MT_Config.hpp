//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <string>
#include <map>
#include <vector>
#include <utility>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Connection.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "util/ProtectedCopyable.hpp"

namespace sim_mob
{
namespace medium
{
class ParseMidTermConfigFile;

class MongoCollectionsMap
{
public:
	MongoCollectionsMap(const std::string& dbName = "");

	std::string getCollectionName(std::string key) const
	{
		//at() is used intentionally so that an out_of_range exception is triggered when invalid key is passed
		return collectionNameMap.at(key);
	}

	const std::map<std::string, std::string>& getCollectionsMap() const
	{
		return collectionNameMap;
	}

	void addCollectionName(const std::string& key, const std::string& value)
	{
		this->collectionNameMap[key] = value;
	}

	const std::string& getDbName() const
	{
		return dbName;
	}

private:
	std::string dbName;
	std::map<std::string, std::string> collectionNameMap; //key=>value
};

class PredayCalibrationParams
{
public:
	PredayCalibrationParams();

	double getAlgorithmCoefficient1() const
	{
		return algorithmCoefficient1;
	}

	void setAlgorithmCoefficient1(double algorithmCoefficient1)
	{
		this->algorithmCoefficient1 = algorithmCoefficient1;
	}

	double getAlgorithmCoefficient2() const
	{
		return algorithmCoefficient2;
	}

	void setAlgorithmCoefficient2(double algorithmCoefficient2)
	{
		this->algorithmCoefficient2 = algorithmCoefficient2;
	}

	const std::string& getCalibrationVariablesFile() const
	{
		return calibrationVariablesFile;
	}

	void setCalibrationVariablesFile(const std::string& calibrationVariablesFile)
	{
		this->calibrationVariablesFile = calibrationVariablesFile;
	}

	double getInitialGradientStepSize() const
	{
		return initialGradientStepSize;
	}

	void setInitialGradientStepSize(double initialGradientStepSize)
	{
		this->initialGradientStepSize = initialGradientStepSize;
	}

	double getInitialStepSize() const
	{
		return initialStepSize;
	}

	void setInitialStepSize(double initialStepSize)
	{
		this->initialStepSize = initialStepSize;
	}

	unsigned getIterationLimit() const
	{
		return iterationLimit;
	}

	void setIterationLimit(unsigned iterationLimit)
	{
		this->iterationLimit = iterationLimit;
	}

	double getStabilityConstant() const
	{
		return stabilityConstant;
	}

	void setStabilityConstant(double stabilityConstant)
	{
		this->stabilityConstant = stabilityConstant;
	}

	double getTolerance() const
	{
		return tolerance;
	}

	void setTolerance(double tolerance)
	{
		this->tolerance = tolerance;
	}

	const std::string& getObservedStatisticsFile() const
	{
		return observedStatisticsFile;
	}

	void setObservedStatisticsFile(const std::string& observedStatisticsFile)
	{
		this->observedStatisticsFile = observedStatisticsFile;
	}

	const std::string& getWeightMatrixFile() const
	{
		return weightMatrixFile;
	}

	void setWeightMatrixFile(const std::string& weightMatrixFile)
	{
		this->weightMatrixFile = weightMatrixFile;
	}

private:
	/**path and file name of csv containing variables to calibrate*/
	std::string calibrationVariablesFile;
	unsigned iterationLimit;
	double tolerance;
	double initialGradientStepSize;
	double algorithmCoefficient2;
	double initialStepSize;
	double stabilityConstant;
	double algorithmCoefficient1;
	std::string observedStatisticsFile;
	std::string weightMatrixFile;
};

/**
 * Represents the "Workers" section of the config file.
 */
class WorkerParams
{
public:
	struct WorkerConf
	{
		WorkerConf() : count(0), granularityMs(0) {}
		unsigned int count;
		unsigned int granularityMs;
	};

	WorkerConf person;
};

struct DB_Details
{
	DB_Details() :
			database(std::string()), credentials(std::string())
	{
	}
	std::string database;
	std::string credentials;
};

/**
 * Represents the loop-detector_counts section of the configuration file
 */
struct ScreenLineParams
{
    ScreenLineParams() : interval(0), outputEnabled(false), fileName("") {}

    ///The frequency of aggregating the vehicle counts at the loop detector
    unsigned int interval;

    ///Indicates whether the counts have to be output to a file
    bool outputEnabled;

    ///Name of the output file
    std::string fileName;
};

/**
 * represent the incident data section of the config file
 */
struct IncidentParams {
    IncidentParams() : incidentId(-1), visibilityDistance(0), segmentId(-1), position(0), severity(0),
            capFactor(0), startTime(0), duration(0), length(0),	compliance(0), accessibility(0){}

    struct LaneParams {
        LaneParams() : laneId(0), speedLimit(0), xLaneStartPos(0), yLaneStartPos(0), xLaneEndPos(0),yLaneEndPos(0){}
        unsigned int laneId;
        float speedLimit;
        float xLaneStartPos;
        float yLaneStartPos;
        float xLaneEndPos;
        float yLaneEndPos;
    };

    unsigned int incidentId;
    float visibilityDistance;
    unsigned int segmentId;
    float position;
    unsigned int severity;
    float capFactor;
    unsigned int startTime;
    unsigned int duration;
    float length;
    float compliance;
    float accessibility;
    std::vector<LaneParams> laneParams;
};

/**
 * simple struct to hold speed density parameters
 *
 * \author Harish Loganathan
 */
struct SpeedDensityParams
{
public:
	double getAlpha() const
	{
		return alpha;
	}

	double getBeta() const
	{
		return beta;
	}

	double getJamDensity() const
	{
		return jamDensity;
	}

	double getMinDensity() const
	{
		return minDensity;
	}

	void setAlpha(double alpha)
	{
		this->alpha = alpha;
	}

	void setBeta(double beta)
	{
		this->beta = beta;
	}

	void setJamDensity(double jamDensity)
	{
		this->jamDensity = jamDensity;
	}

	void setMinDensity(double minDensity)
	{
		this->minDensity = minDensity;
	}

private:
	/** outer exponent in speed density function */
	double alpha;
	/** inner exponent in speed density function */
	double beta;
	/** upper limit of density beyond which the speed of the moving part will be min speed */
	double jamDensity;
	/** lower limit of density below which the speed of the moving part will be free flow speed */
	double minDensity;
};

/**
 * Singleton class to hold Mid-term related configurations
 */

class MT_Config : private ProtectedCopyable
{
	friend class ParseMidTermConfigFile;
public:
	/**
	 * Destructor
	 */
	virtual ~MT_Config();

	/**
	 * Retrieves the singleton instance of MT_Config
	 *
	 * @return reference to the singleton instance
	 */
	static MT_Config& getInstance();

	/**
	 * Retrived pedestrian walk speed
	 *
	 * @return pedestrian walk speed
	 */
	double getPedestrianWalkSpeed() const;

	/**
	 * Retrieves dwell time parameters
	 *
	 * @return dwell time params
	 */
	std::vector<float>& getDwellTimeParams();

	/**
	 * Sets pedestrian walk speed
	 *
	 * @param pedestrianWalkSpeed speed to be set
	 */
	void setPedestrianWalkSpeed(double pedestrianWalkSpeed);

	/**
	 * Retrieves number of threads allocated for Preday
	 *
	 * @return number of threads
	 */
	unsigned getNumPredayThreads() const;

	/**
	 * Sets number of threads allocated for preday
	 *
	 * @param numPredayThreads number of threads
	 */
	void setNumPredayThreads(unsigned numPredayThreads);

	/**
	 * Retrieves model scripts map
	 *
	 * @return model scritps map
	 */
	const ModelScriptsMap& getModelScriptsMap() const;

	/**
	 * Sets model scripts map
	 *
	 * @param modelScriptsMap model scripts map to be set
	 */
	void setModelScriptsMap(const ModelScriptsMap& modelScriptsMap);


	void setServiceControllerScriptsMap(const ModelScriptsMap& modelScriptsMap);

	const ModelScriptsMap& getServiceControllerScriptsMap();

    /**
     * Retrieves Mongo Collection map
     *
     * @return mongo collections map
     */
	const MongoCollectionsMap& getMongoCollectionsMap() const;

	/**
	 * Sets mongo collection map
	 *
	 * @param mongoCollectionsMap mongo collections map to be set
	 */
	void setMongoCollectionsMap(const MongoCollectionsMap& mongoCollectionsMap);

	/**
	 * the object of this class gets sealed when this function is called. No more changes will be allowed via the setters
	 */
	void sealConfig();

	/**
	 * Retrieves Preday calibration params
	 *
	 * @return preday calibration params
	 */
	const PredayCalibrationParams& getPredayCalibrationParams() const;

	/**
	 * Retrieves SPSA Calibration params
	 *
	 * @return SPSA Calibration params
	 */
	const PredayCalibrationParams& getSPSA_CalibrationParams() const;

	/**
	 * Sets SPSA Calibration params
	 *
	 * @param spsaCalibrationParams SPSA calibration params to be set
	 */
	void setSPSA_CalibrationParams(const PredayCalibrationParams& spsaCalibrationParams);

	/**
	 * Retrieves WSPSA Calibration params
	 *
	 * @return WSPSA Calibration params
	 */
	const PredayCalibrationParams& getWSPSA_CalibrationParams() const;

	/**
	 * Sets WSPSA Calibration params
	 *
	 * @param wspsaCalibrationParams WSPSA Calibration params to be set
	 */
	void setWSPSA_CalibrationParams(const PredayCalibrationParams& wspsaCalibrationParams);

	/**
	 * Checks whether output to file is enabled
	 *
	 * @return true if enabled, else false
	 */
	bool isFileOutputEnabled() const;

	/**
	 * Sets output file enabled/disabled status
	 *
	 * @param outputTripchains status to be set
	 */
	void setFileOutputEnabled(bool outputTripchains);

	/**
	 * Checks whether output predictions is enabled
	 *
	 * @return true if enabled, else false
	 */
	bool isOutputPredictions() const;

	/**
	 * Sets output predictions enabled/disabled status
	 *
	 * @param outputPredictions status to be set
	 */
	void setOutputPredictions(bool outputPredictions);

	/**
	 * Checks whether console output enabled/disabled status
	 *
	 * @return true if enabled, else false
	 */
	bool isConsoleOutput() const;

	/**
	 * Sets console output enabled/disabled status
	 *
	 * @param consoleOutput status to be set
	 */
	void setConsoleOutput(bool consoleOutput);

	/**
	 * Checks whether preday simulation is running
	 *
	 * @return true if preday simulation is running, else false
	 */
	bool runningPredaySimulation() const;

	/**
	 * Checks whether preday calibration is running
	 *
	 * @return true if preday calibration is running, else false
	 */
	bool runningPredayCalibration() const;

	/**
	 * Checks whether preday logsum computation is running
	 *
	 * @return true if preday logsum computation is running, else false
	 */
	bool runningPredayLogsumComputation() const;

	/**
	 * Checks whether preday logsum computation for LT is running
	 *
	 * @return true if preday logsum computation for LT is running, else false
	 */
	bool runningPredayLogsumComputationForLT() const;

	/**
	 * Sets the preday run mode
	 *
	 * @param runMode run mode to be set
	 */
	void setPredayRunMode(const std::string runMode);

	/**
	 * Checks whether SPSA is running
	 *
	 * @return true if SPSA is running, else false
	 */
	bool runningSPSA() const;

	/**
	 * Checks whether WSPSA is running
	 *
	 * @return true if WSPSA is running
	 */
	bool runningWSPSA() const;

	/**
	 * Sets calibration methodology
	 *
	 * @param calibrationMethod calibration method to be set
	 */
	void setCalibrationMethodology(const std::string calibrationMethod);

	/**
	 * Retrieves calibration output file name
	 *
	 * @return calibration output file name
	 */
	const std::string& getCalibrationOutputFile() const;

	/**
	 * Sets the calibration output file name
	 *
	 * @param calibrationOutputFile filname to be set
	 */
	void setCalibrationOutputFile(const std::string& calibrationOutputFile);

	/**
	 * Retrieves logsum computation frequency
	 *
	 * @return logsum computation frequency
	 */
	unsigned getLogsumComputationFrequency() const;

	/**
	 * Sets logsum computation frequency
	 *
	 * @param logsumComputationFrequency logsum computation frequency to be set
	 */
	void setLogsumComputationFrequency(unsigned logsumComputationFrequency);

	/**
	 * Retrieves activity schedule loading interval
	 *
	 * @return activity schedule load interval
	 */
	unsigned getActivityScheduleLoadInterval() const;

	/**
	 * Sets activity schedule loading interval
	 *
	 * @param activityScheduleLoadInterval interval to be set
	 */
	void setActivityScheduleLoadInterval(unsigned activityScheduleLoadInterval);

	/**
	 * Retrieves supply update interval
	 *
	 * @return supply update interval
	 */
	unsigned getSupplyUpdateInterval() const;

	/**
	 * Sets supply update interval
	 *
	 * @param supplyUpdateInterval interval to be set
	 */
	void setSupplyUpdateInterval(unsigned supplyUpdateInterval);

	/**
	 * Retrieves bus capacity
	 *
	 * @return bus capacity
	 */
	const unsigned int getBusCapacity() const;

	/**
	 * Sets bus capacity
	 *
	 * @param busCapcacity bus capacity to be set
	 */
	void setBusCapacity(const unsigned int busCapcacity);

	/**
	 * Retrieves population source database type
	 *
	 * @return population source database
	 */
	db::BackendType getPopulationSource() const;

	/**
	 * Sets population source database
	 *
	 * @param src population source database to be set
	 */
	void setPopulationSource(const std::string& src);

	/**
	 * Retrieves simmobility database details
	 *
	 * @return simmobility db details
	 */
	const DB_Details& getSimmobDb() const;

	/**
	 * Sets logsum database details
	 *
	 * @param simmobDb simmobility database name
	 * @param simmobCred credentials for simmob database
	 */
	void setSimmobDb(const std::string& simmobDb, const std::string& simmobCred);

	/**
	 * Retrieves population database details
	 *
	 * @return population db details
	 */
	const DB_Details& getPopulationDb() const;

	/**
	 * Sets population database details
	 *
	 * @param populationDb population database name
	 * @param populationCred credentials for population database
	 */
	void setPopulationDb(const std::string& populationDb, const std::string& populationCred);


    /**
     * Retrieves number of workers for handling agents
     *
     * @return number of workers
     */
    unsigned int& personWorkGroupSize();

    /**
     * Retrieves number of workers for handling agents
     *
     * @return number of workers
     */
    unsigned int personWorkGroupSize() const;

    /**
     * Checks whether CBD area restriction enforced
     *
     * @return true if restriction enforced, else false
     */
    bool isRegionRestrictionEnabled() const;

    void setPublicTransitEnabled(bool val);

    /**
     * Retrives the confluxes
     *
     * @return confluxes
     */
    std::set<Conflux*>& getConfluxes();

    /**
     * Retrives the confluxes
     *
     * @return confluxes (const reference)
     */
    const std::set<Conflux*>& getConfluxes() const;

    /**
     * Retrieves conflux nodes
     *
     * @return conflux nodes
     */
    std::map<const Node*, Conflux*>& getConfluxNodes();

    /**
     * Retrieves conflux nodes
     *
     * @return conflux nodes (const reference)
     */
    const std::map<const Node*, Conflux*>& getConfluxNodes() const;

    /**
     * Retrives the conflux corresponding to a node
     *
     * @param multinode node for which the conflux to be found
     *
     * @return conflux
     */
    Conflux* getConfluxForNode(const Node* multinode) const;

    /**
     * Retrives the segment stats with bus stops
     *
     * @return segment stats with bus stops
     */
    std::set<SegmentStats*>& getSegmentStatsWithBusStops();

    /**
     * Checks whether mid term supply is running
     *
     * @return true if mid term supply is running, else false
     */
    bool RunningMidSupply() const;

    /**
     * Checks whether mid term demand is running
     *
     * @return true if mid term demand is running, else false
     */
    bool RunningMidDemand() const;

    /**
     * Sets the mid term run mode
     *
     * @param runMode run mode (supply/demand/withinday) to be set
     */
    void setMidTermRunMode(const std::string& runMode);

    /**
     * Retrives the incident params list
     *
     * @return incidents
     */
    std::vector<IncidentParams>& getIncidents();

    /**
     * Retrieve the disruption params
     * @return disruption definition
     */
    std::vector<DisruptionParams>& getDisruption();

    /**
     * get person timestep in milliseconds
     * @return timestep in milliseconds
     */
    unsigned int personTimeStepInMilliSeconds() const;


	const WorkerParams& getWorkerParams() const;

	/**
	 * returns speed density params for a given link category
	 * @param linkCategory link category (1=A, 2=B, 3=C, 4=D, 5=E, 6=SLIPROAD, 7=ROUNDABOUT)
	 * @return SpeedDensityParams for the given link category
	 */
	SpeedDensityParams getSpeedDensityParam(int linkCategory) const;

	/**
	 * sets speed density params for a given link category
	 * @param linkCategory link category (1=A, 2=B, 3=C, 4=D, 5=E, 6=SLIPROAD, 7=ROUNDABOUT)
	 * @param sdParams speed density parameters for the link category
	 */
	void setSpeedDensityParam(int linkCategory, SpeedDensityParams sdParams);


	/**
	 * get name of table storing logsums
	 * @return name of table storing logsums
	 */
	const std::string& getLogsumTableName() const;

	/**
	 * sets name of table containing logsums
	 * @param name of table storing logsums
	 */
	void setLogsumTableName(const std::string& logsumTableName);

	/**
	 * Enumerator for mid term run mode
	 */
	enum MidTermRunMode
	{
		MT_NONE, MT_SUPPLY, MT_PREDAY
	};

	/// Mid term run mode identifier
	MidTermRunMode midTermRunMode;

	/// screen line counts parameter
	ScreenLineParams screenLineParams;

	/// Number of ticks to wait before updating all Person agents.
	unsigned int granPersonTicks;

	/// Generic properties, for testing new features.
	std::map<std::string, std::string> genericProps;

private:
	/**
	 * Constructor
	 */
	MT_Config();

	/// Singleton instance
	static MT_Config* instance;

	/// protection for changes after config is loaded
	bool configSealed;

	/// store parameters for dwelling time calculation
	std::vector<float> dwellTimeParams;

	/// store parameters for pedestrian walking speed
	double pedestrianWalkSpeed;

	/**
	 * control variable for running preday simulation/logsum computation
	 */
	enum PredayRunMode
	{
		PREDAY_NONE, PREDAY_SIMULATION, PREDAY_CALIBRATION, PREDAY_LOGSUM_COMPUTATION, PREDAY_LOGSUM_COMPUTATION_LT
	};
	PredayRunMode predayRunMode;

	/// num of threads to run for preday
	unsigned numPredayThreads;

	/// flag to indicate whether output files need to be enabled
	bool fileOutputEnabled;

	/// flag to indicate whether tours and stops need to be output in mongodb
	bool outputPredictions;

	/// flag to indicate whether console output is required
	bool consoleOutput;

	/// Container for lua scripts
	ModelScriptsMap modelScriptsMap;


	ModelScriptsMap ServiceControllerScriptsMap;

	/// container for mongo collections
	MongoCollectionsMap mongoCollectionsMap;

	/** default capacity for bus*/
	unsigned int busCapacity;

	/// supply update interval in frames
	unsigned supplyUpdateInterval;

	/// activity schedule loading interval in seconds
	unsigned activityScheduleLoadInterval;

	/// population database type
	db::BackendType populationSource;

	/// poulation database details
	DB_Details populationDB;

	/// logsum database details
	DB_Details simmobDB;

	/// name of table containing pre-computed values
	std::string logsumTableName;

	/// worker allocation details
	WorkerParams workers;

	/**
	 * Enumerator for calibration methodology
	 */
	enum CalibrationMethodology
	{
		SPSA, WSPSA
	};

	/// Calibration methodology identifier
	CalibrationMethodology calibrationMethodology;

	/// SPSA Calibration params
	PredayCalibrationParams spsaCalibrationParams;

	/// WSPSA Calibration params
	PredayCalibrationParams wspsaCalibrationParams;

	/// Calibration output file name
	std::string calibrationOutputFile;

	/// Logsum computation frequency
	unsigned logsumComputationFrequency;

	///	is CBD area restriction enforced
	bool regionRestrictionEnabled;

	///setting for the incidents
	std::vector<IncidentParams> incidents;


    ///setting for disruptions
    std::vector<DisruptionParams> disruptions;

    /// set of confluxes
    std::set<Conflux*> confluxes;


	/// key:value (MultiNode:Conflux) map
	std::map<const Node*, Conflux*> multinode_confluxes;

	/// set of segment stats with bus stops
	std::set<SegmentStats*> segmentStatsWithBusStops;

	/// array of speed density function parameters indexed by link category
	SpeedDensityParams speedDensityParams[7];
};
}
}

