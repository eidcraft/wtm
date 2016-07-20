#include "stdafx.h"
#include "Network.h"
#include <math.h>  
#include <iostream>

void Network::processNeuron(Neuron* neuron)
{
  int neuronUpdateLag = lastUpdateTime - neuron->lastUpdateTime;

  neuron->updateParameters(neuronUpdateLag);
  
  if (!neuron->isAbleToTakeSignals()) return;

  neuron->receiveSignals();

  if (neuron->isThresholdExceeded())
  {
    neuron->processActivation();
    neuron->sendSignals();
  }
 
  neuron->flushCharges();
}


void Neuron::updateParameters(int updateLag)
{
  activationThreshold = network.parameters->
    getIncreasedActivationThreshold(
    networkLayer, activationThreshold, updateLag);

  //for (IncomingSynapse* synapse : *straightSynapses)
  //  synapse->weight = network.parameters->
  //    getDecreasedSynapticWeight(
  //    networkLayer, synapse->weight, updateLag);

  fatigueSpeed = network.parameters->
    getDecreasedFatigueSpeed(
    networkLayer, fatigueSpeed, updateLag);

  fatigue += restSpeed;
}


bool Neuron::isAbleToTakeSignals()
{
  return fatigue < network.parameters->fatigueThreshold;
}


void Neuron::receiveSignals()
{
  for (IncomingSynapse* synapse : *straightSynapses)
    neuronCharges->at(synapse->neuronChargeType) += 
      synapse->isActive ? synapse->weight * synapse->connectionsNumber : 0;
  
  for (IncomingSynapse* synapse : *reverseSynapses)
    neuronCharges->at(synapse->neuronChargeType) +=
      synapse->isActive ? synapse->weight * synapse->connectionsNumber : 0;
}


bool Neuron::isThresholdExceeded()
{
  bool isExceeded;
  for (int iCharge = 0;  iCharge < neuronCharges->size(); iCharge++)
  {
    isExceeded = neuronCharges->at(iCharge) + 0.00001 >=
      chargesContributions->at(iCharge) * activationThreshold;
    
    if (!isExceeded) return false;
  }
  return true;
}


void Neuron::sendSignals()
{
  SynapticSignal* newSignal = new SynapticSignal(id);
  for (OutcomingSynapse* synapse : *strOutcomingSynapses)
    newSignal->destNeuronsIds->push_back(synapse->destNeuronId);
  network.registerSignalForDelivery(newSignal, 1);

  if (sideOutcomingSynapses->empty()) return;

  newSignal = new SynapticSignal(id);
  for (OutcomingSynapse* synapse : *sideOutcomingSynapses)
    newSignal->destNeuronsIds->push_back(synapse->destNeuronId);
  network.registerSignalForDelivery(newSignal,
    network.parameters->networkPeriod);
}


void InputSignal::sendSignals()
{
  SynapticSignal* newSignal = new SynapticSignal(id);
  for (OutcomingSynapse* synapse : *outcomingSynapses)
    newSignal->destNeuronsIds->push_back(synapse->destNeuronId);
  network->registerSignalForDelivery(newSignal, 1);
}


void Neuron::processActivation()
{
  isActive = true;

  flushCharges();

  activationThreshold = network.parameters->
    getDecreasedActivationThreshold(networkLayer, activationThreshold);

  fatigueSpeed = network.parameters->
    getIncreasedFatigueSpeed(networkLayer, fatigueSpeed);

  fatigue += fatigueSpeed;

}


void Neuron::resetSynapsesActivityFlag()
{
  for (IncomingSynapse* synapse : *straightSynapses)
    synapse->isActive = false;
  for (IncomingSynapse* synapse : *reverseSynapses)
    synapse->isActive = false;
}


void Network::makeProcessingTact()
{
  activeNeuronsId->clear();

  refreshQueues();

  deliverSignals();

  processActiveNeurons();

  if (isNetworkBuilding) return;

  storeRegularities();
}



