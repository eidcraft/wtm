#include "stdafx.h"
#include "NetworkCreator.h"
#include "math.h"



Network* 
NetworkCreator::create(WtmSourceDataSet* networkSourceData)
{
  Network* network = new Network(networkSourceData->networkParameters);

  network->isNetworkBuilding = true;

  buildFirstLayer(network);
  buildMiddleLayers(network, networkSourceData);
  buildLastLayer(network, networkSourceData);

  //buildSideSynapses(network);
  buildInitialBehavior(network, networkSourceData);

  //calcSideSynapsesContribution(network);

  network->flushNetwork();
  network->isNetworkBuilding = false;

  return network;
}




void
NetworkCreator::buildFirstLayer(Network* network)
{
  ÑombinationsFabric* combFabric = new ÑombinationsFabric(
    network->parameters->inputNeuronsNumber, 
    network->parameters->signalsPerAction);
  Combination* combination;

  vector<InputSignal*>* inputSignalsGroup = new vector<InputSignal*>();
  vector<Neuron*>* neuronsLayer = new vector<Neuron*>();
  Neuron* neuron;

  combination = combFabric->nextCombination();
  while (combination)
  {
    for (int iInputSignal : *combination)
      inputSignalsGroup->push_back(network->inputSignals->at(iInputSignal - 1));

    neuron = createNeuronInLayer(network, 0);
    neuronsLayer->push_back(neuron);
    buildSynapsesFromTo(network, inputSignalsGroup, neuron);

    inputSignalsGroup->clear();
    combination = combFabric->nextCombination();
  }

  network->neurons->push_back(neuronsLayer);
  network->mergeNeuronsToLine();
  calcStraigtSynapsesContribution(network);
}



void
NetworkCreator::buildMiddleLayers(Network* network, WtmSourceDataSet* networkSourceData)
{
  ÑombinationsFabric* combFabric = new ÑombinationsFabric(
    network->neurons->at(0)->size(),
    network->parameters->signalsPerAction);
  Combination* combination;

  Neuron* neuron;
  vector<Neuron*>* newLayerNeurons;
  vector<Neuron*>* prevLayerNeurons;

  for (int iLayer = 1; iLayer < network->parameters->layersNumber - 1; iLayer++)
  {
    newLayerNeurons = new vector<Neuron*>();

    combination = combFabric->nextCombination();
    while (combination)
    {
      prevLayerNeurons = new vector<Neuron*>();

      for (int iNeuron : *combination)
        prevLayerNeurons->push_back(
          network->neurons->at(iLayer - 1)->at(iNeuron - 1));

      neuron = createNeuronInLayer(network, iLayer);
      newLayerNeurons->push_back(neuron);
      buildSynapsesFromTo(network, prevLayerNeurons, neuron);

      combination = combFabric->nextCombination();
    }
    network->neurons->push_back(newLayerNeurons);
  }
  network->mergeNeuronsToLine();
  calcStraigtSynapsesContribution(network);
}



void
NetworkCreator::buildLastLayer(Network* network, WtmSourceDataSet* networkSourceData)
{
  Neuron* neuron;

  vector<Neuron*>* prevLayerActiveNeurons;
  vector<vector<Neuron*>*>* prevLayerActiveNeuronsGroups = new
    vector<vector<Neuron*>*>();

  vector<Neuron*>* lastLayer = new vector<Neuron*>();


  for (int iMBasisUnit = 0; iMBasisUnit < networkSourceData->motorBasisData->size(); iMBasisUnit++)
  {
    network->flushNetwork();
    network->installInputData(*networkSourceData->motorBasisData->at(iMBasisUnit)->inputData.at(0));
    network->makeProcessingPeriod();

    prevLayerActiveNeuronsGroups->push_back(network->getActiveNeurons());
    
    neuron = createNeuronInLayer(network, network->parameters->layersNumber - 1);
    lastLayer->push_back(neuron);
  }

  for (int i = 0; i < lastLayer->size(); i++)
    buildSynapsesFromTo(network, 
      prevLayerActiveNeuronsGroups->at(i), lastLayer->at(i));

  network->neurons->push_back(lastLayer);
  network->outNeurons = lastLayer;

  network->mergeNeuronsToLine();
  calcStraigtSynapsesContribution(network);
}



