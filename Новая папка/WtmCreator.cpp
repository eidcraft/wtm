#include "stdafx.h"
#include "WtmCreator.h"
#include "math.h"

WtmCreator::WtmCreator(WtmParameters* parameters,
  vector<WtmSourceData*>* motorBasis,
  vector<WtmSourceData*>* basicBegavior)
{
  this->parameters = parameters;

  virtualNetworks = WtmVirtualNetwork::createNetworks(motorBasis);

  prepareVNetwroksIntersecion(virtualNetworks->size());

  network = new WtmNetwork();
}


WtmNetwork* WtmCreator::create()
{
  realizeMotorBasis();

  // realizeBasicBehavior (associations)

  WtmNetwork* wtmNetwork = new WtmNetwork();

  return wtmNetwork;
}


void 
WtmCreator::realizeMotorBasis()
{
  vector<vector<int>*>* sourceDataIntersections = 
    getSourceDataIntersections(virtualNetworks);

  creteIncomingSignals();
  aggregateSignalsByVNet();

  getSourceDataIntersectionsGroups();

  extrapolateDataIntersections(sourceDataIntersections);

  getIntersectionGroupsSizesForNet();

  createVNetIntersectingNeurons();
  createVNetUniqueNeurons();
  createVNetOutNeurons();

  aggregateNeuronsByVNet();

  createSynapses();

  aggregateNeurons();
}


void
WtmCreator::creteIncomingSignals()
{
  vector<bool>* inputData = 
    virtualNetworks->at(0)->sourceDataUnit->inputData.at(0);

  int signalId;

  for (int i = 0; i < inputData->size(); i++)
  {
    signalId = getNewSignalObjId();

    inputSignals.push_back(
      new WtmNetwork::InputSignal(signalId, network));
  }
}


int
WtmCreator::getNewSignalObjId()
{
  lastSignalObjId += 1;
  return lastSignalObjId;
}


void
WtmCreator::getSourceDataIntersectionsGroups()
{
  vector<bool>* instersectingData;
  vector<WtmNetwork::InputSignal*>* instersectingSignals;

  intersectingInputSignalsGroups = new
    vector<vector<WtmNetwork::InputSignal*>*>(inputSignals.size());

  for (int iSolveStep = 0; iSolveStep < intersectionsSolveSequence->size(); iSolveStep++)
  {
    instersectingData = getIntersectingInputData(intersectionsSolveSequence->at(iSolveStep));
    instersectingSignals = new vector<WtmNetwork::InputSignal*>();

    if (instersectingData->empty()) continue;

    for (int i = 0; i < instersectingData->size(); i++)
    {
      if (instersectingData->at(i))
      {
        instersectingSignals->push_back(inputSignals.at(i));
      }
    }
    intersectingInputSignalsGroups->at(iSolveStep) = instersectingSignals;
  }
}


vector<bool>*
WtmCreator::getIntersectingInputData(vector<WtmVirtualNetwork*>* participants)
{
  vector<bool>* result = nullptr;
  vector<bool>* newData;

  for (int i = 0; i < participants->size(); i++)
  {
    newData = new vector<bool>(
      participants->at(i)->sourceDataUnit->inputData.at(0)->size());
    *newData = *participants->at(i)->sourceDataUnit->inputData.at(0);
    
    if (result && !result->empty())
    {
      result = byteAnd(result, newData);
      continue;
    }

    result = new vector<bool>(newData->size());
    *result = *newData;
  }
  return result;
}


