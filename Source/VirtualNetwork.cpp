#include "stdafx.h"
#include "WtmVirtualNetwork.h"


vector<WtmVirtualNetwork*>* 
WtmVirtualNetwork::createNetworks(vector<WtmSourceData*>* motorBasisSourceData, 
vector<WtmSourceData*>* notMotorBasisSourceData)
{
  vector<WtmVirtualNetwork*>* result = new vector<WtmVirtualNetwork*>();
  
  for (int i = 0; i != motorBasisSourceData->size(); i++)
    result->push_back(new WtmVirtualNetwork(i, motorBasisSourceData->at(i)));
  for (int i = 0; i != notMotorBasisSourceData->size(); i++)
    result->push_back(new WtmVirtualNetwork(i, notMotorBasisSourceData->at(i)));

  return result;
}
