#include "geo10-pimpl.hpp"
#include "entities/signal/Signal.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Signal_t_pimpl::pre ()
{
	  signalHelper.clearSignalHelper();
	  model = signalHelper.getBasicSignal();//this is what will be finally returned
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
	signalHelper.setBasicSignal(new sim_mob::Signal(*book.getNode(value),sim_mob::ConfigParams::GetInstance().mutexStategy,signalHelper.getSignalID()));

}

void sim_mob::xml::Signal_t_pimpl::phases (sim_mob::Signal::phases value)
{
	signalHelper.getBasicSignal()->phases_ = value;
}

void sim_mob::xml::Signal_t_pimpl::linkAndCrossings (sim_mob::LinkAndCrossingC value)
{
	signalHelper.getBasicSignal()->LinkAndCrossings_ = value;
}
void sim_mob::xml::Signal_t_pimpl::SCATS (sim_mob::xml::helper::SignalHelper::SCATS_Info SCATS_Info_)
{
	signalHelper.setTargetSignal(new sim_mob::Signal_SCATS(signalHelper.getBasicSignal()->getNode(),sim_mob::ConfigParams::GetInstance().mutexStategy,signalHelper.getSignalID(),sim_mob::SIG_SCATS));
	signalHelper.getTargetSignal()->phases_ = signalHelper.getBasicSignal()->phases_;
	signalHelper.getTargetSignal()->LinkAndCrossings_ = signalHelper.getBasicSignal()->LinkAndCrossings_;
	  sim_mob::Signal_SCATS *temp = dynamic_cast<sim_mob::Signal_SCATS *>(signalHelper.getTargetSignal());
	  temp->setSignalTimingMode(SCATS_Info_.signalTimingMode);
	  temp->plan_ = SCATS_Info_.SplitPlan;
	  temp->plan_.setParentSignal(temp);
	  temp->initialize();
	  delete signalHelper.getBasicSignal();
}
//void sim_mob::xml::Signal_t_pimpl::SplitPlan (sim_mob::SplitPlan value)
//{
//	signalHelper.getBasicSignal()->set = value;
//}