vector<vector<int>*>* 
WtmCreator::getSourceDataIntersections(vector<WtmVirtualNetwork*>* networks)
{
  vector<vector<int>*>* intersections = new vector<vector<int>*>();
  vector<int>* intersectionsRow;

  vector<bool>* dataOne;
  vector<bool>* dataTwo;

  for (int iNetworkOne = 0; iNetworkOne < networks->size(); iNetworkOne++)
  {
    intersectionsRow = new vector<int>(networks->size(), 0);
    
    dataOne = networks->at(iNetworkOne)->sourceDataUnit->inputData.at(0);

    for (int iNetworkTwo = 0; iNetworkTwo < networks->size(); iNetworkTwo++)
    {
      dataTwo = networks->at(iNetworkTwo)->sourceDataUnit->inputData.at(0);
      intersectionsRow->at(iNetworkTwo) = getTwoDataUnitIntersection(dataOne, dataTwo);
    }
    intersections->push_back(intersectionsRow);
  }

  return intersections;
}


int 
WtmCreator::getTwoDataUnitIntersection(vector<bool>* dataOne, vector<bool>* dataTwo)
{
  int counter = 0;

  for (int index = 0; index < dataOne->size(); index++)
    if (dataOne->at(index) == true && dataOne->at(index) == dataTwo->at(index))
      counter++;

  return counter;
}


void 
WtmCreator::extrapolateDataIntersections(vector<vector<int>*>* intersections)
{
  vector<vector<int>*>* intersectionsDiff = getIntersectionsDiff(intersections);

  vector<vector<int>*>* networksIntersections = getArrayCopy(intersections);

  for (int iLayer = 0; iLayer < parameters->layersNumber; iLayer++)
  {
    networksIntersections = absSubtractArrays(networksIntersections, intersectionsDiff);
    vNetworksIntersections.push_back(networksIntersections);
  }
}


vector<vector<int>*>* 
WtmCreator::getIntersectionsDiff(vector<vector<int>*>*  intersections)
{
  vector<vector<int>*>* diff = getArrayCopy(intersections);

  int layersNumber = parameters->layersNumber - 1;

  for (int iRow = 0; iRow < diff->size(); iRow++)
  {
    for (int iUnit = 0; iUnit < diff->size(); iUnit++)
    {
      if (iRow != iUnit)
        diff->at(iRow)->at(iUnit) = ceil(
        (float)diff->at(iRow)->at(iUnit) / layersNumber);
      else
        diff->at(iRow)->at(iUnit) = 0;
    }
  }
  return diff;
}


void 
WtmCreator::prepareVNetwroksIntersecion(int virtualNetworkNumber)
{
  //vector<IntersectingVNetworks*>* resultSteps = 
  //  new vector<IntersectingVNetworks*>();
  intersectionsSolveSequence = new vector<IntersectingVNetworks*>();
  vector<IntersectingVNetworks*>* newSteps;


  for (int intersectingNetworksNumber = virtualNetworkNumber; 
           intersectingNetworksNumber > 1; 
           intersectingNetworksNumber --)
  {
    newSteps = getIntersectionsSolveSteps(
      virtualNetworkNumber, intersectingNetworksNumber);

    intersectionsSolveSequence->insert(intersectionsSolveSequence->end(),
      newSteps->begin(), newSteps->end());
  }

  vNetsIntersectionNeurons = new vector<vector<NeuronsGroup*>*>();

  for (int iLayer = 0; iLayer < parameters->layersNumber; iLayer++)
    vNetsIntersectionNeurons->push_back(new vector<NeuronsGroup*>(
    intersectionsSolveSequence->size()));

}


vector<WtmCreator::IntersectingVNetworks*>* 
WtmCreator::getIntersectionsSolveSteps(int n, int k)
{
  vector<vector<int>*>* networkNumbersCombinations = getReshuffles(n, k);

  vector<IntersectingVNetworks*>* result = 
    getVNetworksByNumbers(networkNumbersCombinations);

  return result;
}


vector<WtmCreator::IntersectingVNetworks*>*
WtmCreator::getVNetworksByNumbers(vector<vector<int>*>* numbers)
{
  vector<IntersectingVNetworks*>* result = new vector<IntersectingVNetworks*>();
  IntersectingVNetworks* resultElem;

  for (int iNumbersRow = 0; iNumbersRow < numbers->size(); iNumbersRow++)
  {
    resultElem = new IntersectingVNetworks();
    for (int iNumber = 0; iNumber < numbers->at(iNumbersRow)->size(); iNumber++)
    {
      resultElem->push_back(virtualNetworks->at(
        numbers->at(iNumbersRow)->at(iNumber) - 1));
    }
    result->push_back(resultElem);
  }

  return result;
}


