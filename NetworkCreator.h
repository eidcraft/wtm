
#pragma once

#include <vector>
#include <string>
using std::vector; 
using std::string;

#include "Network.h"
#include "Resources.h"

class NetworkCreator
{
public:

  static Network* create(WtmSourceDataSet* wtmSourceDataSet);

private:

  static void buildFirstLayer(Network* net);
  static void buildMiddleLayers(Network* net, WtmSourceDataSet* networkSourceData);
  static void buildLastLayer(Network* net, WtmSourceDataSet* networkSourceData);

  static void buildInitialBehavior(Network* net, WtmSourceDataSet* networkSourceData);

  static void calcSideSynapsesContribution(Network* net);
  static void calcStraigtSynapsesContribution(Network* net);

  static Neuron* createNeuronInLayer(Network* net, int layer);

  static void buildSynapsesFromTo(Network* net, vector<InputSignal*>* sources, Neuron* dest);
  static void buildSynapseFromTo(Network* net, InputSignal* source, Neuron* dest);

  static void buildSynapsesFromTo(Network* net, vector<Neuron*>* sources, Neuron* dest);
  static void buildSynapseFromTo(Network* net, Neuron* source, Neuron* dest);

  static void buildSideSynapses(Network* net);
  static OutcomingSynapse* buildSideSynapse(Network* net, Neuron* source, Neuron* dest, int type);

  static void createAssosiation(Network* net, vector<Neuron*>* sources, vector<Neuron*>* dests);

  static vector<IncomingSynapse*>*findSideSynapses(
    vector<Neuron*>* sources, Neuron* dest, int type);
  static bool isIdInArray(vector<Neuron*>* neurons, int id);

  static Neuron* getRandomNeuronFromGroup(vector<Neuron*>* neuronsGroup);
  static IncomingSynapse* getRandomInSynapseFromGroup(vector<IncomingSynapse*>* synapses);

  static OutcomingSynapse* findOutSynapseByInSynapse(
    vector<Neuron*>* neurons, IncomingSynapse* inSynapse, int destNeuronId);
};

