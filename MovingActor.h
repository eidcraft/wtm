
#pragma once

#include "World.h"

class MovingActor :
  public Actor
{
public:
  MovingActor(Directions dir);
  ~MovingActor();

  Directions dir;

  int motionStage;
  void stageIncrease();

  void registerWorldTasks(World* world);

  void updateLogic();
  void updateView();
  void updatePawnView();

  vector<Coords>* bodyCoords;
  vector<vector<float>*>* bodyScreenCoords;

  float screenXInPercents;
  float screenYInPercents;

  /*-------------------------------------------------*/

  Coords getNextCoords(Coords coords, Directions direction);
  void setCoords(Coords coords);
  bool isActorTakePlace(Coords coords);


  void makeActorStep(Coords coords);

  void actionMoveForward();
  void actionTurnLeft();
  void actionTurnRight();

};