void 
WtmCreator::getIntersectionGroupsSizesForNet()
{
  vNetsIntersectionSizes = new 
    vector<vector<NeuronsGroupSize>*>();

  for (int iLayer = 0; iLayer < parameters->layersNumber; iLayer++)
  {
    vNetsIntersectionSizes->push_back(getIntersectionGroupsSizesForLayer(iLayer));
  }
}


vector<WtmCreator::NeuronsGroupSize>* 
WtmCreator::getIntersectionGroupsSizesForLayer(int layer)
{
  int minPerSolveStep;
  vector<WtmCreator::NeuronsGroupSize>* result = new vector<WtmCreator::NeuronsGroupSize>();

  vector<vector<int>*>* intersections = vNetworksIntersections.at(layer);

  for (int i = 0; i < intersectionsSolveSequence->size(); i++)
  {
    minPerSolveStep = findMinIntersection(intersections, intersectionsSolveSequence->at(i));
    result->push_back(minPerSolveStep);
    subtractFoundedMin(intersections, intersectionsSolveSequence->at(i), minPerSolveStep);
  }

  return result;
}


int 
WtmCreator::findMinIntersection(
vector<vector<int>*>* intersections, vector<WtmVirtualNetwork*>* networks)
{
  int minPerStep = INT_MAX;
  int minPerElem = INT_MAX;
  int rowToCheck;
  int columnToCheck;

  for (int j = 0; j < networks->size(); j++)
  {
    rowToCheck = networks->at(j)->id;
    minPerElem = INT_MAX;

    //for (int k = 0; k < intersections->at(rowToCheck)->size(); k++)
    for (int k = 0; k < networks->size(); k++)
    {
      columnToCheck = networks->at(k)->id;

      if (columnToCheck == rowToCheck) continue;

      if (intersections->at(rowToCheck)->at(columnToCheck) < minPerElem)
        minPerElem = intersections->at(rowToCheck)->at(columnToCheck);
    }

    if (minPerElem == 0) return 0;

    if (minPerElem < minPerStep) minPerStep = minPerElem;
  }

  return minPerStep;
}


void 
WtmCreator::subtractFoundedMin(vector<vector<int>*>* intersections,
vector<WtmVirtualNetwork*>* networks, int minPerStep)
{
  int rowToCheck;
  int columnToCheck;

  for (int j = 0; j < networks->size(); j++)
  {
    rowToCheck = networks->at(j)->id;

    for (int k = 0; k < networks->size(); k++)
    {
      columnToCheck = networks->at(k)->id;

      if (columnToCheck == rowToCheck) continue;
      intersections->at(rowToCheck)->at(columnToCheck) -= minPerStep;
    }
  }
}


void
WtmCreator::createVNetIntersectingNeurons()
{
  NeuronsGroup* neuronsGroup;

  for (int iLayer = 0; iLayer < parameters->layersNumber; iLayer++)
  {
    for (int iNeuronsGroup = 0; 
             iNeuronsGroup < vNetsIntersectionSizes->size(); 
             iNeuronsGroup ++)
    {
      neuronsGroup = createNeurons(vNetsIntersectionSizes->at(iLayer)->at(iNeuronsGroup), iLayer);

      vNetsIntersectionNeurons->at(iLayer)->at(iNeuronsGroup) = neuronsGroup;

      neurons.insert(neurons.end(), neuronsGroup->begin(), neuronsGroup->end());
    }
  }
}


WtmCreator::NeuronsGroup*
WtmCreator::createNeurons(int number, int layer)
{
  NeuronsGroup* result = new NeuronsGroup();
  
  for (int i = 0; i < number; i++)
    result->push_back(createNewNeuron(layer));
  
  return result;
}