void Network::makeProcessingPeriod()
{
  for (int iLayer = 0; iLayer < parameters->layersNumber; iLayer++)
    makeProcessingTact();
}



void Network::installInputData(vector<bool>& inputData)
{
  for (int iSignal = 0; iSignal != inputData.size(); iSignal++)
  {
    if (inputData.at(iSignal))
    {
      inputSignals->at(iSignal)->isActive = true;
      inputSignals->at(iSignal)->sendSignals();
    }
    else
    {
      inputSignals->at(iSignal)->isActive = false;
    }

  }
}


void Network::processInputNeuronsActivation()
{

}


void Network::deliverSignals()
{
  if (!synapticSignalsQueues.size()) return;

  for (SynapticSignal* signal : synapticSignalsQueues.at(currentQueueNumber))
    for (int neuronId : *signal->destNeuronsIds)
      neuronsInRow->at(neuronId)->resetSynapsesActivityFlag();

  for (SynapticSignal* signal : synapticSignalsQueues.at(currentQueueNumber))
    for (int neuronId : *signal->destNeuronsIds)
    {
      neuronsInRow->at(neuronId)->registerIncomingSignal(signal->sourceNeuronId);
      addNeuronIdToActive(neuronId);
    }
}


void Network::refreshQueues()
{
  clearLastQueue();
  makeQueuesCircleSwap();
}


void Network::clearLastQueue()
{
  synapticSignalsQueues.at(currentQueueNumber).clear();
}


void Network::makeQueuesCircleSwap()
{
  currentQueueNumber = (currentQueueNumber + 1) % synapticSignalsQueues.size();
}


void Network::processActiveNeurons()
{
  for (int neuronId : *activeNeuronsId)
    processNeuron(neuronsInRow->at(neuronId));
}


void 
Neuron::registerIncomingSignal(int sourceNeuronId)
{
  vector<IncomingSynapse*>* synapses = findSynapses(sourceNeuronId);

  for (IncomingSynapse* synapse : *synapses)
    synapse->isActive = true;
}


IncomingSynapse*
Neuron::findSideSynapseByNeuronId(int neuronId)
{
  for (IncomingSynapse* synapse : *reverseSynapses)
    if (synapse->sourceNeuronId == neuronId)
      return synapse;

  return nullptr;
}


IncomingSynapse*
Neuron::findStrSynapseByNeuronId(int neuronId)
{
  for (IncomingSynapse* synapse : *straightSynapses)
    if (synapse->sourceNeuronId == neuronId)
      return synapse;

  return nullptr;
}


vector<IncomingSynapse*>*
Neuron::findSynapses(int neuronId)
{
  vector<IncomingSynapse*>* result = new vector<IncomingSynapse*>();

  for (IncomingSynapse* synapse : *straightSynapses)
    if (synapse->sourceNeuronId == neuronId)
      result->push_back(synapse);
  for (IncomingSynapse* synapse : *reverseSynapses)
    if (synapse->sourceNeuronId == neuronId)
      result->push_back(synapse);

  return result;
}



void Network::registerSignalForDelivery(
  SynapticSignal* signal, int distanceToDest)
{
  int queueNumber = (currentQueueNumber + distanceToDest) %
    synapticSignalsQueues.size();

  synapticSignalsQueues.at(queueNumber).push_back(signal);
}


vector<bool>*
Network::returnOutputData()
{
  vector<bool>* result = new vector<bool>();

  if (!outNeurons) return nullptr;
  
  for (Neuron* neuron : *outNeurons)
  {
    if (neuron->isActive) result->push_back(true);
    else result->push_back(false);

    neuron->isActive = false;
  }

  return result;
}


void 
Network::addNeuronIdToActive(int newId)
{
  for (int id : *activeNeuronsId)
    if (id == newId) return;

  activeNeuronsId->push_back(newId);
}


void 
Network::clearCurrentlyActiveNeurons()
{
  for (int neuronId : *activeNeuronsId)
    neuronsInRow->at(neuronId)->flushCharges();

  activeNeuronsId->clear();
}


