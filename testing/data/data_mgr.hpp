#ifndef _data_mgr_hpp_
#define _data_mgr_hpp_

#include <vector>

namespace mit_sim
{

class Base;

class DataManager
{
public:
    static DataManager & singleton()
    {
        if (0 == instance_)
        {
            instance_ = new DataManager;
        }
        return *instance_;
    }

    void add (Base * data);

    void flip();

private:
    DataManager()
    {
    }

    static DataManager * instance_;
    std::vector<Base*> datum_;
};

}

#endif
