#include "stdafx.h"
#include "NetworkJsonLoader.h"



Network* NetworkJsonLoader::createNetworkFromJson(string filename)
{
  std::ifstream ifStream(filename);
  boost::property_tree::ptree propertyTree;
  boost::property_tree::read_json(ifStream, propertyTree);

  //Network* network = new Network();

  //loadParameters(propertyTree, network);
  //loadSigmas(propertyTree, network);
  //loadInputSignals(propertyTree, network);
  //loadNeurons(propertyTree, network);

  //for (size_t i = 0; i < network->parameters->networkPeriod / 2 + 1; i++)
  //{
  //  network->synapticSignalsQueues.push_back(*(new vector<SynapticSignal>()));
  //}

  return nullptr;
}


void
NetworkJsonLoader::loadParameters( ptree& propertyTree, Network* network)
{
  NetworkParameters* parameters = network->parameters;

  parameters->networkPeriod = propertyTree.get<int>("networkPeriod");
  parameters->layersNumber = propertyTree.get<int>("layersNumber");
  parameters->activationThreshold = propertyTree.get<float>("activationThreshold");
  parameters->fatigueThreshold = propertyTree.get<float>("fatigueThreshold");
  parameters->inputNeuronsNumber = propertyTree.get<int>("inputNeuronsNumber");
  parameters->outputNeuronNumber = propertyTree.get<int>("outputNeuronNumber");

  //return parameters;
}


void NetworkJsonLoader::loadSigmas(
  ptree& propertyTree, Network* network)
{
  float initialYScale;
  float initialXOffset;
  float initialExpCoeff;
  float endYScale;
  float endXOffset;
  float endExpCoeff;
  bool isIncreasing;
  string title;

  BOOST_FOREACH(ptree::value_type sigmaObj, propertyTree.get_child("sigmas"))
  {
    initialYScale = sigmaObj.second.get<float>("initialYScale");
    initialXOffset = sigmaObj.second.get<float>("initialXOffset");
    initialExpCoeff = sigmaObj.second.get<float>("initialExpCoeff");
    endYScale = sigmaObj.second.get<float>("endYScale");
    endXOffset = sigmaObj.second.get<float>("endXOffset");
    endExpCoeff = sigmaObj.second.get<float>("endExpCoeff");
    isIncreasing = sigmaObj.second.get<bool>("isIncreasing");

    Sigma* sigma = new Sigma(initialYScale, initialXOffset, initialExpCoeff,
      endYScale, endXOffset, endExpCoeff,
      network->parameters->layersNumber, isIncreasing, title);


    title = sigmaObj.second.get<string>("title");
    if (title == string("thresholdIncreaseFunc"))
      network->parameters->thresholdIncreaseFunc = sigma;
    else if (title == string("thresholdDecreaseFunc"))
      network->parameters->thresholdDecreaseFunc = sigma;
    else if (title == string("fatigueIncreaseFunc"))
      network->parameters->fatigueIncreaseFunc = sigma;
    else if (title == string("fatigueDecreaseFunc"))
      network->parameters->fatigueDecreaseFunc = sigma;
    else if (title == string("weightIncreaseFunc"))
      network->parameters->weightIncreaseFunc = sigma;
    else if (title == string("weightDecreaseFunc"))
      network->parameters->weightDecreaseFunc = sigma;
  }
}


void NetworkJsonLoader::loadInputSignals(
  ptree& propertyTree, Network* network)
{
  InputSignal* signal;

  int id;
  vector<OutcomingSynapse*>* outcomingSynapses = new
    vector<OutcomingSynapse*>();

  OutcomingSynapse* outcomingSynapsePtr;

  BOOST_FOREACH(ptree::value_type signalObj,
    propertyTree.get_child("inputSignals"))
  {
    id = signalObj.second.get<int>("id");

    BOOST_FOREACH(ptree::value_type outcomingSynapse,
      signalObj.second.get_child("outcomingSynapses"))
    {
      outcomingSynapsePtr = new OutcomingSynapse();

      outcomingSynapsePtr->destNeuronId =
        outcomingSynapse.second.get<int>("destNeuronId");
      outcomingSynapsePtr->distanceToDest =
        outcomingSynapse.second.get<int>("distanceToDest");

      outcomingSynapses->push_back(outcomingSynapsePtr);
    }

    signal = new InputSignal(id, network, outcomingSynapses);
    network->inputSignals->push_back(signal);
    outcomingSynapses->clear();
  }

}


void NetworkJsonLoader::loadNeurons(
  ptree& propertyTree, Network* network)
{
  Neuron* neuron;

  int id;
  int networkLayer;

  vector<IncomingSynapse*>* straightSynapses = new 
    vector<IncomingSynapse*>();
  vector<OutcomingSynapse*>* outcomingSynapses = new 
    vector<OutcomingSynapse*>();

  IncomingSynapse* incomingSynapsePtr;
  OutcomingSynapse* outcomingSynapsePtr;

  BOOST_FOREACH(ptree::value_type neuronObj,
    propertyTree.get_child("neurons"))
  {
    id = neuronObj.second.get<int>("id");
    networkLayer = neuronObj.second.get<int>("networkLayer");

    BOOST_FOREACH(ptree::value_type incomingSynapse,
      neuronObj.second.get_child("straightSynapses"))
    {
      incomingSynapsePtr = new IncomingSynapse();

      incomingSynapsePtr->sourceNeuronId =
        incomingSynapse.second.get<int>("sourceNeuronId");
      incomingSynapsePtr->weight =
        incomingSynapse.second.get<float>("weight");

      straightSynapses->push_back(incomingSynapsePtr);
    }

    BOOST_FOREACH(ptree::value_type outcomingSynapse,
      neuronObj.second.get_child("outcomingSynapses"))
    {
      outcomingSynapsePtr = new OutcomingSynapse();

      outcomingSynapsePtr->destNeuronId =
        outcomingSynapse.second.get<int>("destNeuronId");
      outcomingSynapsePtr->distanceToDest =
        outcomingSynapse.second.get<int>("distanceToDest");

      outcomingSynapses->push_back(outcomingSynapsePtr);
    }

    neuron = new Neuron(id, networkLayer, network,
      straightSynapses, outcomingSynapses);
    network->neuronsInRow->push_back(neuron);
  }
  network->splitNeuronsToLayers();
}