void 
Neuron::flushCharges()
{
  for (float& charge : *neuronCharges)
    charge = 0;
}


void 
Network::flushNetwork()
{
  activeNeuronsId->clear();

  currentQueueNumber = 0;

  for (vector<SynapticSignal*>& signalsRow : synapticSignalsQueues)
    signalsRow.clear();

  for (Neuron* neuron : *neuronsInRow)
  {
    if (neuron) neuron->isActive = false;
    for (IncomingSynapse* synapse : *neuron->straightSynapses)
      synapse->isActive = false;
  }
}


Network* 
Network::getNetworkCopy()
{
  Network* resultNetwork = new Network(parameters);

  resultNetwork->inputSignals = getInputSignalsCopy(resultNetwork);

  resultNetwork->neuronsInRow = getNeuronsInLineCopy(resultNetwork);
  resultNetwork->splitNeuronsToLayers();

  resultNetwork->outNeurons = getOutputNeurons();

  resultNetwork->isNetworkBuilding = this->isNetworkBuilding;

  return resultNetwork;
}


vector<InputSignal*>*
Network::getInputSignalsCopy(Network* net)
{
  vector<InputSignal*>* newInputSignals = new
    vector<InputSignal*>();

  InputSignal* newSignal;
  OutcomingSynapse* outSynapse;
  for (InputSignal* signal : *inputSignals)
  {
    newSignal = new InputSignal(signal->id, net);

    for (OutcomingSynapse* protOutSynapse : *signal->outcomingSynapses)
    {
      outSynapse = new OutcomingSynapse();
      outSynapse->destNeuronId = protOutSynapse->destNeuronId;
      outSynapse->distanceToDest = protOutSynapse->distanceToDest;

      newSignal->outcomingSynapses->push_back(outSynapse);
    }

    newInputSignals->push_back(newSignal);
  }

  return newInputSignals;
}



vector<Neuron*>*
Network::getNeuronsInLineCopy(Network* newNetwork)
{
  vector<Neuron*>* newNeurons = new
    vector<Neuron*>();

  Neuron* newNeuron;
  Neuron* neuronProt;
  for (int i = 0; i < neuronsInRow->size(); i++)
  {
    neuronProt = neuronsInRow->at(i);
    newNeuron = new Neuron(neuronProt->id, neuronProt->networkLayer, newNetwork);

    // copy out synapses
    OutcomingSynapse* newOutSynapse;
    for (OutcomingSynapse* synapse : *neuronProt->strOutcomingSynapses)
    {
      newOutSynapse = new OutcomingSynapse();
      newOutSynapse->destNeuronId = synapse->destNeuronId;
      newOutSynapse->distanceToDest = synapse->distanceToDest;
      newOutSynapse->isWorking = synapse->isWorking;

      newNeuron->strOutcomingSynapses->push_back(newOutSynapse);
    }
    for (OutcomingSynapse* synapse : *neuronProt->sideOutcomingSynapses)
    {
      newOutSynapse = new OutcomingSynapse();
      newOutSynapse->destNeuronId = synapse->destNeuronId;
      newOutSynapse->distanceToDest = synapse->distanceToDest;
      newOutSynapse->isWorking = synapse->isWorking;

      newNeuron->sideOutcomingSynapses->push_back(newOutSynapse);
    }

    // copy input synapses
    IncomingSynapse* newSynapse;
    for (IncomingSynapse* synapse : *neuronProt->straightSynapses)
    {
      newSynapse = new IncomingSynapse();

      newSynapse->sourceNeuronId = synapse->sourceNeuronId;
      newSynapse->weight = synapse->weight;
      newSynapse->connectionsNumber = synapse->connectionsNumber;
      newSynapse->neuronChargeType = synapse->neuronChargeType;

      newNeuron->straightSynapses->push_back(newSynapse);
    }
    for (IncomingSynapse* synapse : *neuronProt->reverseSynapses)
    {
      newSynapse = new IncomingSynapse();

      newSynapse->sourceNeuronId = synapse->sourceNeuronId;
      newSynapse->weight = synapse->weight;
      newSynapse->connectionsNumber = synapse->connectionsNumber;
      newSynapse->neuronChargeType = synapse->neuronChargeType;

      newNeuron->reverseSynapses->push_back(newSynapse);
    }

    newNeuron->id = neuronProt->id;
    newNeuron->networkLayer = neuronProt->networkLayer;

    *(newNeuron->neuronCharges) = *(neuronProt->neuronCharges);
    *(newNeuron->chargesContributions) = *(neuronProt->chargesContributions);

    newNeurons->push_back(newNeuron);
  }

  return newNeurons;
}


