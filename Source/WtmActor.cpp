
#include "stdafx.h"
#include "WtmActor.h"
#include "JsonDataLoader.h"
#include <stdlib.h>
#include <time.h>


void WtmActor::registerWorldTasks(World* world)
{
}


WorldObstaclesMapSegment* WtmActor::getNetworkData()
{
  WorldObstaclesMapSegment* mapSegment =
    getObstaclesMapSegment(getVisibilityScope());

  WorldObstaclesMapSegment* resultMap = new WorldObstaclesMapSegment();

  *resultMap->coords = *mapSegment->coords;
  *resultMap->coordsAccessabily = *mapSegment->coordsAccessabily;
  
  return resultMap;
}


void WtmActor::translateWorldDataToNetwork()
{

  WorldObstaclesMapSegment* mapSegment = getNetworkData();
  WorldObstaclesMapSegment* inversedMapSegment = 
    inverseMapSegment(mapSegment);

  setWorldDataToNetwork(*inversedMapSegment->coordsAccessabily);
}


WorldObstaclesMapSegment*  
WtmActor::inverseMapSegment(WorldObstaclesMapSegment* baseMap)
{
  WorldObstaclesMapSegment* inversedMap = new WorldObstaclesMapSegment();

  for (int i = 0; i != baseMap->coords->size(); i++)
  {
    inversedMap->coords->push_back(
      baseMap->coords->at(i));
    inversedMap->coordsAccessabily->push_back(
      ! baseMap->coordsAccessabily->at(i));
  }

  return inversedMap;
}


void WtmActor::translateNetworkSignalsToActions()
{
  vector<bool>* outputData;

  outputData = network.returnOutputData();
  
  if (!outputData) outputData = new vector<bool>(3, false);
  
  if (outputData->at(0))
  {
    //actionTurnLeft();
    actionMoveLeft();
    std::cout << "actionTurnLeft" << std::endl;
  }
  if (outputData->at(1))
  {
    actionMoveForward();
    std::cout << "actionTurnLeft" << std::endl;
  }
  //if (outputData->at(2)) actionTurnRight();

  //for (int i = 0; i < outputData->size(); i++)
  //  if (outputData->at(i))
  //    std::cout << i << std::endl;

  for (int i = 0;i < network.neuronsInRow->size();i++)
    network.neuronsInRow->at(i)->flushCharges();
}


void WtmActor::actionMoveForward()
{
  Coords forwardCoords = getNextCoords(actorsCoords, actorDirection.direction);

  if (world->getCoordType(forwardCoords) != 1) 
    actorsCoords = forwardCoords;
}


void 
WtmActor::actionMoveLeft()
{
  ActorDirection dir = actorDirection.turnDirectionLeft(
    actorDirection.turnDirectionLeft(actorDirection));

  Coords newCoords = getNextCoords(actorsCoords, dir.direction);

  if (world->getCoordType(newCoords) != 1)
    actorsCoords = newCoords;
}


Coords WtmActor::getNextCoords(Coords coords, Directions direction)
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


void WtmActor::actionTurnLeft()
{
  actorDirection = actorDirection.turnDirectionLeft(actorDirection);
}


void WtmActor::actionTurnRight()
{
  actorDirection = actorDirection.turnDirectionRight(actorDirection);
}


WorldObstaclesMapSegment* WtmActor::getObstaclesMapSegment(vector<Coords>* coords)
{
  WorldObstaclesMapSegment* result = new WorldObstaclesMapSegment();
  *result->coords = *coords;

  for (Coords& scopeSector : *coords)
  {
    result->coordsAccessabily->push_back(
      world->getCoordType(scopeSector) == 0);
  }

  return result;
}


