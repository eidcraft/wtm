#include "stdafx.h"
#include "JsonDataLoader.h"


NetworkParameters* WtmJsonLoader::loadParameters(fstream& ifStream)
{
  ptree* propertyTree = loadJsonFile(ifStream);
  return parseParameters(propertyTree);
}


ptree*  WtmJsonLoader::loadJsonFile(fstream& ifStream)
{
  ptree* propertyTree = new ptree();
  read_json(ifStream, *propertyTree);

  return propertyTree;
}


NetworkParameters* WtmJsonLoader::parseParameters(ptree* propertyTree)
{
  NetworkParameters* wtmParameters = new NetworkParameters();

  ptree parametersObj = propertyTree->get_child("parameters");
  
  // if (!parametersObj) out of there

  parseNumberParameters(parametersObj, wtmParameters);
  parseSigmaParameters(parametersObj, wtmParameters);

  return wtmParameters;
}


void WtmJsonLoader::parseNumberParameters(ptree& propertyTree, NetworkParameters* parameters)
{
  parameters->networkPeriod = propertyTree.get<int>("networkPeriod");
  parameters->signalsPerAction = propertyTree.get<int>("signalsPerAction");
  parameters->layersNumber = propertyTree.get<int>("layersNumber");
  parameters->activationThreshold = propertyTree.get<float>("activationThreshold");
  parameters->fatigueThreshold = propertyTree.get<float>("fatigueThreshold");
  parameters->inputNeuronsNumber = propertyTree.get<int>("inputNeuronsNumber");
  parameters->outputNeuronNumber = propertyTree.get<int>("outputNeuronNumber");
  parameters->uniqueNeuronsPerVNet = propertyTree.get<int>("uniqueNeuronsPerVNet"); 
  parameters->averageSynapseWeight = propertyTree.get<float>("averageSynapseWeight");
  parameters->initialSynapseWeight = propertyTree.get<float>("initialSynapseWeight");
}


