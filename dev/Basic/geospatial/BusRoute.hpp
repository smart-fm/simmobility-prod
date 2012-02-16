#include "BusStop.hpp"
#include "Lane.hpp"





namespace sim_mob{
  class Lane;
  class BusStop;
  class Route;



  /**
   * \author Skyler Seto
   * \author Seth N. Hetu
   */
  //I am disabling this temporarily; see vehicle/BusRoute.hpp ~Seth
	/*class Busroute: public Route {
	public:
		struct Way_point {
			enum { LANE, BUS_STOP } type;
			union {
				Lane* lane;
				BusStop* bus_stop;
			};
			time_t expected_time;
		};

		std::vector<Way_point*> Bus_route;

	};*/
	
}
