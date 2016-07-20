#include "stdafx.h"
#include "WtmJsonLoader.h"




WtmParameters* WtmJsonLoader::loadParameters(string& filename)
{
  ptree* propertyTree = loadJsonFile(filename);
  return parseParameters(propertyTree);
}


ptree*  WtmJsonLoader::loadJsonFile(string& filename)
{
  std::ifstream ifStream(filename);

  if (!ifStream)
  {
    cout << "file not found" << std::endl;
    return nullptr;
  }
  ptree* propertyTree = new ptree();
  read_json(ifStream, *propertyTree);

  return propertyTree;
}


WtmParameters* WtmJsonLoader::parseParameters(ptree* propertyTree)
{
  WtmParameters* wtmParameters = new WtmParameters();

  ptree parametersObj = propertyTree->get_child("wtmParameters");
  
  // if (!parametersObj) out of there

  parseNumberParameters(parametersObj, wtmParameters);
  parseSigmaParameters(parametersObj, wtmParameters);

  return wtmParameters;
}


void WtmJsonLoader::parseNumberParameters(ptree& propertyTree, WtmParameters* parameters)
{
  parameters->networkPeriod = propertyTree.get<int>("networkPeriod");
  parameters->layersNumber = propertyTree.get<int>("layersNumber");
  parameters->activationThreshold = propertyTree.get<float>("activationThreshold");
  parameters->fatigueThreshold = propertyTree.get<float>("fatigueThreshold");
  parameters->inputNeuronsNumber = propertyTree.get<int>("inputNeuronsNumber");
  parameters->outputNeuronNumber = propertyTree.get<int>("outputNeuronNumber");
  parameters->uniqueNeuronsPerVNet = propertyTree.get<int>("uniqueNeuronsPerVNet"); 
  parameters->averageSynapseWeight = propertyTree.get<float>("averageSynapseWeight");
}


void WtmJsonLoader::parseSigmaParameters(ptree& propertyTree, WtmParameters* parameters)
{
  float initialYScale;
  float initialXOffset;
  float initialExpCoeff;
  float endYScale;
  float endXOffset;
  float endExpCoeff;
  bool isIncreasing;
  string title;

  WtmSigma* sigma;

  BOOST_FOREACH(ptree::value_type sigmaObj, propertyTree.get_child("sigmas"))
  {
    initialYScale = sigmaObj.second.get<float>("initialYScale");
    initialXOffset = sigmaObj.second.get<float>("initialXOffset");
    initialExpCoeff = sigmaObj.second.get<float>("initialExpCoeff");
    endYScale = sigmaObj.second.get<float>("endYScale");
    endXOffset = sigmaObj.second.get<float>("endXOffset");
    endExpCoeff = sigmaObj.second.get<float>("endExpCoeff");
    isIncreasing = sigmaObj.second.get<bool>("isIncreasing");
    title = sigmaObj.second.get<string>("title");

    sigma = new WtmSigma(initialYScale, initialXOffset, initialExpCoeff,
      endYScale, endXOffset, endExpCoeff, parameters->layersNumber, 
      isIncreasing, title);

    title = sigmaObj.second.get<string>("title");
    if (title == string("thresholdIncreaseFunc"))
      parameters->thresholdIncreaseFunc = sigma;
    else if (title == string("thresholdDecreaseFunc"))
      parameters->thresholdDecreaseFunc = sigma;
    else if (title == string("fatigueIncreaseFunc"))
      parameters->fatigueIncreaseFunc = sigma;
    else if (title == string("fatigueDecreaseFunc"))
      parameters->fatigueDecreaseFunc = sigma;
    else if (title == string("weightIncreaseFunc"))
      parameters->weightIncreaseFunc = sigma;
    else if (title == string("weightDecreaseFunc"))
      parameters->weightDecreaseFunc = sigma;
  }
}


vector<WtmSourceData*>* WtmJsonLoader::loadMotorBasisSources(string& filename)
{
  ptree* propertyTree = loadJsonFile(filename);
  return parseMotorBasisSources(propertyTree);
}


vector<WtmSourceData*>* WtmJsonLoader::parseMotorBasisSources(ptree* propertyTree)
{
  ptree sourcesObj = propertyTree->get_child("wtmMotorBasis");
  return parseSourceData(&sourcesObj);
}