vector<Coords>* WtmActor::getVisibilityScope()
{
  vector<Coords>* scope = new vector<Coords>();
  vector<Coords>* tmpScope = new vector<Coords>();


  ActorDirection leftDiagDir = actorDirection.turnDirectionLeft(actorDirection);
  ActorDirection rightDiagDir = actorDirection.turnDirectionRight(actorDirection);
  ActorDirection leftDir = actorDirection.turnDirectionLeft(leftDiagDir);
  ActorDirection rightDir = actorDirection.turnDirectionRight(rightDiagDir);

  tmpScope->push_back(getNextCoords(actorsCoords, leftDir.direction));
  tmpScope->push_back(getNextCoords(actorsCoords, leftDiagDir.direction));
  tmpScope->push_back(getNextCoords(actorsCoords, actorDirection.direction));
  tmpScope->push_back(getNextCoords(actorsCoords, rightDiagDir.direction));
  tmpScope->push_back(getNextCoords(actorsCoords, rightDir.direction));

  tmpScope->push_back(getNextCoords(tmpScope->at(0), leftDir.direction));
  tmpScope->push_back(getNextCoords(tmpScope->at(0), leftDiagDir.direction));
  
  tmpScope->push_back(getNextCoords(tmpScope->at(1), leftDiagDir.direction));

  tmpScope->push_back(getNextCoords(tmpScope->at(2), leftDiagDir.direction));
  tmpScope->push_back(getNextCoords(tmpScope->at(2), actorDirection.direction));
  tmpScope->push_back(getNextCoords(tmpScope->at(2), rightDiagDir.direction));

  tmpScope->push_back(getNextCoords(tmpScope->at(3), rightDiagDir.direction));

  tmpScope->push_back(getNextCoords(tmpScope->at(4), rightDiagDir.direction));
  tmpScope->push_back(getNextCoords(tmpScope->at(4), rightDir.direction));

  for (Coords& tmpScopeSector : *tmpScope)
  {
    if (world->isCoordsExits(tmpScopeSector))
    {
      scope->push_back(tmpScopeSector);
    }
  }

  return tmpScope;
}


void WtmActor::setWorldDataToNetwork(vector<bool>& worldData)
{
  network.installInputData(worldData);
}


void 
WtmActor::updateLogic()
{
  translateWorldDataToNetwork();
  network.makeProcessingPeriod();
  translateNetworkSignalsToActions();
}


void WtmActor::updateView()
{
  screenXInPercents = (world->blockSizeInPercents + world->paddingInPercents) *
    (1.f / 2.f + actorsCoords.x);
  screenYInPercents = (world->blockSizeInPercents + world->paddingInPercents) *
    (1.f / 2.f + actorsCoords.y);

  updatePawnView();
  updateVisibilityScopeView();
}


void WtmActor::updatePawnView()
{
  glPushMatrix();
  
  glColor3f(0.0f, 0.5, 0.0);
  glTranslatef(screenXInPercents, screenYInPercents, 0.f);
  gluDisk(gluNewQuadric(), 0., 0.4 * world->blockSizeInPercents, 64, 1);

  glPopMatrix();
}


void WtmActor::updateVisibilityScopeView()
{
  WorldObstaclesMapSegment* mapSegment = getVisibleMap();
  float leftVertexX;
  float leftVertexY;

  float pad = world->paddingInPercents;
  float blockSize = world->blockSizeInPercents;

  glBegin(GL_QUADS);

  for (int scopeSector = 0; scopeSector < mapSegment->coords->size(); scopeSector++)
  {
    glColor4f(0.5, 1.0, 0.0, 0.5);

    leftVertexX = pad / 2 + (blockSize + pad) * mapSegment->coords->at(scopeSector).x;
    leftVertexY = pad / 2 + (blockSize + pad) * mapSegment->coords->at(scopeSector).y;

    glVertex3f(leftVertexX, leftVertexY, 0.0);
    glVertex3f(leftVertexX + blockSize, leftVertexY, 0.0);
    glVertex3f(leftVertexX + blockSize,leftVertexY + blockSize, 0.0);
    glVertex3f(leftVertexX, leftVertexY + blockSize, 0.0);
  }
  
  glEnd();
  glFlush();
}


WorldObstaclesMapSegment* WtmActor::getVisibleMap()
{
  WorldObstaclesMapSegment* mapSegment =
    getObstaclesMapSegment(getVisibilityScope());

  WorldObstaclesMapSegment* resultMap = mapSegment;

  return resultMap;
}


void 
WtmActor::setCoords(Coords coords) 
{ 
  this->actorsCoords = coords; 
}


bool 
WtmActor::isActorTakePlace(Coords coords)
{
  return actorsCoords == coords;
}



void 
WtmActor::flushNetworkState()
{
  network.flushNetwork();
}