void WtmJsonLoader::parseSigmaParameters(ptree& propertyTree, NetworkParameters* parameters)
{
  float initialYScale;
  float initialXOffset;
  float initialExpCoeff;
  float endYScale;
  float endXOffset;
  float endExpCoeff;
  bool isIncreasing;
  string title;

  Sigma* sigma;

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

    sigma = new Sigma(initialYScale, initialXOffset, initialExpCoeff,
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


vector<WtmSourceData*>* 
WtmJsonLoader::loadMotorBasisData(fstream& ifStream)
{
  ptree* propertyTree = loadJsonFile(ifStream);
  return parseMotorBasisSources(propertyTree);
}


vector<WtmSourceData*>*
WtmJsonLoader::loadBasicBehaviorData(fstream& ifStream)
{
  ptree* propertyTree = loadJsonFile(ifStream);
  return parseBasicBehaviorSources(propertyTree);
}


vector<WtmSourceData*>* 
WtmJsonLoader::parseMotorBasisSources(ptree* propertyTree)
{
  vector<WtmSourceData*>* result = new vector<WtmSourceData*>();

  WtmSourceData* sourceDataUnit = nullptr;

  vector<bool>* inputDataVector;
  //vector<bool>* outputDataVector;

  BOOST_FOREACH(ptree::value_type sourceDataJson, propertyTree->get_child("motorBasis"))
  {
    inputDataVector = new vector<bool>();
    //outputDataVector = new vector<bool>();
    
    ptree inputDataJson = sourceDataJson.second.get_child("inputData");
    //ptree outputDataJson = sourceDataJson.second.get_child("outputData");

    BOOST_FOREACH(ptree::value_type inputDataUnitJson, inputDataJson)
      inputDataVector->push_back(inputDataUnitJson.second.get_value<bool>());
    //BOOST_FOREACH(ptree::value_type outputDataUnitJson, outputDataJson)
    //  outputDataVector->push_back(outputDataUnitJson.second.get_value<bool>());

    sourceDataUnit = new WtmSourceData();
    sourceDataUnit->inputData.push_back(inputDataVector);
    //sourceDataUnit->outputData.push_back(outputDataVector);
    sourceDataUnit->title = sourceDataJson.second.get<string>("title");

    result->push_back(sourceDataUnit);
  }

  return result;
}


vector<WtmSourceData*>* 
WtmJsonLoader::parseBasicBehaviorSources(ptree* propertyTree)
{
  vector<WtmSourceData*>* result = new vector<WtmSourceData*>();

  WtmSourceData* sourceDataUnit = nullptr;

  vector<bool>* inputDataVector;

  BOOST_FOREACH(ptree::value_type sourceDataJson, propertyTree->get_child("basicBehavior"))
  {
    sourceDataUnit = new WtmSourceData();
    sourceDataUnit->title = sourceDataJson.second.get<string>("title");

    BOOST_FOREACH(ptree::value_type inputDataJson, sourceDataJson.second.get_child("inputData"))
    {
      inputDataVector = new vector<bool>();

      BOOST_FOREACH(ptree::value_type inputDataUnitJson, inputDataJson.second)
        inputDataVector->push_back(inputDataUnitJson.second.get_value<bool>());

      sourceDataUnit->inputData.push_back(inputDataVector);
    }

    result->push_back(sourceDataUnit);
  }

  return result;
}


ptree 
WtmJsonLoader::packInputSignals(vector<InputSignal*>* signals)
{
  ptree signalsArrayTree;
  ptree signalTree;
  ptree synapsesArrayTree;
  ptree synapseTree;

  for (InputSignal* signal : *signals)
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

  //ptree result;
  return signalsArrayTree;
  //result.add_child("inputSignals", signalsArrayTree);

  //appendToFile(destFile, result);
  //write_json(fstream, result);

}


ptree
WtmJsonLoader::packNeurons(vector<Neuron*>* neurons)
{
  ptree neuronsArrayTree;
  ptree singleNeuronTree;
  ptree inputSynapseArrayTree;
  ptree outputSynapseArrayTree;
  ptree synapseTree;

  Neuron* neuron;


  for (int i = 0; i < neurons->size(); i++)
  {
    if (neurons->at(i)) neuron = neurons->at(i);
    else continue;

    singleNeuronTree.clear();
    inputSynapseArrayTree.clear();
    outputSynapseArrayTree.clear();

    for (IncomingSynapse* synapse : *neuron->straightSynapses)
    {
      synapseTree.clear();

      synapseTree.put("sourceNeuronId", synapse->sourceNeuronId);
      synapseTree.put("weight", synapse->weight);

      inputSynapseArrayTree.push_back(std::make_pair("", synapseTree));
    }

    for (OutcomingSynapse* synapse : *neuron->strOutcomingSynapses)
    {
      synapseTree.clear();

      synapseTree.put("destNeuronId", synapse->destNeuronId);
      synapseTree.put("distanceToDest", synapse->distanceToDest);

      outputSynapseArrayTree.push_back(std::make_pair("", synapseTree));
    }
    for (OutcomingSynapse* synapse : *neuron->sideOutcomingSynapses)
    {
      synapseTree.clear();

      synapseTree.put("destNeuronId", synapse->destNeuronId);
      synapseTree.put("distanceToDest", synapse->distanceToDest);

      outputSynapseArrayTree.push_back(std::make_pair("", synapseTree));
    }

    singleNeuronTree.put("id", neuron->id);
    singleNeuronTree.put("networkLayer", neuron->networkLayer);
    singleNeuronTree.add_child("straightSynapses", inputSynapseArrayTree);
    singleNeuronTree.add_child("outcomingSynapses", outputSynapseArrayTree);
  
    neuronsArrayTree.push_back(std::make_pair("", singleNeuronTree));
  }
  
  return neuronsArrayTree;
}


void
WtmJsonLoader::appendToFile(std::ifstream& ifStream, ptree& additionTree)
{

  ptree fileTree;
  if (ifStream.peek() != std::ifstream::traits_type::eof())
    read_json(ifStream, fileTree);

  ptree resultTree;

  BOOST_FOREACH(ptree::value_type& elem, fileTree)
    resultTree.add_child(elem.first, elem.second);

  BOOST_FOREACH(ptree::value_type& elem, additionTree)
    resultTree.add_child(elem.first, elem.second);

  //write_json(filename, resultTree);
  //write_json(std::cout, resultTree);
}


vector<InputSignal*>* 
WtmJsonLoader::parseInputSignals(ptree& propertyTree, Network* network)
{
  vector<InputSignal*>* result = new
    vector<InputSignal*>();

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

    result->push_back(signal);
    outcomingSynapses->clear();
  }

  return result;
}


vector<Neuron*>*
WtmJsonLoader::parseNeurons(ptree& propertyTree, Network* network)
{
  vector<Neuron*>* result = new
    vector<Neuron*>();

  Neuron* neuron;

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
        outcomingSynapse.second.get<int>("distance");

      outcomingSynapses->push_back(outcomingSynapsePtr);
    }

    neuron = new Neuron(id, networkLayer, network,
      incomingSynapses, outcomingSynapses);
    result->push_back(neuron);
  }
  return result;
}


