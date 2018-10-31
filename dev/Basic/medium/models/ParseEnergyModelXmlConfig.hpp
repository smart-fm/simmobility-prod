#pragma  once

#include <boost/noncopyable.hpp>
#include <string>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

#include "conf/ParseConfigXmlBase.hpp"
#include "conf/RawConfigParams.hpp"
#include "models/EnergyModelBase.hpp"

using namespace sim_mob;
using namespace xercesc;

namespace sim_mob
{
namespace medium
{

class ParseEnergyModelXmlConfig : public ParseConfigXmlBase, private boost::noncopyable
{
	EnergyModelBase* energyModel;
	void ProcessEnergyModelNode(xercesc::DOMElement* node);
public:
	///Parse a config file into RawConfigParams, performing all XML parsing and some trivial semantic processing.
	ParseEnergyModelXmlConfig(const std::string& configFileName, EnergyModelBase* energyModel);
	/**
	 * pure virtual function to override in derived classes.
	 * code for processing the respective xml must be written in this function in the derived classes
	 * @param parser reference to parser after parsing the xml
	 */
	void processXmlFile(xercesc::XercesDOMParser& parser) ;

private:
	/**
	 * process energy model params node in config
	 * @param node corresponding to energy model type inside xml file
	 */
	void processEMParamsNode(xercesc::DOMElement* node);

};

} // namespace medium
} // namespace sim_mob
