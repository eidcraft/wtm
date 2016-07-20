
#pragma once

#include "WtmResources.h"

class WtmVirtualNetwork
{
public:
  WtmVirtualNetwork(int id, WtmSourceData* sourceDataUnit) :
    id(id), sourceDataUnit(sourceDataUnit) {};

  int id;

  static vector<WtmVirtualNetwork*>* createNetworks(
    vector<WtmSourceData*>* motorBasisSourceData, vector<WtmSourceData*>* notMotorBasisSourceData);

  WtmSourceData* sourceDataUnit;

private:

};

