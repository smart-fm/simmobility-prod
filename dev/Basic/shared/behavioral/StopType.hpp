//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{
/**
 * Fundamental stop (activity) types supported by preday.
 *
 * \author Harish Loganathan
 */
enum StopType
{
	WORK=1,
	EDUCATION,
	SHOP,
	OTHER,
	WORK_BASED_SUBTOUR,
	NULL_STOP
};
}
