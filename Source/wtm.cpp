// wtm.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#include "JsonDataLoader.h"
#include "NetworkCreator.h"

#include "World.h"
#include "WtmActor.h"
#include "MovingActor.h"


int main(int argc, char **argv)
{
  string sourceFile("SourceData.json");
  WtmSourceDataSet* wtmSourceDataSet = WtmJsonLoader::loadWtmSourceDataSet(sourceFile);
  Network* network = NetworkCreator::create(wtmSourceDataSet);

  string worldFile("world2.json");
  World* world = createWorldFromJsonFile(worldFile, argc, argv);

  WtmActor wtmActor(network);
  world->addActor(wtmActor, Coords(6, 6), Directions::Top);
  
  MovingActor movingActorA(Directions::Top);
  world->addActor(movingActorA, Coords(8, 0), Directions::Right);

  MovingActor movingActorB(Directions::Right);
  world->addActor(movingActorB, Coords(9, 5), Directions::Right);

  MovingActor movingActorC(Directions::Right);
  world->addActor(movingActorC, Coords(3, 11), Directions::Right);

  MovingActor movingActorD(Directions::Right);
  world->addActor(movingActorD, Coords(2, 3), Directions::Right);

  world->startWorldLoop();


  ///*----------------------------------------------------------------------------------*/

  //string wtmNetworkFile("DestData.json");
  //Network* network = WtmJsonLoader::loadNetwork(wtmNetworkFile);

  //string worldFile("world2.json");
  //World* world = createWorldFromJsonFile(worldFile, argc, argv);

  //MovingActor movingActor(Directions::Top);
  //world->addActor(movingActor, Coords(7, 7), Directions::Right);
  //WtmActor wtmActor(network);
  //world->addActor(wtmActor, Coords(6, 9), Directions::Top);

  //world->startWorldLoop();

  ///*----------------------------------------------------------------------------------*/

  //string destFile("DestData.json");

  //WtmJsonLoader::saveNetwork(destFile, wtmActor.network);

	return 0;
}

