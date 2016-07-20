
#pragma once

#include "Network.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;

class NetworkJsonLoader
{
public:
  static Network* createNetworkFromJson(std::string filename);

private:
  NetworkJsonLoader() {}
  static void loadParameters(ptree& propertyTree, Network* network);
  static void loadSigmas(ptree& propertyTree, Network* network);
  static void loadInputSignals(ptree& propertyTree, Network* network);
  static void loadNeurons(ptree& propertyTree, Network* network);
};