Network*
WtmJsonLoader::loadNetwork(string& filename)
{
  fstream fstream(filename.c_str(), std::fstream::in);
  ptree* tree = loadJsonFile(fstream);

  if (!tree) return nullptr;
  
  NetworkParameters* wtmParameters = parseParameters(tree);

  Network* network = new Network(wtmParameters);

  network->inputSignals = parseInputSignals(*tree, network);
  network->neuronsInRow = parseNeurons(*tree, network);
  network->outNeurons = parseOutNeurons(*tree, network);

  network->splitNeuronsToLayers();

  return network;
}


ptree
WtmJsonLoader::packParameters(NetworkParameters* parameters)
{
  ptree resultTree;
  ptree gammaArrayTree;
  ptree* gammaTree;

  resultTree.put("networkPeriod", parameters->networkPeriod);
  resultTree.put("signalsPerAction", parameters->signalsPerAction);
  resultTree.put("layersNumber", parameters->layersNumber);
  resultTree.put("inputNeuronsNumber", parameters->inputNeuronsNumber);
  resultTree.put("outputNeuronNumber", parameters->outputNeuronNumber);
  resultTree.put("uniqueNeuronsPerVNet", parameters->uniqueNeuronsPerVNet);
  resultTree.put("averageSynapseWeight", parameters->averageSynapseWeight);
  resultTree.put("initialSynapseWeight", parameters->initialSynapseWeight);
  resultTree.put("activationThreshold", parameters->activationThreshold);
  resultTree.put("fatigueThreshold", parameters->fatigueThreshold);

  gammaTree = saveSigma(parameters->thresholdIncreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveSigma(parameters->thresholdDecreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveSigma(parameters->fatigueIncreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveSigma(parameters->fatigueDecreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveSigma(parameters->weightIncreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  gammaTree = saveSigma(parameters->weightDecreaseFunc);
  gammaArrayTree.push_back(std::make_pair("", *gammaTree));

  resultTree.add_child("sigmas", gammaArrayTree);

  return resultTree;
}


ptree* 
WtmJsonLoader::saveSigma(Sigma* sigma)
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


ptree
WtmJsonLoader::packOutNeurons(vector<Neuron*>* neurons)
{
  ptree resultTree;
  ptree subResultTree;

  for (Neuron* neuron : *neurons)
  {
    resultTree.put("", neuron->id);
    subResultTree.push_back(std::make_pair("", resultTree));
  }

  return subResultTree;
}


vector<Neuron*>*
WtmJsonLoader::parseOutNeurons(ptree& propertyTree, Network* network)
{
  vector<Neuron*>* result = new vector<Neuron*>();
  int neuronsId;
  int neuronsIdPadding = network->inputSignals->size() + 1;

  BOOST_FOREACH(ptree::value_type neuronIdObj, propertyTree.get_child("outNeurons"))
  {
    neuronsId = neuronIdObj.second.get<int>("");
    result->push_back(network->neuronsInRow->at(neuronsId - neuronsIdPadding));
  }

  return result;
}


WtmSourceDataSet* 
WtmJsonLoader::loadWtmSourceDataSet(string& filename)
{
  fstream fstream(filename.c_str(), std::fstream::in);
  ptree* propertyTree = loadJsonFile(fstream);

  NetworkParameters* networkParameters = parseParameters(propertyTree);
  MotorBasisData*    motorBasisData = parseMotorBasisSources(propertyTree);
  BasicBehaviorData* basicBehaviorData = parseBasicBehaviorSources(propertyTree);

  return new WtmSourceDataSet(networkParameters, motorBasisData, basicBehaviorData);
}



void 
WtmJsonLoader::saveNetwork(string& filename, Network& network)
{
  ptree result;
  result.add_child("inputSignals", packInputSignals(network.inputSignals));
  result.add_child("neurons", packNeurons(network.neuronsInRow));
  result.add_child("outNeurons", packOutNeurons(network.outNeurons));
  result.add_child("parameters", packParameters(network.parameters));

  fstream fstream(filename.c_str(), std::fstream::out | std::fstream::trunc);
  write_json(fstream, result);
}
