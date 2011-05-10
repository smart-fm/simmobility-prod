#include "data_mgr.hpp"
#include "base.hpp"

namespace mit_sim
{

/* static */ DataManager * DataManager::instance_ = 0;

void
DataManager::add (Base* data)
{
    datum_.push_back (data);
}

void
DataManager::flip()
{
    for (int i = 0; i < datum_.size(); ++i)
    {
        datum_[i]->flip();
    }
}

}
