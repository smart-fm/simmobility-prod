/*
 * TrainRemoval.h
 *
 *  Created on: 3 Sep 2016
 *      Author: jabir
 */
#include "roles/driver/TrainDriver.hpp"
#include "entities/TrainController.hpp"
class TrainDriver;
namespace sim_mob
{
	namespace medium
	{
		class TrainRemoval
		{
			public:
				TrainRemoval();
				virtual ~TrainRemoval();
				static TrainRemoval *getInstance();
				void addToTrainRemovalList(TrainDriver *driver);
				void removeTrainsBeforeNextFrameTick();
			private:
			    std::vector<TrainDriver*> trainsToBeRemoved;
				static TrainRemoval *instance;
				boost::mutex trainRemovalListLock;
		};
	}
}

