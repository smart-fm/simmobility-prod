#include "ParseEnergyModelXmlConfig.hpp"
#include "util/XmlParseHelper.hpp"
#include "models/EnergyModelBase.hpp"
#include <xercesc/dom/DOM.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include "logging/Log.hpp"

using namespace xercesc;
using namespace std;
using namespace sim_mob::medium;

ParseEnergyModelXmlConfig::ParseEnergyModelXmlConfig(const std::string& configFileName, EnergyModelBase* energyModel) :  energyModel(energyModel), ParseConfigXmlBase(configFileName)
{
	if(configFileName.size() == 0)
	{
		throw std::runtime_error("Energy Model Params config file not specified");
	}
	parseXmlAndProcess();
}

void ParseEnergyModelXmlConfig::processXmlFile(XercesDOMParser& parser)
{
	//Verify that the root node is "energy_models"
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	if (TranscodeString(rootNode->getTagName()) != "energy_models") {
		throw std::runtime_error("xml parse error: root node of path set configuration file must be \"energy_models\"");
	}

	ProcessEnergyModelNode(rootNode);
}


void ParseEnergyModelXmlConfig::ProcessEnergyModelNode(xercesc::DOMElement* node){
  std::cout <<"Parsing energy model node\n";
	if (!node)
	{
		std::cerr << "Energy Model Params Configuration Not Found\n" ;
		return;
  }

  // check that config is enabled
	if(!(ParseBoolean(GetNamedAttributeValue(node, "enabled"), "false"))) { return; }
  
  ///Loop through models to check which model config to use.
  for (DOMElement* model=node->getFirstElementChild(); model; model=model->getNextElementSibling())
  {
      if (TranscodeString(model->getNodeName())!="model")
      {
        Warn() <<"CONFIG (Energy Model Params): Invalid model node " << TranscodeString(model->getNodeName()) << "\n";
          continue;
      }

      std::string modelType = ParseString(GetNamedAttributeValue(model, "type"), "");
      if (modelType.empty())
      {
          Warn() <<"CONFIG (Energy Model Params): Invalid model node; missing \"type\".\n";
          continue;
      }
     
      // if the model type found is equal to the energy model type then process node
      if (modelType == energyModel->getModelType())
      {
          processEMParamsNode(model);
          break;
      }
  }

}

void ParseEnergyModelXmlConfig::processEMParamsNode(xercesc::DOMElement* node)
{
    ///Loop through and save child attributes.
    for (DOMElement* param=node->getFirstElementChild(); param; param=param->getNextElementSibling())
    {
        if (TranscodeString(param->getNodeName())!="param")
        {
            Warn() <<"CONFIG (Energy Model Params): Invalid params node: " << TranscodeString(param->getNodeName()) << "\n";
            continue;
        }

        std::string key = ParseString(GetNamedAttributeValue(param, "name"), "");
        std::string val = ParseString(GetNamedAttributeValue(param, "value"), "");
        if (key.empty() || val.empty())
        {
            Warn() <<"CONFIG (Energy Model Params): Invalid param field... missing \"name\" or \"value\".\n";
            continue;
        }

        energyModel->setParams(key, val);
    }
    energyModel->setupParamsVariables();
}