vector<WtmSourceData*>* WtmJsonLoader::parseSourceData(ptree* propertyTree)
{
  vector<WtmSourceData*>* result = new vector<WtmSourceData*>();

  WtmSourceData* sourceDataUnit = nullptr;

  vector<bool>* inputDataVector;
  vector<bool>* outputDataVector;

  BOOST_FOREACH(ptree::value_type sourceDataJson,
    *propertyTree)
  {
    sourceDataUnit = new WtmSourceData();

    BOOST_FOREACH(ptree::value_type inputDataJson,
      sourceDataJson.second.get_child("inputData"))
    {
      inputDataVector = new vector<bool>();
      BOOST_FOREACH(ptree::value_type inputDataUnitJson,
        inputDataJson.second)
      {
        inputDataVector->push_back(inputDataUnitJson.second.get_value<bool>());
      }
      sourceDataUnit->inputData.push_back(inputDataVector);
    }

    BOOST_FOREACH(ptree::value_type outputDataJson,
      sourceDataJson.second.get_child("outputData"))
    {
      outputDataVector = new vector<bool>();
      BOOST_FOREACH(ptree::value_type outputDataUnitJson,
        outputDataJson.second)
      {
        outputDataVector->push_back(outputDataUnitJson.second.get_value<bool>());
      }
      sourceDataUnit->outputData.push_back(outputDataVector);
    }

    sourceDataUnit->title = sourceDataJson.second.get<string>("title");

    result->push_back(sourceDataUnit);
  }
  
  return result;
}


void 
WtmJsonLoader::saveInputSignals(string& destFile, 
vector<WtmNetwork::InputSignal*>* signals)
{
  ptree signalsArrayTree;
  ptree signalTree;
  ptree synapsesArrayTree;
  ptree synapseTree;

  for (WtmNetwork::InputSignal* signal : *signals)
  {
    synapsesArrayTree.clear();
    signalTree.clear();
    
    for (OutcomingSynapse* synapse : *signal->outcomingSynapses)
    {
      synapseTree.clear();

      synapseTree.put("destNeuronId", synapse->destNeuronId);
      synapseTree.put("distanceToDest", synapse->distanceToDest);

      synapsesArrayTree.push_back(std::make_pair("", synapseTree));
    }
    
    signalTree.put("id", signal->id);
    signalTree.add_child("outcomingSynapses", synapsesArrayTree);

    signalsArrayTree.push_back(std::make_pair("", signalTree));
  }

  ptree result;
  result.add_child("inputSignals", signalsArrayTree);

  appendToFile(destFile, result);
}


void
WtmJsonLoader::saveNeurons(string& destFile,
vector<WtmNetwork::Neuron*>& neurons)
{
  ptree neuronsArrayTree;
  ptree singleNeuronTree;
  ptree inputSynapseArrayTree;
  ptree outputSynapseArrayTree;
  ptree synapseTree;

  for (WtmNetwork::Neuron* neuron : neurons)
  {
    singleNeuronTree.clear();
    inputSynapseArrayTree.clear();
    outputSynapseArrayTree.clear();

    for (IncomingSynapse* synapse : *neuron->incomingSynapses)
    {
      synapseTree.clear();

      synapseTree.put("sourceNeuronId", synapse->sourceNeuronId);
      synapseTree.put("weight", synapse->weight);

      inputSynapseArrayTree.push_back(std::make_pair("", synapseTree));
    }

    for (OutcomingSynapse* synapse : *neuron->outcomingSynapses)
    {
      synapseTree.clear();

      synapseTree.put("destNeuronId", synapse->destNeuronId);
      synapseTree.put("distanceToDest", synapse->distanceToDest);

      outputSynapseArrayTree.push_back(std::make_pair("", synapseTree));
    }

    singleNeuronTree.put("id", neuron->id);
    singleNeuronTree.put("networkLayer", neuron->networkLayer);
    singleNeuronTree.add_child("incomingSynapses", inputSynapseArrayTree);
    singleNeuronTree.add_child("outcomingSynapses", outputSynapseArrayTree);
  
    neuronsArrayTree.push_back(std::make_pair("", singleNeuronTree));
  }
  
  ptree result;
  result.add_child("neurons", neuronsArrayTree);

  appendToFile(destFile, result);
}


