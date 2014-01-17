//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

#include "event/args/EventArgs.hpp"


namespace sim_mob {
namespace event {

/**
 * The event arguments for EVT_CORE_COMMSIM_REROUTING_REQUEST. Contains a Region to blacklist when re-routing.
 */
class ReRouteEventArgs : public sim_mob::event::EventArgs {
public:
	/// \param blacklistRegion The ID of the Region to blacklist. No Segments in this Region will be included in the
	///        new Route, if possible.
	ReRouteEventArgs(const std::string& blacklistRegion);
	virtual ~ReRouteEventArgs();

	std::string getBlacklistRegion() const;

private:
	std::string blacklistRegion;
};

}}