WtmNetwork::Neuron*
WtmCreator::createNewNeuron(int layer)
{
  int neuronId = getNewSignalObjId();
  return new WtmNetwork::Neuron(neuronId, layer, network);
}


void 
WtmCreator::createVNetUniqueNeurons()
{
  vNetsUniqueNeurons = new vector<vector<NeuronsGroup*>*>();
  
  vector<NeuronsGroup*>* neuronsArray;
  NeuronsGroup* neuronsGroup;

  for (int iLayer = 0; iLayer < parameters->layersNumber - 1; iLayer++)
  {
    neuronsArray = new vector<NeuronsGroup*>();
    for (int iGroup = 0; iGroup < virtualNetworks->size(); iGroup++)
    {
      neuronsGroup = new NeuronsGroup();
      for (int iNeuron = 0; iNeuron < parameters->uniqueNeuronsPerVNet; iNeuron++)
      {
        neuronsGroup->push_back(createNewNeuron(iLayer));
      }
      neuronsArray->push_back(neuronsGroup);
    }
    vNetsUniqueNeurons->push_back(neuronsArray);
  }
}


void 
WtmCreator::createSynapses()
{
  createIntersectingNeuronsSynapses();

  createUniqueNeuronsSynapses();
}

void
WtmCreator::createVNetOutNeurons()
{
  outNeurons = createNeurons(
    parameters->outputNeuronNumber, parameters->layersNumber - 1);

  vNetsUniqueNeurons->push_back(new vector<NeuronsGroup*>());

  for (int iNeuron = 0; iNeuron < parameters->outputNeuronNumber; iNeuron++)
  {
    vNetsUniqueNeurons->at(parameters->layersNumber - 1)->push_back(new NeuronsGroup());
    vNetsUniqueNeurons->at(parameters->layersNumber - 1)->at(iNeuron)->
      push_back(outNeurons->at(iNeuron));
  }
}



void
WtmCreator::createIntersectingNeuronsSynapses()
{
  WtmCreator::NeuronsGroup* synapsesSources;
  WtmCreator::NeuronsGroup* synapsesDests;

  for (int iLayer = parameters->layersNumber - 2; iLayer >= 0; iLayer--)
  {
    for (int iGroup = 0; iGroup < vNetsIntersectionNeurons->at(iLayer)->size(); iGroup++)
    {
      synapsesSources = vNetsIntersectionNeurons->at(iLayer)->at(iGroup);
      synapsesDests = vNetsIntersectionNeurons->at(iLayer + 1)->at(iGroup);

      createSynapsesFromTo(synapsesSources, synapsesDests);
    }
  }

  vector<WtmNetwork::InputSignal*>* sources;
  WtmCreator::NeuronsGroup* dests;

  for (int iGroup = 0; iGroup < vNetsIntersectionNeurons->at(0)->size(); iGroup++)
  {
    sources = intersectingInputSignalsGroups->at(iGroup);
    dests = vNetsIntersectionNeurons->at(0)->at(iGroup);

    createSynapsesFromTo(sources, dests);
  }
}


void
WtmCreator::createUniqueNeuronsSynapses()
{
  WtmCreator::NeuronsGroup* synapsesSources;
  WtmCreator::NeuronsGroup* synapsesDests;

  for (int iLayer = parameters->layersNumber - 2; iLayer >= 0; iLayer--)
  {
    for (int iGroup = 0; iGroup < vNetsUniqueNeurons->at(iLayer)->size(); iGroup++)
    {
      synapsesSources = getVNetLayerNeurons(iGroup, iLayer);
      synapsesDests = vNetsUniqueNeurons->at(iLayer + 1)->at(iGroup);

      createSynapsesFromTo(synapsesSources, synapsesDests);
    }
  }

  vector<WtmNetwork::InputSignal*>* sources;
  WtmCreator::NeuronsGroup* dests;

  for (int iGroup = 0; iGroup < vNetsUniqueNeurons->at(0)->size(); iGroup++)
  {
    sources = inputSignalsByVNet.at(iGroup);
    dests = vNetsUniqueNeurons->at(0)->at(iGroup);

    createSynapsesFromTo(sources, dests);
  }
}


