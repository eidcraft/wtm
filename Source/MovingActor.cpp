
#include "stdafx.h"
#include <time.h>
#include "MovingActor.h"


MovingActor::MovingActor(Directions dir)
{
  this->dir = dir;

  bodyCoords = new vector<Coords>(3);
  bodyScreenCoords = new vector<vector<float>*>();
  for (int i = 0; i < 3; i++)
    bodyScreenCoords->push_back(new vector<float>(2));
}



void
MovingActor::setCoords(Coords newCoords)
{
  for (Coords& coords : *bodyCoords)
    coords = newCoords;
}



MovingActor::~MovingActor()
{
}



void 
MovingActor::registerWorldTasks(World* world)
{


}


void 
MovingActor::actionMoveForward()
{
  Coords forwardCoords = getNextCoords(bodyCoords->at(0), actorDirection.direction);

  if (world->getCoordType(forwardCoords) == 0)
    makeActorStep(forwardCoords);
}


void
MovingActor::makeActorStep(Coords coords)
{
  bodyCoords->at(2) = bodyCoords->at(1);
  bodyCoords->at(1) = bodyCoords->at(0);
  bodyCoords->at(0) = coords;
}


void 
MovingActor::actionTurnLeft()
{
  actorDirection = actorDirection.turnDirectionLeft(actorDirection);
}


void 
MovingActor::actionTurnRight()
{
  actorDirection = actorDirection.turnDirectionRight(actorDirection);
}


void 
MovingActor::updateLogic()
{
  if (motionStage != -1)
  {
    actionMoveForward();
  }
  else if (motionStage == 7)
  {
    motionStage = 0;
    actionTurnRight();
    actionTurnRight();
  }

  motionStage++;
  //updateView();

  //actionTurnRight();
  //actionTurnLeft();
  //stageIncrease();
}


void
MovingActor::stageIncrease()
{
  motionStage = (motionStage + 1) % 5;
}


void 
MovingActor::updateView()
{
  for (int i = 0; i < bodyScreenCoords->size(); i++)
  {
    bodyScreenCoords->at(i)->at(0) = (world->blockSizeInPercents + world->paddingInPercents) *
      (1.f / 2.f + bodyCoords->at(i).x);
    bodyScreenCoords->at(i)->at(1) = (world->blockSizeInPercents + world->paddingInPercents) *
      (1.f / 2.f + bodyCoords->at(i).y);
  }
  //bodyScreenCoords->at(0)->at(0) = (world->blockSizeInPercents + world->paddingInPercents) *
  //  (1.f / 2.f + bodyCoords->at(0).x);
  //bodyScreenCoords->at(0)->at(1) = (world->blockSizeInPercents + world->paddingInPercents) *
  //  (1.f / 2.f + bodyCoords->at(0).y);

  //screenXInPercents = (world->blockSizeInPercents + world->paddingInPercents) *
  //  (1.f / 2.f + actorsCoords.x);
  //screenYInPercents = (world->blockSizeInPercents + world->paddingInPercents) *
  //  (1.f / 2.f + actorsCoords.y);

  updatePawnView();
}


void 
MovingActor::updatePawnView()
{

  glColor3f(0.2f, 0.2, 0.6);

  for (int i = 0; i != bodyCoords->size(); i++)
  {
    glPushMatrix();

    glTranslatef(bodyScreenCoords->at(i)->at(0), bodyScreenCoords->at(i)->at(1), 0.f);
    gluDisk(gluNewQuadric(), 0., 0.4 * world->blockSizeInPercents, 64, 1);

    glPopMatrix();
  }
}


Coords 
MovingActor::getNextCoords(Coords coords, Directions direction)
{
  Coords* forwardCoords = new Coords(coords);

  switch (direction)
  {
  case Top:
    *forwardCoords += Coords(1, 0);
    break;

  case TopRight:
    *forwardCoords += Coords(1, 1);
    break;

  case Right:
    *forwardCoords += Coords(0, 1);
    break;

  case DownRight:
    *forwardCoords += Coords(-1, 1);
    break;

  case Down:
    *forwardCoords += Coords(-1, 0);
    break;

  case DownLeft:
    *forwardCoords += Coords(-1, -1);
    break;

  case Left:
    *forwardCoords += Coords(0, -1);
    break;

  case TopLeft:
    *forwardCoords += Coords(1, -1);
    break;

  default:
    break;
  }

  return *forwardCoords;
}




bool
MovingActor::isActorTakePlace(Coords coords)
{
  for (Coords bCoords : *bodyCoords)
    if (bCoords == coords)
      return true;
  return false;
}