void
NetworkCreator::buildInitialBehavior(Network* network, WtmSourceDataSet* sourceData)
{
  vector<Neuron*>* firstNetActiveNeurons;
  vector<Neuron*>* secondNetActiveNeurons;

  vector<vector<Neuron*>>* firstNetworkNeuronsGroups;
  vector<vector<Neuron*>>* secondNetworkNeuronsGroups;
  
  Network* firstNetworkCopy = network->getNetworkCopy();
  Network* secondNetworkCopy = network->getNetworkCopy();

  int counterFinish;

  for (int iBehavior = 0; iBehavior < sourceData->basicBehaviorData->size(); iBehavior++)
  {
    firstNetworkCopy->flushNetwork();
    secondNetworkCopy->flushNetwork();

    int iBehaviorStep = 0;
    int stepsCounter = 0;

    int counterFinish = (sourceData->basicBehaviorData->at(iBehavior)->inputData.size() - 1) *
      network->parameters->networkPeriod;
    while (stepsCounter != counterFinish)
    {

      if (0 == ((stepsCounter + 1) % network->parameters->networkPeriod) &&
        iBehaviorStep + 1 < sourceData->basicBehaviorData->at(iBehavior)->inputData.size())
      {
        stepsCounter++;
        continue;
      }

      firstNetworkCopy->makeNeuronsNotActive();
      secondNetworkCopy->makeNeuronsNotActive();

      if (0 == (stepsCounter % network->parameters->networkPeriod) &&
        iBehaviorStep + 1 < sourceData->basicBehaviorData->at(iBehavior)->inputData.size())
      {
        firstNetworkCopy->installInputData(
          *sourceData->basicBehaviorData->at(iBehavior)->inputData.at(iBehaviorStep));

        secondNetworkCopy->installInputData(
          *sourceData->basicBehaviorData->at(iBehavior)->inputData.at(iBehaviorStep + 1));

        iBehaviorStep++;
      }

      firstNetworkCopy->makeProcessingTact();
      secondNetworkCopy->makeProcessingTact();

      firstNetActiveNeurons = firstNetworkCopy->getActiveNeurons();
      secondNetActiveNeurons = secondNetworkCopy->getActiveNeurons();

      firstNetworkNeuronsGroups = new vector<vector<Neuron*>>(
        network->parameters->layersNumber);
      secondNetworkNeuronsGroups = new vector<vector<Neuron*>>(
        network->parameters->layersNumber);

      for (Neuron* neuron : *firstNetActiveNeurons)
        firstNetworkNeuronsGroups->at(neuron->networkLayer).push_back(neuron);
      for (Neuron* neuron : *secondNetActiveNeurons)
        secondNetworkNeuronsGroups->at(neuron->networkLayer).push_back(neuron);

      for (int iGroup = 0; iGroup != network->parameters->layersNumber - 1; iGroup++)
        createAssosiation(network,
        &firstNetworkNeuronsGroups->at(iGroup), &secondNetworkNeuronsGroups->at(iGroup));

      stepsCounter++;

    }
  }
}