vector<Neuron*>*
Network::getOutputNeurons()
{
  return outNeurons;
}








void 
Neuron::storeRegularities()
{
  for (IncomingSynapse* synapse : *reverseSynapses)
  {
    if (synapse->isActive)
    {
      synapse->weight += network.parameters->
        getIncreasedSynapticWeight(this->networkLayer, synapse->weight);
      synapse->isActive = false;
    }
  }
  for (IncomingSynapse* synapse : *straightSynapses)
    synapse->isActive = false;
}



vector<Neuron*>* 
Network::getActiveNeurons()
{
  vector<Neuron*>* neurons = new vector<Neuron*>();

  for (SynapticSignal* signal : synapticSignalsQueues.at(currentQueueNumber))
    neurons->push_back(neuronsInRow->at(signal->sourceNeuronId));

  //for (int iNeuronId = 0; iNeuronId < activeNeuronsId->size(); iNeuronId++)
  //  if (neuronsInRow->at(activeNeuronsId->at(iNeuronId))->isActive)
  //    neurons->push_back(neuronsInRow->at(activeNeuronsId->at(iNeuronId)));
  return neurons;
}


Network::Network(NetworkParameters* parameters)
{
  this->parameters = parameters;

  inputSignals    = new vector<InputSignal*>();
  neurons         = new vector<vector<Neuron*>*>();
  outNeurons      = new vector<Neuron*>();
  activeNeuronsId = new vector<int>();
  neurons         = new vector<vector<Neuron*>*>();
  neuronsInRow   = new vector<Neuron*>();


  for (size_t i = 0; i < parameters->networkPeriod + 1; i++)
    synapticSignalsQueues.push_back(*(new vector<SynapticSignal*>()));

  for (int id = 0; id < parameters->inputNeuronsNumber; id++)
    inputSignals->push_back(new InputSignal(id, this));
}



InputSignal::InputSignal(int id, Network* network,
  vector<OutcomingSynapse*>* outcomingSynapses) :
  id(id), network(network)
{
  if (!outcomingSynapses)
    this->outcomingSynapses = new vector<OutcomingSynapse*>();
  else
    this->outcomingSynapses = outcomingSynapses;
};



InputSignal* 
InputSignal::getCopy()
{
  InputSignal* newSignal = new InputSignal(id, network);

  OutcomingSynapse* newOutSynapse;
  for (OutcomingSynapse* outSynapse : *outcomingSynapses)
  {
    newOutSynapse = new OutcomingSynapse();
    newOutSynapse->destNeuronId = outSynapse->destNeuronId;
    newOutSynapse->distanceToDest = outSynapse->distanceToDest;

    newSignal->outcomingSynapses->push_back(newOutSynapse);
  }

  return newSignal;
}


Neuron::Neuron(int id, int networkLayer, Network* network) :
  id(id), networkLayer(networkLayer), network(*network)
{
  strOutcomingSynapses = new vector<OutcomingSynapse*>();
  sideOutcomingSynapses = new vector<OutcomingSynapse*>();
  straightSynapses = new vector<IncomingSynapse*>();
  reverseSynapses = new vector<IncomingSynapse*>();

  activationThreshold = network->parameters->activationThreshold;

  neuronCharges = new vector<float>();
  chargesContributions = new vector<float>();
}


