#include "stdafx.h"
#include "WtmVirtualNetwork.h"


vector<WtmVirtualNetwork*>* 
WtmVirtualNetwork::createNetworks(vector<WtmSourceData*>* sourceData)
{
  vector<WtmVirtualNetwork*>* result = new vector<WtmVirtualNetwork*>();
  
  for (int i = 0; i != sourceData->size();i ++)
    result->push_back(new WtmVirtualNetwork(i, sourceData->at(i)));
  
  return result;
}
