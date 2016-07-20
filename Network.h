
#pragma once

#include <vector>
using std::vector;

#include "Resources.h"

struct IncomingSynapse;
struct OutcomingSynapse;
struct SynapticSignal;

class NetworkJsonLoader;
class InputSignal;
class Network;
class Neuron;


struct SynapticSignal
{
  SynapticSignal(int id) : sourceNeuronId(id) 
  {
    destNeuronsIds = new vector<int>();
  }
  int sourceNeuronId;
  vector<int>* destNeuronsIds;
};

struct IncomingSynapse
{
  int sourceNeuronId;
  float weight;
  bool isActive;

  int connectionsNumber;
  int neuronChargeType;
};


struct OutcomingSynapse
{
  int destNeuronId;
  int distanceToDest;
  bool isWorking = false;
};


class InputSignal
{
public:
  InputSignal(int id, Network* network, vector<OutcomingSynapse*>* outcomingSynapses = nullptr);

  int id;
  vector<OutcomingSynapse*>* outcomingSynapses;
  bool isActive;
  
  Network* network;

  void sendSignals();
  InputSignal* getCopy();
};


class Neuron
{
public:
  Neuron(int id, int networkLayer, Network* network);

  Neuron(int id, int networkLayer, Network* network,
    vector<IncomingSynapse*>* straightSynapses,
    vector<OutcomingSynapse*>* outcomingSynapses);
  
  /*------------------------------------------------*/

  int id;
  int networkLayer;
  int lastUpdateTime;

  Network& network;

  /*------------------------------------------------*/

  vector<float>* neuronCharges;
  vector<float>* chargesContributions;
  bool isActive;
  float activationThreshold;
  bool isThresholdExceeded();

  void flushCharges();
  /*------------------------------------------------*/

  float fatigue;
  float restSpeed;
  float fatigueSpeed;
  bool isAbleToTakeSignals();

  /*------------------------------------------------*/

  void updateParameters(int updateLag);
  void processActivation();
  
  /*------------------------------------------------*/

  vector<IncomingSynapse*>*  straightSynapses;
  vector<IncomingSynapse*>*  reverseSynapses;

  void receiveSignals();


  IncomingSynapse* findSideSynapseByNeuronId(int neuronId);
  IncomingSynapse* findStrSynapseByNeuronId(int neuronId);
  vector<IncomingSynapse*>* findSynapses(int neuronId);
  
  void registerIncomingSignal(int sourceNeuronId);

  void resetSynapsesActivityFlag();

  /*------------------------------------------------*/

  vector<OutcomingSynapse*>* strOutcomingSynapses;
  vector<OutcomingSynapse*>* sideOutcomingSynapses;

  void sendSignals();

  IncomingSynapse* getInSynapseFrom(int id);
  /*------------------------------------------------*/
  
  void storeRegularities();


  /*------------------------------------------------*/

private:

};


class Network
{
public:

  friend NetworkJsonLoader;

  Network(NetworkParameters* parameters);
  void flushNetwork();

  Network* getNetworkCopy();

  /*------------------------------------------------*/

  void installInputData(vector<bool>& inputData);
  void makeProcessingTact();
  void makeProcessingPeriod();
  vector<bool>* returnOutputData();

  //private:

  int lastUpdateTime;
  NetworkParameters* parameters;
  bool isNetworkBuilding;

  /*------------------------------------------------*/

  vector<InputSignal*>* inputSignals;
  vector<vector<Neuron*>*>* neurons;
  vector<Neuron*>* neuronsInRow;
  vector<Neuron*>* outNeurons;
  vector<int>* activeNeuronsId;

  void processActiveNeurons();
  void processInputNeuronsActivation();
  void processNeuron(Neuron* neuron);

  void addNeuronIdToActive(int neuronId);

  void makeNeuronsNotActive();
  void clearCurrentlyActiveNeurons();

  /*------------------------------------------------*/

  vector< vector< SynapticSignal* > > synapticSignalsQueues;

  int currentQueueNumber;

  void registerSignalForDelivery(SynapticSignal* signal, int distanceToDest);
  void deliverSignals();

  void refreshQueues();
  void clearLastQueue();
  void makeQueuesCircleSwap();

  /*---------------------------------------------------------*/


  vector<InputSignal*>* getInputSignalsCopy(Network* newNet);
  vector<Neuron*>* getNeuronsInLineCopy(Network* newNet);
  
  vector<Neuron*>* getOutputNeurons();
  vector<Neuron*>* getActiveNeurons();
  Neuron* getNeuronById(int id);
  int takeLastNeuronId();
  int lastNeuronId;

  void splitNeuronsToLayers();
  void mergeNeuronsToLine();
  void makeNeuronsNotActive(Network* net);

  void storeRegularities();
  void strengthenLinkBetweenGroups(vector<Neuron*>* preNeurons, vector<Neuron*>* postNeurons);
  void strengthenLinkBetweenNeurons(Neuron* preNeuron, Neuron* postNeuron);
  void strengthenSynapse(Neuron* postNeuron, IncomingSynapse* synapse);
  void addSideSynapse(Neuron* preNeuron, Neuron* postNeuron);

  vector<IncomingSynapse*>* findSideSynapses(vector<Neuron*>* sources, Neuron* dest, int type);
  IncomingSynapse* getRandomInSynapseFromGroup(vector<IncomingSynapse*>* synapses);
  bool isIdInArray(vector<Neuron*>* neurons, int id);
  OutcomingSynapse* buildSideSynapse(Neuron* source, Neuron* dest, int type);
  Neuron* getRandomNeuronFromGroup(vector<Neuron*>* neuronsGroup);
  OutcomingSynapse* findOutSynapseByInSynapse(
    vector<Neuron*>* neurons, IncomingSynapse* inSynapse, int destNeuronId);

  int getNeuronLayer(int neuronId);
};