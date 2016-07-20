
#pragma once

#include <vector>
#include <string>
using std::vector; 
using std::string;

#include "WtmVirtualNetwork.h"
#include "WtmNetwork.h"
#include "WtmResources.h"

class WtmCreator
{
public:
  WtmCreator(WtmParameters* wtmParameters, 
    vector<WtmSourceData*>* motorBasis, 
    vector<WtmSourceData*>* basicBegavior = nullptr);

  WtmNetwork* create();

  WtmParameters* parameters;

//private:
  vector<WtmVirtualNetwork*>* virtualNetworks;

  void creteIncomingSignals();

  vector<WtmNetwork::InputSignal*> inputSignals;
  vector<vector<WtmNetwork::InputSignal*>*>* intersectingInputSignalsGroups;

  vector<vector<WtmNetwork::InputSignal*>*> inputSignalsByVNet;
  void aggregateSignalsByVNet();

  vector<bool>* getIntersectingInputData(vector<WtmVirtualNetwork*>* participants);
     
  int getNewSignalObjId();

  int lastSignalObjId;

  /*----------------------------------------------------------------------------------*/

  vector<vector<int>*>* getSourceDataIntersections(
    vector<WtmVirtualNetwork*>* virtualNetworks);

  void getSourceDataIntersectionsGroups();

  int getTwoDataUnitIntersection(vector<bool>* dataOne, vector<bool>* dataTwo);
  void extrapolateDataIntersections(vector<vector<int>*>* sourceDataIntersections);

  void realizeMotorBasis();

  /*----------------------------------------------------------------------------------*/

  typedef vector<WtmVirtualNetwork*> IntersectingVNetworks;

  vector<vector<vector<int>*>*> vNetworksIntersections;

  vector<IntersectingVNetworks*>* intersectionsSolveSequence;

  void prepareVNetwroksIntersecion(int virtualNetworkNumber);

  vector<IntersectingVNetworks*>* getIntersectionsSolveSteps(int n, int k);
  vector<IntersectingVNetworks*>* getVNetworksByNumbers(vector<vector<int>*>* numbers);

  vector<vector<int>*>* getIntersectionsDiff(vector<vector<int>*>*  intersections);

  /*---------------------------------------------------------------------------------*/

  typedef int NeuronsGroupSize;

  vector<vector<NeuronsGroupSize>*>* vNetsIntersectionSizes;

  void getIntersectionGroupsSizesForNet();
  vector<NeuronsGroupSize>* getIntersectionGroupsSizesForLayer(int layer);

  int findMinIntersection(vector<vector<int>*>* intersections, 
    vector<WtmVirtualNetwork*>* networks);
  void subtractFoundedMin(vector<vector<int>*>* intersections,
    vector<WtmVirtualNetwork*>* networks, int minPerStep);

  /*---------------------------------------------------------------------------------*/

  void createVNetIntersectingNeurons();
  void createVNetUniqueNeurons();
  void createVNetOutNeurons();

  typedef vector<WtmNetwork::Neuron*> NeuronsGroup;
  vector<vector<NeuronsGroup*>*>* vNetsIntersectionNeurons;
  vector<vector<NeuronsGroup*>*>* vNetsUniqueNeurons;

  vector<WtmNetwork::Neuron*> neurons;
  vector<vector<NeuronsGroup*>*>* neuronsByVNet;
  vector<WtmNetwork::Neuron*>* outNeurons;

  NeuronsGroup* createNeurons(int number, int layer);
  WtmNetwork::Neuron* createNewNeuron(int layer);

  void aggregateNeurons();
  void aggregateNeuronsByVNet();
  
  void addIntersectingNeuronsToVNets(NeuronsGroup* neurons, 
    IntersectingVNetworks* networks, int layer);
  void addUniqueNeuronsToVNets(NeuronsGroup* neurons,
    int vNetworkId, int layer);

  WtmNetwork* network;

  /*---------------------------------------------------------------------------------*/

  void createSynapses();

  void createIntersectingNeuronsSynapses();
  void createUniqueNeuronsSynapses();

  void createSynapsesFromTo(WtmCreator::NeuronsGroup* sources,
    WtmCreator::NeuronsGroup* dests);
  void createSynapsesFromTo(
    vector<WtmNetwork::InputSignal*>* sources, WtmCreator::NeuronsGroup* dests);

  void createSynapseFromTo(WtmNetwork::Neuron* source, WtmNetwork::Neuron* dest);
  void createSynapseFromTo(WtmNetwork::InputSignal* sources, WtmNetwork::Neuron* dest);

  WtmCreator::NeuronsGroup* getVNetLayerNeurons(int vNetNumber, int layer);

};