void 
NetworkCreator::calcSideSynapsesContribution(Network* network)
{
  Neuron* neuron;
  IncomingSynapse* synapse;

  vector<int>* chargeTypeTable;
  int nSynapses;

  for (int iNeuron = 0; iNeuron < network->neuronsInRow->size(); iNeuron++)
  {
    neuron = network->neuronsInRow->at(iNeuron);

    neuron->chargesContributions = new vector<float>();

    nSynapses = 0;
    for (int iSynapse = 0; iSynapse < neuron->reverseSynapses->size(); iSynapse++)
      nSynapses += neuron->reverseSynapses->at(iSynapse)->connectionsNumber;

    for (size_t i = 0; i < neuron->reverseSynapses->size(); i++)
      neuron->chargesContributions->push_back((float)
        neuron->reverseSynapses->at(i)->connectionsNumber / nSynapses);
  }
}


void
NetworkCreator::buildSynapsesFromTo(
  Network* net,
  vector<Neuron*>* sources, Neuron* dest)
{
  if (!dest || NULL == sources->size()) return;
  
  float nSourceNeurons = sources->size();
  
  int synapsesRequired = std::ceil(
    net->parameters->activationThreshold / net->parameters->averageSynapseWeight);
  
  
  int iSynapses;
  vector<int>* synapsesPlacingPlan;
  
  iSynapses = synapsesRequired;
  
  while (iSynapses >= nSourceNeurons)
  {
    for (int iSource = 0; iSource < nSourceNeurons; iSource++)
    {
      buildSynapseFromTo(net, sources->at(iSource), dest);
    }
    iSynapses -= nSourceNeurons;
  }
  
  if (iSynapses > 0)
  {
    synapsesPlacingPlan = getSynapsesPlacingPlan(nSourceNeurons, iSynapses);
  
    for (int i = 0; i < iSynapses; i++)
    {
      buildSynapseFromTo(net, sources->at(synapsesPlacingPlan->at(i)), dest);
    }
  }
}
  


void 
NetworkCreator::buildSynapseFromTo(Network* net, Neuron* source, Neuron* dest)
{
  IncomingSynapse* existingSynapse =
    dest->findStrSynapseByNeuronId(source->id);

  if (existingSynapse) 
  {
    existingSynapse->connectionsNumber++;
    return;
  }

  OutcomingSynapse* synapseBegin = new OutcomingSynapse();
  synapseBegin->destNeuronId = dest->id;
  synapseBegin->isWorking = true;
  synapseBegin->distanceToDest = std::abs(dest->networkLayer - 
    source->networkLayer);

  if (dest->networkLayer == source->networkLayer)
    source->sideOutcomingSynapses->push_back(synapseBegin);
  else
    source->strOutcomingSynapses->push_back(synapseBegin);

  IncomingSynapse* synapseEnd = new IncomingSynapse();
  synapseEnd->connectionsNumber = 1;
  synapseEnd->sourceNeuronId = source->id;
  synapseEnd->weight = net->parameters->averageSynapseWeight;

  if (dest->straightSynapses->size() != 0)
    synapseEnd->neuronChargeType = 
      dest->straightSynapses->back()->neuronChargeType + 1;

  dest->straightSynapses->push_back(synapseEnd);
}




void
NetworkCreator::buildSynapsesFromTo(
Network* net,
vector<InputSignal*>* sources, Neuron* dest)
{
  if (!dest) return;

  float nSourceNeurons = sources->size();

  int synapsesRequired = std::ceil(
    net->parameters->activationThreshold / net->parameters->averageSynapseWeight);


  int iSynapses;
  vector<int>* synapsesPlacingPlan;

  iSynapses = synapsesRequired;

  while (iSynapses >= nSourceNeurons)
  {
    for (int iSource = 0; iSource < nSourceNeurons; iSource++)
      buildSynapseFromTo(net, sources->at(iSource), dest);
    iSynapses -= nSourceNeurons;
  }

  if (iSynapses > 0)
  {
    synapsesPlacingPlan = getSynapsesPlacingPlan(nSourceNeurons, iSynapses);
    for (int i = 0; i < iSynapses; i++)
      buildSynapseFromTo(net, sources->at(synapsesPlacingPlan->at(i)), dest);
  }
}


