#pragma  once

#include <boost/noncopyable.hpp>
#include <string>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

#include "conf/ParseConfigXmlBase.hpp"
#include "conf/RawConfigParams.hpp"

using namespace sim_mob;
using namespace xercesc;

namespace sim_mob
{

class ParsePathXmlConfig : public ParseConfigXmlBase, private boost::noncopyable
{
	PathSetConf &cfg;
	void ProcessPathSetNode(xercesc::DOMElement* node);
public:
	///Parse a config file into RawConfigParams, performing all XML parsing and some trivial semantic processing.
	ParsePathXmlConfig(const std::string& configFileName, PathSetConf& result);
	/**
	 * pure virtual function to override in derived classes.
	 * code for processing the respective xml must be written in this function in the derived classes
	 * @param parser reference to parser after parsing the xml
	 */
	void processXmlFile(xercesc::XercesDOMParser& parser) ;

private:
	/**
	 * process public pathset node in config
	 * @param node node corresponding to public pathset element inside xml file
	 */
	void processPrivatePathsetNode(xercesc::DOMElement* node);

	/**
	 * process public pathset node in config
	 * @param node node corresponding to public pathset element inside xml file
	 */
	void processPublicPathsetNode(xercesc::DOMElement* node);
};

}