Neuron::Neuron(int id, int networkLayer, Network* network,
  vector<IncomingSynapse*>* straightSynapses,
  vector<OutcomingSynapse*>* outcomingSynapses) :
  id(id), networkLayer(networkLayer), network(*network)
{
  strOutcomingSynapses = new vector<OutcomingSynapse*>();
  sideOutcomingSynapses = new vector<OutcomingSynapse*>();
  //this->outcomingSynapses = outcomingSynapses;

  if (!straightSynapses)
    straightSynapses = new vector<IncomingSynapse*>();
  this->straightSynapses = straightSynapses;

  activationThreshold = network->parameters->activationThreshold;
}



void
Network::splitNeuronsToLayers()
{
  for (Neuron* neuron : *neuronsInRow)
  {
    if (neurons->size() <= neuron->networkLayer)
      for (int i = neurons->size(); i < neuron->networkLayer + 1; i++)
        neurons->push_back(new vector<Neuron*>());
    
    neurons->at(neuron->networkLayer)->
      push_back(neuron);
  }
}


void 
Network::mergeNeuronsToLine()
{
  if (!neuronsInRow->empty())
    neuronsInRow->clear();

  for (vector<Neuron*>* layer : *neurons)
    for (Neuron* neuron : *layer)
      neuronsInRow->push_back(neuron);
}


int 
Network::takeLastNeuronId()
{
  return lastNeuronId++;
}


void
Network::makeNeuronsNotActive()
{
  for (Neuron* neuron : *neuronsInRow)
    neuron->isActive = false;
}


IncomingSynapse* 
Neuron::getInSynapseFrom(int id)
{
  for (IncomingSynapse* synapse : *reverseSynapses)
    if (synapse->sourceNeuronId == id)
      return synapse;

  return nullptr;
}



void 
Network::storeRegularities()
{
  int nextQueueNumber = (currentQueueNumber + 1) % synapticSignalsQueues.size();

  if (synapticSignalsQueues.at(currentQueueNumber).empty() ||
    synapticSignalsQueues.at(nextQueueNumber).empty())
      return;

  int netWorkLayer = getNeuronLayer(
    synapticSignalsQueues.at(nextQueueNumber).at(0)->sourceNeuronId);
    
  vector<Neuron*>* preNeurons = new vector<Neuron*>();
  for (SynapticSignal* signal : synapticSignalsQueues.at(currentQueueNumber))
    if (netWorkLayer == neuronsInRow->at(signal->sourceNeuronId)->networkLayer)
      preNeurons->push_back(neuronsInRow->at(signal->sourceNeuronId));

  vector<Neuron*>* postNeurons = new vector<Neuron*>();
  for (SynapticSignal* signal : synapticSignalsQueues.at(nextQueueNumber))
    postNeurons->push_back(neuronsInRow->at(signal->sourceNeuronId));
  
  strengthenLinkBetweenGroups(preNeurons, postNeurons);
}



void 
Network::strengthenLinkBetweenGroups(vector<Neuron*>* preNeurons, vector<Neuron*>* postNeurons)
{
  if (preNeurons->empty()) return;
  
    IncomingSynapse* straightSynapses;
  vector<IncomingSynapse*>* sideSynapses;

  IncomingSynapse* inSynapse;
  float newContribution;

  Neuron* sourceNeuron;
  for (Neuron* destNeuron : *postNeurons)
  {
    for (int iType = 0; iType < destNeuron->straightSynapses->size(); iType++)
    {
      sideSynapses = findSideSynapses(preNeurons, destNeuron, iType);
      inSynapse = getRandomInSynapseFromGroup(sideSynapses);

      if (inSynapse)
      {
        strengthenSynapse(destNeuron, inSynapse);
      }
      else
      {
        sourceNeuron = getRandomNeuronFromGroup(preNeurons);
        buildSideSynapse(sourceNeuron, destNeuron, iType);
      }
    }
  }
}



