/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "TestAgent.hpp"

#include <algorithm>

#include "workers/Worker.hpp"
#include "entities/UpdateParams.hpp"
#include "conf/simpleconf.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif
//
using std::map;
using std::string;
using std::vector;
using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;

namespace sim_mob {
    using std::string;
    using std::endl;
    using std::cout;

    namespace long_term {

        TestAgent::TestAgent(EventDispatcher* dispatcher, Role* role, const MutexStrategy& mtxStrat, int id) :
        Agent(mtxStrat, id), currRole(role), params(NULL), dispatcher(dispatcher) {
            RegisterEvent(1);
            RegisterEvent(2);
            RegisterEvent(3);
            RegisterEvent(4);
            dispatcher->RegisterPublisher(this);
        }

        TestAgent::~TestAgent() {
            dispatcher->UnRegisterPublisher(this);
            safe_delete_item(currRole);
            params = NULL; // only leave the pointer
        }

        void TestAgent::load(const map<string, string>& configProps) {
        }

        bool TestAgent::frame_init(timeslice now) {
            //Failsafe: no Role at all?
            if (!currRole) {
                cout << "TestAgent: " << this->getId() << " has no Role.";
                throw std::runtime_error("Agent without Role.");
            }
            params = &currRole->make_frame_tick_params(now);
            currRole->frame_init(*params);
            dispatcher->Dispatch(this, 1, EventArgs());
            //Notify(1, EventArgs());
            return true;
        }

        Entity::UpdateStatus TestAgent::frame_tick(timeslice now) {
            if (!params) {
                params = &currRole->make_frame_tick_params(now);
            }
            dispatcher->Dispatch(this, 2, EventArgs());
            //Notify(2, EventArgs());
            Entity::UpdateStatus retVal(UpdateStatus::RS_CONTINUE);
            if (!isToBeRemoved()) {
                dispatcher->Dispatch(this, 3, EventArgs());
                //Notify(3, EventArgs());
                //Reset the start time (to the NEXT time tick) so our dispatcher doesn't complain.
                setStartTime(now.ms() + ConfigParams::GetInstance().baseGranMS);
                currRole->frame_tick(*params);
            }
            return retVal;
        }

        void TestAgent::frame_output(timeslice now) {
            dispatcher->Dispatch(this, 4, EventArgs());
            //Notify(4, EventArgs());
            //Save the output
            if (!isToBeRemoved()) {
                currRole->frame_tick_output(*params);
            }
        }

        void TestAgent::SetRole(Role* newRole) {
            this->currRole = newRole;
        }

    }
}

