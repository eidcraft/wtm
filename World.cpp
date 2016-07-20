
#include "stdafx.h"
#include "World.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>


#include "JsonDataLoader.h"
#include "NetworkCreator.h"
#include "WtmActor.h"
#include "MovingActor.h"


World* World::worldInstance = nullptr;

World* createWorldFromJsonFile(string& filename, int argc, char **argv)
{
  std::ifstream ifStream(filename);
  boost::property_tree::ptree propertyTree;

  boost::property_tree::read_json(ifStream, propertyTree);

  vector<vector<int>>* obstaclesMap =
    asTwoDimVector<int>(propertyTree, "obstaclesMap");

  World* newWorld = new World(obstaclesMap);
  
  //newWorld->xBlocks = obstaclesMap->size();
  //newWorld->yBlocks = obstaclesMap->at(0).size();

  newWorld->initWorldDisplay(argc, argv);

  return newWorld;
}


World::World(vector<vector<int>>* obstaclesMap)
{
  vector<vector<int>>* reversedMap = new vector<vector<int>>();
  *reversedMap = *obstaclesMap;

  std::reverse(reversedMap->begin(), reversedMap->end());

  this->map = reversedMap;

  World::worldInstance = this;
  
  updatePeriod = 100;
  updateCouter = 0;

}


void World::addTask(WorldTask& task)
{
  tasks.emplace_back(task);
}


void worldDrawLoopTransitFunction()
{
  //World* world = World::worldInstance;
  //if (world) world->drawLoop();
}


void worldTasksLoopTransitFunction()
{
	//glColor3f(0.5, 0.3, 0.4);

	//glBegin(GL_QUADS);

	//glVertex3f(0.3, 0.3, 0.5);
	//glVertex3f(0.3, 0.6, 0.5);
	//glVertex3f(0.6, 0.6, 0.5);
	//glVertex3f(0.6, 0.3, 0.5);

	//glEnd();

  World* world = World::worldInstance;
  if (world)
  {
    world->tasksLoop();
    world->drawLoop();
  }
}


void World::initWorldDisplay(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  
  glutInitWindowSize(screenYInPixels, screenXInPixels);
  //glutInitWindowSize(blockSizeInPixels * yBlocks, blockSizeInPixels * xBlocks);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("wtm");
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
  glClearColor(0.5, 0.5, 0.5, 0.0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  createMapDisplayList();

  glutDisplayFunc(worldDrawLoopTransitFunction);
  glutIdleFunc(worldTasksLoopTransitFunction);
}


void World::drawLoop()
{
  glClear(GL_COLOR_BUFFER_BIT);
  
  glCallList(mapDisplayList);

  for (int iActor = 0; iActor < actors.size(); iActor++)
  {
    actors.at(iActor)->updateView();
  }

  glFlush();

  //std::cout << "drawLoop" << std::endl;

}


void World::tasksLoop()
{
  updateCouter++;

  //if (saveCounter == savePeriod)
  //{
  //  updateWorld();
  //  saveCounter++;
  //}
  if (updateCouter >= updatePeriod)
  {
    updateCouter = 0;
    saveCounter++;

    for (Actor* actor : actors)
      actor->updateLogic();

    //for (WorldTask& task : tasks)
    //{
    //  bool isTimeToExecuteTask = task.executionTimeCounter == 0;

    //  if (isTimeToExecuteTask)
    //  {
    //    task.functionToExecute->invoke();
    //    task.executionTimeCounter = task.counterRefreshValue;
    //  }
    //  else
    //  {
    //    task.executionTimeCounter--;
    //  }
    //}
  }

  //drawLoop();
}


void World::createMapDisplayList()
{
  mapDisplayList = glGenLists(1);
  glNewList(mapDisplayList, GL_COMPILE);
  drawMap();
  glEndList();
}


void World::drawMap()
{
  float blockXSize = (1.0 - map->at(0).size() * paddingInPercents) /
    map->at(0).size();
  float blockYSize = (1.0 - map->size() * paddingInPercents) /
    map->size();

  blockSizeInPercents = min(blockYSize, blockXSize);

  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_QUADS);

  float leftVertexX;
  float leftVertexY;
  float xSize = map->at(0).size();
  float ySize = map->size();

  for (size_t y = 0; y < ySize; y++)
  {
    for (size_t x = 0; x < xSize; x++)
    {
      if (map->at(y).at(x) == 0) glColor3f(1.0, 1.0, 1.0);
      else if (map->at(y).at(x) == 1) glColor3f(0.0, 0.0, 0.0);
      else if (map->at(y).at(x) == 2) glColor3f(1.0, 1.0, 0.0);

      leftVertexX = paddingInPercents / 2 + (blockSizeInPercents + 
        paddingInPercents) * x;
      leftVertexY = paddingInPercents / 2 + (blockSizeInPercents + 
        paddingInPercents) * y;

      glVertex3f(leftVertexX, leftVertexY, 0.0);
      glVertex3f(leftVertexX + blockSizeInPercents, leftVertexY, 0.0);
      glVertex3f(leftVertexX + blockSizeInPercents, 
        leftVertexY + blockSizeInPercents, 0.0);
      glVertex3f(leftVertexX, leftVertexY + blockSizeInPercents, 0.0);
    }
  }
  glEnd();
  glFlush();
}


void World::startWorldLoop()
{
  isWorldLoopActive = true;
  glutMainLoop();
}


void World::stopWorldLoop()
{
  isWorldLoopActive = false;
}


int World::getCoordType(Coords& coords)
{
  bool isCoordsExist = isCoordsExits(coords);

  if (!isCoordsExist) return false;

  for (Actor* actor : actors)
    if (actor->isActorTakePlace(coords))
      return 1;

  return map->at(coords.y).at(coords.x);
}


bool World::isCoordsExits(Coords& coords)
{
  return coords.y >= 0 &&
         coords.x >= 0 &&
         coords.y < map->size() &&
         coords.x < map->at(0).size();
}


ActorDirection ActorDirection::turnDirectionLeft(ActorDirection actorDirection)
{
  int tmp = actorDirection.direction - 1;

  if (tmp == -1) return ActorDirection(Directions::TopLeft);
  else           return ActorDirection(Directions(tmp));
}


ActorDirection ActorDirection::turnDirectionRight(ActorDirection actorDirection)
{
  return ActorDirection(Directions(
    (actorDirection.direction + 1) % (Directions::TopLeft + 1)));
}


int World::addActor(Actor& actor, Coords coords, Directions dir)
{
  if (getCoordType(coords) == 1) return -1;

  actors.push_back(&actor);

  actor.setWorld(this);
  actor.registerWorldTasks(this);
  actor.setCoords(coords);
  actor.setDir(dir);

  return 0;
}


void 
World::dismissActors()
{
  actors.clear();
}



void 
World::updateWorld()
{
  WtmActor* wtmActor = (WtmActor*)actors.at(0);

  wtmActor->flushNetworkState();
  actors.clear();

  addActor(*wtmActor, Coords(5, 5), Directions::Top);


  MovingActor* movingActorA = new MovingActor(Directions::Top);
  addActor(*movingActorA, Coords(7, 0), Directions::Right);

  //MovingActor* movingActorB = new MovingActor(Directions::Left);
  //addActor(*movingActorB, Coords(2, 2), Directions::Right);

  //MovingActor* movingActorC = new MovingActor(Directions::Right);
  //addActor(*movingActorC, Coords(3, 4), Directions::Right);
}
