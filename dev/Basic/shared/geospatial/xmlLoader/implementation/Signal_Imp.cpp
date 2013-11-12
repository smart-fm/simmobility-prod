//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"
#include "entities/signal/Signal.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Signal_t_pimpl::pre ()
{
	  signalHelper.clearSignalHelper();
	  model = signalHelper.getTargetSignal();//this is what will be finally returned
}

sim_mob::Signal* sim_mob::xml::Signal_t_pimpl::post_Signal_t ()
{
	return model;
}

void sim_mob::xml::Signal_t_pimpl::signalID (unsigned int value)
{
	signalHelper.setSignalID(value);
}

void sim_mob::xml::Signal_t_pimpl::nodeID (unsigned int value)
{
	signalHelper.setBasicSignal(new sim_mob::Signal(*book.getNode(value),sim_mob::ConfigManager::GetInstance().FullConfig().mutexStategy(),signalHelper.getSignalID()));

}

void sim_mob::xml::Signal_t_pimpl::phases (sim_mob::Signal::phases& value)
{
	signalHelper.getBasicSignal()->phases_ = value;
}

void sim_mob::xml::Signal_t_pimpl::linkAndCrossings (sim_mob::LinkAndCrossingC& value)
{
	signalHelper.getBasicSignal()->LinkAndCrossings_ = value;
}
void sim_mob::xml::Signal_t_pimpl::SCATS (sim_mob::xml::helper::SignalHelper::SCATS_Info& SCATS_Info_)
{
	signalHelper.setTargetSignal(new sim_mob::Signal_SCATS(signalHelper.getBasicSignal()->getNode(),sim_mob::ConfigManager::GetInstance().FullConfig().mutexStategy(),signalHelper.getSignalID(),sim_mob::SIG_SCATS));
	signalHelper.getTargetSignal()->phases_ = signalHelper.getBasicSignal()->phases_;
	signalHelper.getTargetSignal()->LinkAndCrossings_ = signalHelper.getBasicSignal()->LinkAndCrossings_;
	sim_mob::Signal_SCATS *temp = dynamic_cast<sim_mob::Signal_SCATS *>(signalHelper.getTargetSignal());
	// temp->setSignalTimingMode(SCATS_Info_.signalTimingMode);
	temp->plan_ = SCATS_Info_.SplitPlan;
	temp->plan_.setParentSignal(temp);
	temp->initialize();
	model = signalHelper.getTargetSignal();//this is what will be finally returned
	delete signalHelper.getBasicSignal();
	//debugging. error checking the null link value
	sim_mob::LinkAndCrossingC& LAC = signalHelper.getTargetSignal()->LinkAndCrossings_;
	sim_mob::LinkAndCrossingC::iterator it = LAC.begin();
	for(;it != LAC.end(); it++){
		if(!it->link){
			std::ostringstream out("");
			out <<"signal " <<  signalHelper.getTargetSignal()->getNode().getID() << " has null link in LAC" << std::endl;
			throw std::runtime_error(out.str());
		}
	}
}
//void sim_mob::xml::Signal_t_pimpl::SplitPlan (sim_mob::SplitPlan value)
//{
//	signalHelper.getBasicSignal()->set = value;
//}


