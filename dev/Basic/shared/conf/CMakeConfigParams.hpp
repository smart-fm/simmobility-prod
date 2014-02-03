//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "util/ProtectedCopyable.hpp"
#include <string>


namespace sim_mob {


/**
 * Contains CMake-defined parameters.
 *
 * \note
 * Developers should make sure NOT to include "GenConfig.h" here; otherwise, this forces most of the source
 * tree to be rebuilt whenever a single parameter changes. (In other words, don't implement any functions in
 * this header file, except perhaps constructors.)
 *
 * \author Seth N. Hetu
 */
class CMakeConfigParams : private sim_mob::ProtectedCopyable {
public:
	//@{
	///Synced to the value of SIMMOB_DISABLE_MPI; used for runtime checks.
	bool MPI_Enabled() const;
	bool MPI_Disabled() const;
	//@}
	bool PathSetMode() const;
	//@{
	///Synced to the value of SIMMOB_DISABLE_OUTPUT; used for runtime checks.
	bool OutputEnabled() const;
	bool OutputDisabled() const;
	//@}

	///Synced to the value of SIMMOB_STRICT_AGENT_ERRORS; used for runtime checks.
	bool StrictAgentErrors() const;

	///Synced to the value of SIMMOB_PROFILE_ON; used for runtime checks.
	bool ProfileOn() const;

	///Synced to the value of SIMMOB_PROFILE_AGENT_UPDATES; used for runtime checks.
	///If "accountForOnFlag" is false, *only* the cmake define flag is checked.
	bool ProfileAgentUpdates(bool accountForOnFlag=true) const;

	///Synced to the value of SIMMOB_PROFILE_WORKER_UPDATES; used for runtime checks.
	///If "accountForOnFlag" is false, *only* the cmake define flag is checked.
	bool ProfileWorkerUpdates(bool accountForOnFlag=true) const;

	///Synced to the value of SIMMOB_PROFILE_AURAMGR; used for runtime checks.
	///If "accountForOnFlag" is false, *only* the cmake define flag is checked.
	bool ProfileAuraMgrUpdates(bool accountForOnFlag=true) const;

	///Synced to the value of SIMMOB_PROFILE_COMMSIM; used for runtime checks.
	///If "accountForOnFlag" is false, *only* the cmake define flag is checked.
	bool ProfileCommsimUpdates(bool accountForOnFlag=true) const;

	///Synced to the value of SIMMOB_USE_CONFLUXES; used for runtime checks.
	bool UsingConfluxes() const;

	///Synced to the value of SIMMOB_INTERACTIVE_MODE; used for to detect if we're running "interactively"
	/// with the GUI or console.
	bool InteractiveMode() const;

	///Synced to the value of SIMMOB_XML_WRITER; this should probably be a config-file flag.
	bool XmlWriterOn() const;


};


}