Neuron*
Network::getRandomNeuronFromGroup(vector<Neuron*>* neuronsGroup)
{
  if (neuronsGroup->empty()) return nullptr;

  int rand = std::rand() % neuronsGroup->size();
  return neuronsGroup->at(rand);
}



OutcomingSynapse*
Network::buildSideSynapse(Neuron* source, Neuron* dest, int type)
{
  OutcomingSynapse* synapseBegin = new OutcomingSynapse();
  synapseBegin->destNeuronId = dest->id;
  synapseBegin->distanceToDest = dest->network.parameters->networkPeriod;

  source->sideOutcomingSynapses->push_back(synapseBegin);

  IncomingSynapse* synapseEnd = new IncomingSynapse();
  synapseEnd->connectionsNumber = 1;
  synapseEnd->sourceNeuronId = source->id;
  synapseEnd->weight = parameters->initialSynapseWeight;
  synapseEnd->neuronChargeType = type;

  dest->reverseSynapses->push_back(synapseEnd);

  return synapseBegin;
}




IncomingSynapse*
Network::getRandomInSynapseFromGroup(vector<IncomingSynapse*>* synapses)
{
  if (synapses->empty()) return nullptr;

  int rand = std::rand() % synapses->size();
  return synapses->at(rand);
}



OutcomingSynapse*
Network::findOutSynapseByInSynapse(
vector<Neuron*>* neurons, IncomingSynapse* inSynapse, int destNeuronId)
{
  for (Neuron* neuron : *neurons)
    if (neuron->id == inSynapse->sourceNeuronId)
      for (OutcomingSynapse* outSynapse : *neuron->sideOutcomingSynapses)
        if (outSynapse->destNeuronId == destNeuronId) return outSynapse;
  return nullptr;
}


void
Network::strengthenSynapse(Neuron* postNeuron, IncomingSynapse* synapse)
{
  synapse->weight += parameters->getIncreasedSynapticWeight(
    postNeuron->networkLayer, synapse->weight);
}



vector<IncomingSynapse*>*
Network::findSideSynapses(
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
Network::isIdInArray(vector<Neuron*>* neurons, int id)
{
  for (Neuron* neuron : *neurons)
    if (neuron->id == id) return true;

  return false;
}


void 
Network::strengthenLinkBetweenNeurons(Neuron* preNeuron, Neuron* postNeuron)
{
  IncomingSynapse* synapse = postNeuron->findSideSynapseByNeuronId(preNeuron->id);

  if (synapse)
    synapse->weight += parameters->getIncreasedSynapticWeight(
      postNeuron->networkLayer, synapse->weight);
  else
    addSideSynapse(preNeuron, postNeuron);
}




void
Network::addSideSynapse(Neuron* preNeuron, Neuron* postNeuron)
{
  OutcomingSynapse* synapseBegin = new OutcomingSynapse();
  synapseBegin->destNeuronId = postNeuron->id;
  synapseBegin->distanceToDest = parameters->networkPeriod;
  preNeuron->sideOutcomingSynapses->push_back(synapseBegin);

  IncomingSynapse* synapseEnd = new IncomingSynapse();
  synapseEnd->connectionsNumber = 1;
  synapseEnd->weight = parameters->initialSynapseWeight;
  synapseEnd->sourceNeuronId = preNeuron->id;
  synapseEnd->neuronChargeType = !postNeuron->reverseSynapses->empty() ?
    (postNeuron->reverseSynapses->back()->neuronChargeType + 1) % 
    postNeuron->straightSynapses->size() : 0;

  postNeuron->reverseSynapses->push_back(synapseEnd);
}



int 
Network::getNeuronLayer(int neuronId)
{
  for (int iLayer = 0; iLayer < neurons->size(); iLayer++)
    if (neuronId <= neurons->at(iLayer)->back()->id)
      return iLayer;
  return 0;
}








