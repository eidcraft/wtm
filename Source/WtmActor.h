
#pragma once

#include "World.h"
#include "Network.h"


class WtmActor :
  public Actor
{
public:
  WtmActor(Network* network) :
    network(*network) {}
  ~WtmActor() {};

  Network& network;
  void registerWorldTasks(World* world);
  void flushNetworkState();

  /*-------------------------------------------------*/

  void updateLogic();
  void updateView();
  void updatePawnView();
  void updateVisibilityScopeView();

  float screenXInPercents;
  float screenYInPercents;

  /*-------------------------------------------------*/

  void translateNetworkSignalsToActions();

  void actionMoveForward();
  void actionMoveLeft();

  void actionTurnLeft();
  void actionTurnRight();


  /*-------------------------------------------------*/

  Coords getNextCoords(Coords coords, Directions direction);

  /*-------------------------------------------------*/

  int savePeriod = 10;
  int saveCounter;

  void setCoords(Coords coords);
  bool isActorTakePlace(Coords coords);

  void translateWorldDataToNetwork();

  WorldObstaclesMapSegment* getObstaclesMapSegment(vector<Coords>* coords);
  vector<Coords>* getVisibilityScope();
  WorldObstaclesMapSegment* getVisibleMap();
  WorldObstaclesMapSegment* getNetworkData();

  void setWorldDataToNetwork(vector<bool>& worldData);
  
  WorldObstaclesMapSegment*  inverseMapSegment(WorldObstaclesMapSegment* mapSegment);
};
