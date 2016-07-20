
#pragma once

#include <vector>
#include <string>
#include <iostream>
using std::vector;
using std::string;
using std::cout;

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
using boost::property_tree::ptree;
using boost::property_tree::read_json;

#include "WtmParameters.h"
#include "WtmCreator.h"

class WtmJsonLoader
{
public:

  static WtmNetwork* loadNetwork(std::string& filename);

  static WtmParameters* loadParameters(string& sourceFile);
  static vector<WtmSourceData*>* loadMotorBasisSources(string& sourceFile);


  static void saveInputSignals(string& sourceFile,
    vector<WtmNetwork::InputSignal*>* sources);
  static void saveNeurons(string& destFile,
    vector<WtmNetwork::Neuron*>& neurons);
  static void saveParameters(string& destFile,
    WtmParameters* parameters);

private:
  static ptree* loadJsonFile(string& filename);
  
  static WtmParameters* parseParameters(ptree* propertyTree);
  static void parseNumberParameters(ptree& propertyTree, WtmParameters* parameters);
  static void parseSigmaParameters(ptree& propertyTree, WtmParameters* parameters);

  static vector<WtmSourceData*>* parseMotorBasisSources(ptree* propertyTree);
  static vector<WtmSourceData*>* parseSourceData(ptree* propertyTree);

  static vector<WtmNetwork::InputSignal*>* parseInputSignals(ptree& propertyTree, 
    WtmNetwork* network);
  static vector<WtmNetwork::Neuron*>* parseNeurons(ptree& propertyTree, 
    WtmNetwork* network);

  static ptree* saveWtmSigma(WtmSigma* sigma);

  static void appendToFile(string& filename, ptree& propertyTree);


};