void 
NetworkCreator::buildSynapseFromTo(Network* net, InputSignal* source, Neuron* dest)
{
  IncomingSynapse* existingSynapse =
    dest->findStrSynapseByNeuronId(source->id);

  if (existingSynapse)
  {
    existingSynapse->connectionsNumber++;
    return;
  }

  OutcomingSynapse* synapseBegin = new OutcomingSynapse();
  synapseBegin->destNeuronId = dest->id;
  synapseBegin->isWorking = true;
  synapseBegin->distanceToDest = 1;
  source->outcomingSynapses->push_back(synapseBegin);

  IncomingSynapse* synapseEnd = new IncomingSynapse();
  synapseEnd->connectionsNumber = 1;
  synapseEnd->sourceNeuronId = source->id;
  synapseEnd->weight = net->parameters->averageSynapseWeight;

  if (dest->straightSynapses->size() != 0)
    synapseEnd->neuronChargeType =
    dest->straightSynapses->back()->neuronChargeType + 1;

  dest->straightSynapses->push_back(synapseEnd);
}


#include <iostream>

void
NetworkCreator::buildSideSynapses(Network* network)
{
  for (int iLayer = 0; iLayer < network->parameters->layersNumber; iLayer++)
    for (int iSource = 0; iSource < network->neurons->at(iLayer)->size(); iSource++)
      for (int iDest = 0; iDest < network->neurons->at(iLayer)->size(); iDest++)
        if (iSource != iDest)
          buildSideSynapse(
          network,
          network->neurons->at(iLayer)->at(iSource),
          network->neurons->at(iLayer)->at(iDest),
          -1);
}



OutcomingSynapse*
NetworkCreator::buildSideSynapse(
  Network* network, Neuron* source, Neuron* dest, int type = -1)
{
  IncomingSynapse* existingSynapse =
    dest->findSideSynapseByNeuronId(source->id);

  if (existingSynapse)
  {
    existingSynapse->connectionsNumber++;
    
    vector<Neuron*>* neuronWrap = new vector<Neuron*>();
    neuronWrap->push_back(source);

    return findOutSynapseByInSynapse(neuronWrap, existingSynapse, dest->id);
  }

  OutcomingSynapse* synapseBegin = new OutcomingSynapse();
  synapseBegin->destNeuronId = dest->id;
  synapseBegin->distanceToDest = dest->network.parameters->networkPeriod;

  if (dest->networkLayer == source->networkLayer)
    source->sideOutcomingSynapses->push_back(synapseBegin);
  else
    source->strOutcomingSynapses->push_back(synapseBegin);

  IncomingSynapse* synapseEnd = new IncomingSynapse();
  synapseEnd->connectionsNumber = 1;
  synapseEnd->sourceNeuronId = source->id;
  synapseEnd->weight = 0;
  
  if (type == -1)
    synapseEnd->neuronChargeType = !dest->reverseSynapses->empty() ?
      (dest->reverseSynapses->back()->neuronChargeType + 1) % dest->straightSynapses->size() :
      0;
  else
    synapseEnd->neuronChargeType = type;
  
  dest->reverseSynapses->push_back(synapseEnd);

  return synapseBegin;
}



Neuron*
NetworkCreator::createNeuronInLayer(Network* network, int layer)
{
  int neuronId = network->takeLastNeuronId();
  Neuron* neuron = new Neuron(neuronId, layer, network);
  return neuron;
}



void
NetworkCreator::calcStraigtSynapsesContribution(Network* network)
{
  IncomingSynapse* synapse;

  vector<int>* chargeTypeTable;
  int nSynapses;

  for (Neuron* neuron : *network->neuronsInRow)
  {
    if (!neuron->neuronCharges->empty()) continue;

    nSynapses = 0;

    for (int iSynapse = 0; iSynapse < neuron->straightSynapses->size(); iSynapse++)
      nSynapses += neuron->straightSynapses->at(iSynapse)->connectionsNumber;

    for (size_t i = 0; i < neuron->straightSynapses->size(); i++)
      neuron->chargesContributions->push_back(
      (float)neuron->straightSynapses->at(i)->connectionsNumber / (float)nSynapses);

    neuron->neuronCharges = new vector<float>(neuron->chargesContributions->size());
  }
}



