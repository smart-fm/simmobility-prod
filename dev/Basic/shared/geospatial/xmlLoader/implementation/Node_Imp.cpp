#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Node_t_pimpl::pre ()
{
	model = sim_mob::Node(0, 0);
}

sim_mob::Node* sim_mob::xml::Node_t_pimpl::post_Node_t ()
{
	sim_mob::Node* res = new sim_mob::Node(model);

	//TODO: Not 100% sure what we are tracking here. ~Seth
/*
	  geo_LinkLoc_linkID & inserter = get<1>(geo_LinkLoc_);
	  if(this->linkLoc_)
	  {
		  geo_LinkLoc_linkID::iterator it = inserter.find(this->linkLoc_);
		  if(it == inserter.end())//if not available, insert it
		  {
			  geo_LinkLoc_mapping g(this->linkLoc_,node_);
			  inserter.insert(g);//node_ address in this container will later be replace by a uninode, intesection or roundabout adress
//			  std::cout << "An entry ["<< g.linkID <<"," << g.rawNode  << "] Inserted into multi index container size("<< inserter.size() << ")\n";
		  }
		  else//otherwise, update the calling node
		  {
			  geo_LinkLoc_mapping temp = *it;
			  const sim_mob::Node * t = it->rawNode;//for debugging only
			  temp.rawNode = node_;
			  inserter.replace(it,temp);
//			  std::cout << "basic raw node " << t << " updated by rawNode " <<  temp.rawNode << std::endl;//" .. container size("<< inserter.size() << ") "<< "["<< it->linkID <<"," << it->node1 << "," <<it->node2<< "," << it->rawNode  << "]\n";
		  }

	  }*/


	  return res;
}


void sim_mob::xml::Node_t_pimpl::nodeID (unsigned int value)
{
	model.nodeId = value;
}

void sim_mob::xml::Node_t_pimpl::location (sim_mob::Point2D value)
{
	model.location = value;
}

void sim_mob::xml::Node_t_pimpl::linkLoc (unsigned long long value)
{
	model.setLinkLoc(Links_pimpl::LookupLink(value));
}

void sim_mob::xml::Node_t_pimpl::originalDB_ID (const std::string& value)
{
	model.originalDB_ID = value;
}