void 
WtmJsonLoader::appendToFile(string& filename, ptree& additionTree)
{
  std::ifstream ifStream(filename);
  if (!ifStream)
  {
    cout << "file not found" << std::endl;
    return ;
  }

  ptree fileTree;
  read_json(ifStream, fileTree);

  ptree resultTree;

  BOOST_FOREACH(ptree::value_type& elem, fileTree)
    resultTree.add_child(elem.first, elem.second);

  BOOST_FOREACH(ptree::value_type& elem, additionTree)
    resultTree.add_child(elem.first, elem.second);

  write_json(filename, resultTree);
  //write_json(std::cout, resultTree);
}


vector<WtmNetwork::InputSignal*>* 
WtmJsonLoader::parseInputSignals(ptree& propertyTree, WtmNetwork* network)
{
  vector<WtmNetwork::InputSignal*>* result = new
    vector<WtmNetwork::InputSignal*>();

  WtmNetwork::InputSignal* signal;

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

    signal = new WtmNetwork::InputSignal(id, network, outcomingSynapses);

    result->push_back(signal);
    outcomingSynapses->clear();
  }

  return result;
}


vector<WtmNetwork::Neuron*>*
WtmJsonLoader::parseNeurons(ptree& propertyTree, WtmNetwork* network)
{
  vector<WtmNetwork::Neuron*>* result = new
    vector<WtmNetwork::Neuron*>();

  WtmNetwork::Neuron* neuron;

  int id;
  int networkLayer;

  vector<IncomingSynapse*>* incomingSynapses = new
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
      neuronObj.second.get_child("incomingSynapses"))
    {
      incomingSynapsePtr = new IncomingSynapse();

      incomingSynapsePtr->sourceNeuronId =
        incomingSynapse.second.get<int>("sourceNeuronId");
      incomingSynapsePtr->weight =
        incomingSynapse.second.get<float>("weight");

      incomingSynapses->push_back(incomingSynapsePtr);
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

    neuron = new WtmNetwork::Neuron(id, networkLayer, *network,
      incomingSynapses, outcomingSynapses);
    result->push_back(neuron);
  }
  return result;
}


WtmNetwork*
WtmJsonLoader::loadNetwork(string& filename)
{
  ptree* tree = loadJsonFile(filename);

  WtmNetwork* network = new WtmNetwork();

  network->parameters = parseParameters(tree);
  network->inputSignals = parseInputSignals(*tree, network);
  network->neurons = parseNeurons(*tree, network);

  return network;
}


void 
WtmJsonLoader::saveParameters(string& destFile,
WtmParameters* parameters)
{
  ptree resultTree;
  ptree gammaArrayTree;
  ptree* gammaTree;

  resultTree.put("networkPeriod", parameters->networkPeriod);
  resultTree.put("layersNumber", parameters->layersNumber);
  resultTree.put("inputNeuronsNumber", parameters->inputNeuronsNumber);
  resultTree.put("outputNeuronNumber", parameters->outputNeuronNumber);
  resultTree.put("uniqueNeuronsPerVNet", parameters->uniqueNeuronsPerVNet);
  resultTree.put("averageSynapseWeight", parameters->averageSynapseWeight);
  resultTree.put("activationThreshold", parameters->activationThreshold);
  resultTree.put("fatigueThreshold", parameters->fatigueThreshold);

  gammaTree = saveWtmSigma(parameters->thresholdIncreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveWtmSigma(parameters->thresholdDecreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveWtmSigma(parameters->fatigueIncreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveWtmSigma(parameters->fatigueDecreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveWtmSigma(parameters->weightIncreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveWtmSigma(parameters->weightDecreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  resultTree.add_child("sigmas", gammaArrayTree);

  ptree result;
  result.add_child("wtmParameters", resultTree);

  appendToFile(destFile, result);
}


ptree* 
WtmJsonLoader::saveWtmSigma(WtmSigma* sigma)
{
  ptree* result = new ptree();
  
  result->put("initialYScale", sigma->initialYScale);
  result->put("initialXOffset", sigma->initialXOffset);
  result->put("initialExpCoeff", sigma->initialExpCoeff);

  result->put("endYScale", sigma->endYScale);
  result->put("endXOffset", sigma->endXOffset);
  result->put("endExpCoeff", sigma->endExpCoeff);

  result->put("layersNumber", sigma->layersNumber);
  result->put("isIncreasing", sigma->isIncreasing);
  result->put("title", sigma->title);

  return result;
}


