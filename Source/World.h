
#pragma once

#include <windows.h>
#include <glut.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <vector>
#include <string>

using boost::property_tree::ptree;
using boost::property_tree::read_json;

using std::vector;
using std::string;


class World;
class Actor;
struct WorldTask;

struct Coords
{
  int x;
  int y;

  Coords(int y = 0, int x = 0) : y(y), x(x) {}

  Coords& operator+=(const Coords& addition)
  {
    this->y += addition.y;
    this->x += addition.x;
    return *this;
  }
  bool operator==(const Coords& coords)
  {
    return x == coords.x && y == coords.y;
  }
};


enum Directions
{
  Top, TopRight, Right, DownRight, Down, DownLeft, Left, TopLeft
};

struct ActorDirection
{
  Directions direction;
  
  ActorDirection(Directions direction = Directions::Top) : direction(direction) {}
  ActorDirection turnDirectionLeft(ActorDirection direction);
  ActorDirection turnDirectionRight(ActorDirection direction);
};



template <typename T>
vector<T> asVector(ptree const& propertyTree, ptree::key_type const& key)
{
  vector<T> result;
  //for (auto& item : propertyTree.get_child(key))
  //{
  //  result.push_back(item.second.get_value<T>());
  //}

  //BOOST_FOREACH(ptree::value_type rowPair, propertyTree.get_child(key))
  //{
  //  BOOST_FOREACH(ptree::value_type itemPair, rowPair.second) 
  //  {
  //    std::cout << itemPair.second.get_value<string>() << " ";
  //  }
  //  std::cout << std::endl;
  //}
  return result;
}


template <typename T>
vector<vector<T>>* asTwoDimVector(ptree const& propertyTree, ptree::key_type const& key)
{
  vector<vector<T>>* result = new vector<vector<T>>();
  vector<T> resultRow;

  BOOST_FOREACH(ptree::value_type itemsRow, propertyTree.get_child(key))
  {
    BOOST_FOREACH(ptree::value_type item, itemsRow.second)
    {
      resultRow.push_back(item.second.get_value<T>());
    }
    result->push_back(resultRow);
    resultRow.clear();
  }

  return result;
}


struct WorldObstaclesMapSegment
{
  vector<Coords>* coords;
  vector<bool>* coordsAccessabily;

  WorldObstaclesMapSegment()
  {
    coords = new vector<Coords>();
    coordsAccessabily = new vector<bool>();
  }
  ~WorldObstaclesMapSegment()
  {
    delete coords;
    delete coordsAccessabily;
  }
};


class World
{
public:
  int addActor(Actor& actor, Coords coords, Directions dir);
  void addTask(WorldTask& task);
  void updateWorld();

  bool isCoordsExits(Coords& worldPlace);
  int getCoordType(Coords& worldPlace);
  
  bool isWorldLoopActive;
  void startWorldLoop();
  void stopWorldLoop();
  
  float paddingInPercents = 0.005;
  float blockSizeInPercents;
  
  int saveCounter;
  int savePeriod = 100;

private:
  World(vector<vector<int>>* map); 


  friend World* createWorldFromJsonFile(
    string& filename, int argc, char **argv);

  void initWorldDisplay(int argc, char **argv);

  /*------------------------------------------------------*/

  vector<vector<int>>* map;
  vector<WorldTask> tasks;
  vector<Actor*> actors;

  void dismissActors();

  /*------------------------------------------------------*/

  static World* worldInstance;
  //static World* getWorldInstance();
  
  friend void worldDrawLoopTransitFunction();
  friend void worldTasksLoopTransitFunction();

  void tasksLoop();
  void drawLoop();

  int updatePeriod;
  int updateCouter;

  /*------------------------------------------------------*/

  GLuint mapDisplayList;

  void drawMap();
  void createMapDisplayList();

  float screenYInPixels = 400;
  float screenXInPixels = 400;
  //float blockSizeInPixels = 50;

  int xBlocks;
  int yBlocks;
};


class Actor
{
public:
  World* world;

  Coords actorsCoords;
  ActorDirection actorDirection;

  void setWorld(World* world) { this->world = world; }
  void setDir(Directions& dir) 
  { 
    this->actorDirection.direction = dir;
  }

  virtual void registerWorldTasks(World* world) = 0;
  virtual void updateLogic() = 0;
  virtual void updateView() = 0;
  virtual void setCoords(Coords coords) = 0;
  virtual bool isActorTakePlace(Coords coords) = 0;

  //void setCoords(Coords coords) { this->actorsCoords = coords; }

};


class VoidDelegate
{
public:
  virtual ~VoidDelegate() {}
  virtual void invoke() = 0;
};


template<class T>
class VoidMethodDelegate :
  public VoidDelegate
{
public:
  typedef void (T::*TMethod)();

  VoidMethodDelegate(T& object, TMethod method) :
    object(object), method(method) {}

  virtual ~VoidMethodDelegate() {}
  virtual void invoke()
  { 
    (object.*method)();
  }

private:
  T& object;
  TMethod method;
};


template <class T>
VoidDelegate* newVoidDelegate(T& TObject, void (T::*TMethod)())
{
  return new VoidMethodDelegate<T>(TObject, TMethod);
}


struct WorldTask
{
  VoidDelegate* functionToExecute;
  int executionTimeCounter;
  int counterRefreshValue;
};











