#if 0
/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "LoadAgents.hpp"

#include "conf/Config.hpp"
#include "entities/Agent.hpp"

using std::map;
using std::set;
using std::list;
using std::vector;
using std::string;

using namespace sim_mob;


namespace {
///TODO: This code is old code from simpleconf.cpp; it is the only thing which was not immediately upgraded.
///      We need to make an "ExplicitSignal" class, and allow loading from conf. For now, though, we can rely on DB loading.
///NOTE: Do *NOT* delete this code, even though it's commented. ~Seth
/***********************************
bool LoadExplicitSignals()
{
	//Quick check.
	std::string signalKeyID = "signal";
	if (signalKeyID!="signal") {
		return false;
	}

	//Build the expression dynamically.
	TiXmlHandle handle(&document);
	TiXmlElement* node = handle.FirstChild("config").FirstChild(signalKeyID+"s").FirstChild(signalKeyID).ToElement();
	if (!node) {
		//Signals are optional
		return true;
	}

        StreetDirectory& streetDirectory = StreetDirectory::instance();

	//Loop through all agents of this type
	for (;node;node=node->NextSiblingElement()) {
            char const * xPosAttr = node->Attribute("xpos");
            char const * yPosAttr = node->Attribute("ypos");
            if (0 == xPosAttr || 0 == yPosAttr)
            {

                std::cerr << "signals must have 'xpos', and 'ypos' attributes in the config file." << std::endl;
                return false;
            }

            try
            {
                int xpos = boost::lexical_cast<int>(xPosAttr);
                int ypos = boost::lexical_cast<int>(yPosAttr);

                const Point2D pt(xpos, ypos);
                Node* road_node = ConfigParams::GetInstance().getNetwork().locateNode(pt, true);
                if (0 == road_node)
                {
                    std::cerr << "xpos=\"" << xPosAttr << "\" and ypos=\"" << yPosAttr
                              << "\" are not suitable attributes for Signal because there is no node there; correct the config file."
                              << std::endl;
                    continue;
                }

                // See the comments in createSignals() in geospatial/aimsun/Loader.cpp.
                // At some point in the future, this function loadXMLSignals() will be removed
                // in its entirety, not just the following code fragment.
                std::set<Link const *> links;
                if (MultiNode const * multi_node = dynamic_cast<MultiNode const *>(road_node))
                {
                    std::set<RoadSegment*> const & roads = multi_node->getRoadSegments();
                    std::set<RoadSegment*>::const_iterator iter;
                    for (iter = roads.begin(); iter != roads.end(); ++iter)
                    {
                        RoadSegment const * road = *iter;
                        links.insert(road->getLink());
                    }
                }
                if (links.size() != 4)
                {
                    std::cerr << "the multi-node at " << pt << " does not have 4 links; "
                              << "no signal will be created here." << std::endl;
                    continue;
                }
                Signal const * signal = streetDirectory.signalAt(*road_node);
                if (signal)
                {
                    std::cout << "signal at node(" << xpos << ", " << ypos << ") already exists; "
                              << "skipping this config file entry" << std::endl;
                }
                else
                {
                	std::cout << "signal at node(" << xpos << ", " << ypos << ") was not found; No more action will be taken\n ";
//                    // The following call will create and register the signal with the
//                    // street-directory.
//                	std::cout << "register signal again!" << std::endl;
//                    Signal::signalAt(*road_node, ConfigParams::GetInstance().mutexStategy);
                }
            }
            catch (boost::bad_lexical_cast &)
            {
            	std::cout << "catch the loop error try!" << std::endl;
                std::cerr << "signals must have 'id', 'xpos', and 'ypos' attributes with numerical values in the config file." << std::endl;
                return false;
            }
	}
	return true;
}
***************************************/
} //End un-named namespace



sim_mob::LoadAgents::LoadAgents(Config& cfg, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents) : cfg(cfg)
{
	//Ensure as we go that all Agents have unique IDs.
	//Note that this is only a mild guarantee, as Agents can override their own IDs in their
	// constructors (as BusController once did).
	AgentConstraints constraints;

	//TODO: The fact that this is used twice indicates that it needs a more permanent storage location
	//      (e.g., in "system"), rather than in "gen_props"
	constraints.startingAutoAgentID = cfg.system().startingAgentAutoID;

	//A list to hold all generated Agents.
	std::list<sim_mob::Agent*> res_agents;

	//Perform key tasks
	CreateAgentShells(constraints, res_agents);
	//TODO: BusControllers need to be initialized here (check pg. 40, simpleconf)
	DispatchOrPend(res_agents, active_agents, pending_agents);
}


void sim_mob::LoadAgents::CreateAgentShells(AgentConstraints& constraints, std::list<sim_mob::Agent*>& res_agents)
{
	for (list<AbstractAgentLoader*>::const_iterator it=cfg.simulation().agentsLoaders.begin(); it!=cfg.simulation().agentsLoaders.end(); it++) {
		(*it)->loadAgents(res_agents, constraints, cfg);
	}
}

void sim_mob::LoadAgents::DispatchOrPend(const std::list<sim_mob::Agent*>& res_agents, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents)
{
	//TODO: This *might* be better off in the WorkGroup class (e.g., the WorkGroup handles this kind of thing, even though
	//      historically we've done it in the config file).
	for (std::list<sim_mob::Agent*>::const_iterator it=res_agents.begin(); it!=res_agents.end(); it++) {
		//Copied from the old "add or stash"
		if ((*it)->getStartTime()==0) {
			(*it)->load((*it)->getConfigProperties());
			(*it)->clearConfigProperties();
			active_agents.push_back(*it);
		} else {
			//Start later.
			pending_agents.push(*it);
		}
	}
}





#endif