WtmCreator::NeuronsGroup*
WtmCreator::getVNetLayerNeurons(int vNetId, int layer)
{
  return neuronsByVNet->at(vNetId)->at(layer);
}


void 
WtmCreator::createSynapsesFromTo(
WtmCreator::NeuronsGroup* sources, WtmCreator::NeuronsGroup* dests)
{
  float nSourceNeurons = sources->size();
  float nDestNeurons = dests->size();

  if (nDestNeurons == 0) return;

  int synapsesRequired = std::ceil(
    parameters->activationThreshold / parameters->averageSynapseWeight);


  int iSynapses;
  vector<int>* synapsesPlacingPlan;

  for (int iDest = 0; iDest < nDestNeurons; iDest++)
  {
    iSynapses = synapsesRequired;

    while (iSynapses > 0)
    {
      if (iSynapses > nSourceNeurons)
      {
        for (int iSource = 0; iSource < nSourceNeurons; iSource++)
        {
          createSynapseFromTo(sources->at(iSource), dests->at(iDest));
        }
        iSynapses -= nSourceNeurons;
      }
      
      else
      {
        synapsesPlacingPlan = getSynapsesPlacingPlan(nSourceNeurons, iSynapses);

        for (int i = 0; i < iSynapses; i++)
        {
          createSynapseFromTo(sources->at(synapsesPlacingPlan->at(i)), dests->at(iDest));
        }

        break;
      }
    }
  }
}


void
WtmCreator::createSynapsesFromTo(
vector<WtmNetwork::InputSignal*>* sources, WtmCreator::NeuronsGroup* dests)
{
  float nSourceNeurons = sources->size();
  float nDestNeurons = dests->size();

  if (nDestNeurons == 0) return;

  int synapsesRequired = std::ceil(
    parameters->activationThreshold / parameters->averageSynapseWeight);


  int iSynapses;
  vector<int>* synapsesPlacingPlan;

  for (int iDest = 0; iDest < nDestNeurons; iDest++)
  {
    iSynapses = synapsesRequired;

    while (iSynapses > 0)
    {
      if (iSynapses >= nSourceNeurons)
      {
        for (int iSource = 0; iSource < nSourceNeurons; iSource++)
        {
          createSynapseFromTo(sources->at(iSource), dests->at(iDest));
        }
        iSynapses -= nSourceNeurons;
      }

      else
      {
        synapsesPlacingPlan = getSynapsesPlacingPlan(nSourceNeurons, iSynapses);

        for (int i = 0; i < iSynapses; i++)
        {
          createSynapseFromTo(sources->at(synapsesPlacingPlan->at(i)), dests->at(iDest));
        }

        break;
      }
    }
  }
}


void
WtmCreator::createSynapseFromTo(
WtmNetwork::Neuron* source, WtmNetwork::Neuron* dest)
{
  OutcomingSynapse* synapseBegin = new OutcomingSynapse();
  synapseBegin->destNeuronId = dest->id;
  synapseBegin->distanceToDest = std::abs(dest->networkLayer - 
    source->networkLayer);
  source->outcomingSynapses->push_back(synapseBegin);
  
  IncomingSynapse* synapseEnd = new IncomingSynapse();
  synapseEnd->sourceNeuronId = source->id;
  synapseEnd->weight = parameters->averageSynapseWeight;
  dest->incomingSynapses->push_back(synapseEnd);
}


