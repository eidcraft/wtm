
#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
using std::vector;
using std::string;
using std::cout;
using std::fstream;


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
using boost::property_tree::ptree;
using boost::property_tree::read_json;

#include "Resources.h"
#include "NetworkCreator.h"

class WtmJsonLoader
{
public:

  static Network* loadNetwork(string& filename);
  static WtmSourceDataSet* loadWtmSourceDataSet(string& filename);

  static NetworkParameters* loadParameters(fstream& fileStream);
  static vector<WtmSourceData*>* loadMotorBasisData(fstream& fileStream);
  static vector<WtmSourceData*>* loadBasicBehaviorData(fstream& fileStream);

  static void saveNetwork(string& sourceFile, Network& network);

  static ptree packInputSignals(vector<InputSignal*>* sources);
  static ptree packNeurons(vector<Neuron*>* neurons);
  static ptree packOutNeurons(vector<Neuron*>* neurons);
  static ptree packParameters(NetworkParameters* parameters);

private:
  static ptree* loadJsonFile(fstream& fileStream);
  
  static NetworkParameters* parseParameters(ptree* propertyTree);
  static void parseNumberParameters(ptree& propertyTree, NetworkParameters* parameters);
  static void parseSigmaParameters(ptree& propertyTree, NetworkParameters* parameters);


  static vector<InputSignal*>* parseInputSignals(ptree& propertyTree, 
    Network* network);
  static vector<Neuron*>* parseNeurons(ptree& propertyTree, 
    Network* network);
  static vector<Neuron*>* parseOutNeurons(ptree& propertyTree,
    Network* network);
  static vector<WtmSourceData*>* parseMotorBasisSources(ptree* propertyTree);
  
  static vector<WtmSourceData*>* parseBasicBehaviorSources(ptree* propertyTree);


  static ptree* saveSigma(Sigma* sigma);

  static void appendToFile(std::ifstream& fileStream, ptree& propertyTree);


};

