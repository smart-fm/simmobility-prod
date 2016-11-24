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

			/**
			 * This interface adds the trains to be removed to the "trainsToBeRemoved" vector
			 */
			void addToTrainRemovalList(TrainDriver *driver);

			/**
			 * This interface removes the trains before the next frame tick after all
			 * the workers have synchronized at the end of frame tick
			 */
			void removeTrainsBeforeNextFrameTick();
		private:
			/*list which maintains the trains to be removed */
			std::vector<TrainDriver*> trainsToBeRemoved;
			static TrainRemoval *instance;
			/* lock for trainsToBeRemoved vector */
			boost::mutex trainRemovalListLock;
	};
}
}