void 
WtmCreator::createSynapseFromTo(
WtmNetwork::InputSignal* source, WtmNetwork::Neuron* dest)
{
  OutcomingSynapse* synapseBegin = new OutcomingSynapse();
  synapseBegin->destNeuronId = dest->id;
  synapseBegin->distanceToDest = 1;
  source->outcomingSynapses->push_back(synapseBegin);

  IncomingSynapse* synapseEnd = new IncomingSynapse();
  synapseEnd->sourceNeuronId = source->id;
  synapseEnd->weight = parameters->averageSynapseWeight;
  dest->incomingSynapses->push_back(synapseEnd);
}


void
WtmCreator::aggregateNeuronsByVNet()
{
  neuronsByVNet = new vector<vector<NeuronsGroup*>*>();

  vector<NeuronsGroup*>* vNetLayers;
  NeuronsGroup* vNetNeurons;

  for (int iLayer = 0; iLayer < parameters->layersNumber; iLayer++)
  {
    vNetLayers = new vector<NeuronsGroup*>();
    neuronsByVNet->push_back(vNetLayers);
    for (int iGroup = 0; iGroup < intersectionsSolveSequence->size(); iGroup++)
    {
      vNetNeurons = new NeuronsGroup();
      vNetLayers->push_back(vNetNeurons);
    }
  }

  for (int iLayer = 0; iLayer < parameters->layersNumber; iLayer++)
  {
    for (int iGroup = 0; iGroup < intersectionsSolveSequence->size(); iGroup++)
    {
      if (vNetsIntersectionSizes->at(iLayer)->at(iGroup) != 0)
      {
        addIntersectingNeuronsToVNets(
          vNetsIntersectionNeurons->at(iLayer)->at(iGroup), 
          intersectionsSolveSequence->at(iGroup),
          iLayer);
      }
    }
  }

  for (int iLayer = 0; iLayer < parameters->layersNumber - 1; iLayer++)
  {
    for (int iVNetwork = 0; iVNetwork < virtualNetworks->size(); iVNetwork++)
    {
      addUniqueNeuronsToVNets(
        vNetsUniqueNeurons->at(iLayer)->at(iVNetwork),
        iVNetwork, iLayer);
    }
  }

}


void
WtmCreator::addIntersectingNeuronsToVNets(NeuronsGroup* newNeurons,
IntersectingVNetworks* vNetworks, int layer)
{
  vector<WtmNetwork::Neuron*>* baseNeurons;

  for (WtmVirtualNetwork* vNetwork : *vNetworks)
  {
    baseNeurons = neuronsByVNet->at(vNetwork->id)->at(layer);

    baseNeurons->insert(baseNeurons->end(),
      newNeurons->begin(), newNeurons->end());
  }
}


void
WtmCreator::addUniqueNeuronsToVNets(NeuronsGroup* newNeurons,
int vNetworkId, int layer)
{
  vector<WtmNetwork::Neuron*>* baseNeurons;

  baseNeurons = neuronsByVNet->at(vNetworkId)->at(layer);

  baseNeurons->insert(baseNeurons->end(),
    newNeurons->begin(), newNeurons->end());
}



void
WtmCreator::aggregateSignalsByVNet()
{
  vector<bool>* data;
  vector<WtmNetwork::InputSignal*>* signals;

  int signalId;

  for (int i = 0; i < virtualNetworks->size(); i++)
  {
    data = virtualNetworks->at(i)->sourceDataUnit->inputData.at(0);
    signals = new vector<WtmNetwork::InputSignal*>();

    for (int j = 0; j < data->size(); j++)
    {
      if (data->at(j)) signals->push_back(inputSignals.at(j));
    }
    inputSignalsByVNet.push_back(signals);
  }
}


void 
WtmCreator::aggregateNeurons()
{
  for (vector<NeuronsGroup*>* neuronsLayer : *vNetsUniqueNeurons)
  {
    for (NeuronsGroup* neuronsGroup : *neuronsLayer)
    {
      neurons.insert(neurons.end(),
        neuronsGroup->begin(), neuronsGroup->end());
    }
  }
}

