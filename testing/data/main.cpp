#include <position.hpp>
#include <integer.hpp>
#include <trace.hpp>
#include <data_mgr.hpp>

int
main()
{
    mit_sim::Position pos (3, 5);
    mit_sim::Integer count (8);
    traceln ("pos=" << pos << ", count=" << count);

    pos.increment_x (1);
    count.set (count.get() + 1);
    traceln ("pos=" << pos << ", count=" << count);

    mit_sim::DataManager::singleton().flip();
    traceln ("pos=" << pos << ", count=" << count);

    return 0;
}