void
NetworkCreator::createAssosiation(
  Network* net, vector<Neuron*>* sourcesCopys, vector<Neuron*>* destsCopys)
{
  if (!sourcesCopys || !destsCopys ||
    sourcesCopys->size() == 0 || destsCopys->size() == 0)
    return;

  vector<Neuron*>* sources = new vector<Neuron*>();
  vector<Neuron*>* dests = new vector<Neuron*>();

  for (Neuron* copy : *sourcesCopys)
    sources->push_back(net->neuronsInRow->at(copy->id));
  for (Neuron* copy : *destsCopys)
    dests->push_back(net->neuronsInRow->at(copy->id));

  int synapseIncomingsNumber;

  IncomingSynapse* straightSynapses;
  vector<IncomingSynapse*>* sideSynapses;

  IncomingSynapse* inSynapse;
  float newContribution;

  Neuron* sourceNeuron;
  for (Neuron* destNeuron : *dests)
  {
    for (int iType = 0; iType < destNeuron->straightSynapses->size(); iType++)
    {
      sideSynapses =  findSideSynapses(sources, destNeuron, iType);
      inSynapse = getRandomInSynapseFromGroup(sideSynapses);

      if (inSynapse) continue;

      sourceNeuron = getRandomNeuronFromGroup(sources);
      buildSideSynapse(net, sourceNeuron, destNeuron, iType);

      inSynapse = destNeuron->getInSynapseFrom(sourceNeuron->id);
      inSynapse->weight = net->parameters->averageSynapseWeight;
      inSynapse->connectionsNumber = destNeuron->straightSynapses->at(iType)->connectionsNumber;
    }
  }
}



vector<IncomingSynapse*>*
NetworkCreator::findSideSynapses(
  vector<Neuron*>* sources, Neuron* dest, int type)
{
  vector<IncomingSynapse*>* result = new vector<IncomingSynapse*>();

  for (int iSynapse = 0; iSynapse < dest->reverseSynapses->size(); iSynapse++)
    if (dest->reverseSynapses->at(iSynapse)->neuronChargeType == type &&
      isIdInArray(sources, dest->reverseSynapses->at(iSynapse)->sourceNeuronId))
      result->push_back(dest->reverseSynapses->at(iSynapse));

  return result;
}



bool 
NetworkCreator::isIdInArray(vector<Neuron*>* neurons, int id)
{
  for (Neuron* neuron : *neurons)
    if (neuron->id == id) return true;
  
  return false;
}


Neuron* 
NetworkCreator::getRandomNeuronFromGroup(vector<Neuron*>* neuronsGroup)
{
  if (neuronsGroup->empty()) return nullptr;

  int rand = std::rand() % neuronsGroup->size();
  return neuronsGroup->at(rand);
}


IncomingSynapse*
NetworkCreator::getRandomInSynapseFromGroup(vector<IncomingSynapse*>* synapses)
{
  if (synapses->empty()) return nullptr;
  
  int rand = std::rand() % synapses->size();
  return synapses->at(rand);
}


OutcomingSynapse* 
NetworkCreator::findOutSynapseByInSynapse(
vector<Neuron*>* neurons, IncomingSynapse* inSynapse, int destNeuronId)
{
  for (Neuron* neuron : *neurons)
    if (neuron->id == inSynapse->sourceNeuronId)
    {
      for (OutcomingSynapse* outSynapse : *neuron->sideOutcomingSynapses)
        if (outSynapse->destNeuronId == destNeuronId) return outSynapse;
    }
  
  return nullptr;
}


