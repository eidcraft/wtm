
#pragma once

#include <vector>
#include <string>
using std::vector;
using std::string;


class Sigma
{
public:

  Sigma(float initialYScale, float initialXOffset, float initialExpCoeff,
    float endYScale, float endXOffset, float endExpCoeff,
    int layersNumber, bool isIncreasing, string& title) :
    initialYScale(initialYScale), initialXOffset(initialXOffset),
    initialExpCoeff(initialExpCoeff), endYScale(endYScale),
    endXOffset(endXOffset), endExpCoeff(endExpCoeff),
    layersNumber(layersNumber), isIncreasing(isIncreasing), title(title)
  {
    float stepYScale = (endYScale - initialYScale) / layersNumber;
    float stepXOffset = (endExpCoeff - initialXOffset) / layersNumber;
    float stepExpCoeff = (endExpCoeff - initialExpCoeff) / layersNumber;

    yScales.push_back(initialYScale);
    xOffsets.push_back(initialXOffset);
    expCoeffs.push_back(initialExpCoeff);

    for (int iLayer = 1; iLayer < layersNumber; iLayer++)
    {
      yScales.push_back(yScales.at(iLayer - 1) + stepYScale);
      xOffsets.push_back(xOffsets.at(iLayer - 1) + stepXOffset);
      expCoeffs.push_back(expCoeffs.at(iLayer - 1) + stepExpCoeff);
    }
  };

  //private:
  string title;

  float initialYScale;
  float initialXOffset;
  float initialExpCoeff;

  float endYScale;
  float endXOffset;
  float endExpCoeff;

  int layersNumber;


  vector<float> yScales;
  vector<float> xOffsets;
  vector<float> expCoeffs;

  bool isIncreasing;

  float getSigmaValue(float networkLayer, float xValue)
  {
    float curYScale = yScales.at((unsigned int)networkLayer);
    float curXOffset = xOffsets.at((unsigned int)networkLayer);
    float curExpCoeff = expCoeffs.at((unsigned int)networkLayer);

    float result = curYScale / (1 + curExpCoeff * exp(-xValue + curXOffset));
    if (!isIncreasing) result = curYScale - result;

    return result;
  }
};


class NetworkParameters
{
public:
  int networkPeriod;
  int layersNumber;
  int signalsPerAction;

  int inputNeuronsNumber;
  int outputNeuronNumber;
  int uniqueNeuronsPerVNet;

  float averageSynapseWeight;
  float initialSynapseWeight;

  float activationThreshold;
  float fatigueThreshold;

  Sigma* thresholdIncreaseFunc;
  Sigma* thresholdDecreaseFunc;

  Sigma* fatigueIncreaseFunc;
  Sigma* fatigueDecreaseFunc;

  Sigma* weightIncreaseFunc;
  Sigma* weightDecreaseFunc;


  float getIncreasedActivationThreshold(
    int networkLayerNumber, float currentThreshold, int updateLag);

  float getDecreasedFatigueSpeed(
    int networkLayerNumber, float currentFatigue, int updateLag);

  float getDecreasedSynapticWeight(
    int networkLayerNumber, float currentWeight, int updateLag);


  float getDecreasedActivationThreshold(
    int networkLayerNumber, float currentThreshold);

  float getIncreasedFatigueSpeed(
    int networkLayerNumber, float currentFatigue);

  float getIncreasedSynapticWeight(
    int networkLayerNumber, float currentWeight);


};


struct WtmSourceData
{
  string title;
  vector<vector<bool>*> inputData;
  vector<vector<bool>*> outputData;
};


typedef vector<WtmSourceData*> MotorBasisData;
typedef vector<WtmSourceData*> BasicBehaviorData;

struct WtmSourceDataSet
{
  NetworkParameters* networkParameters;
  MotorBasisData* motorBasisData;
  BasicBehaviorData* basicBehaviorData;

  WtmSourceDataSet(NetworkParameters* networkParameters,
                   MotorBasisData* motorBasisData,
                   BasicBehaviorData* basicBehaviorData) :
    networkParameters(networkParameters),
    motorBasisData(motorBasisData),
    basicBehaviorData(basicBehaviorData) {}
};

typedef vector<int> Combination;

class ÑombinationsFabric
{
public:
  static ÑombinationsFabric newFabric(int n, int k);

  int* A;
  int i;
  int p;
  int n;
  int k;
  int end = 0;

  ÑombinationsFabric(int n, int k) : n(n), k(k)
  {
    A = (int *)malloc(k*sizeof(int));

    for (i = 0; i < k; i++) A[i] = i;
  }


  vector<int>* nextCombination()
  {
    if (end == 1) return nullptr;

    vector<int>* result = new vector<int>();

    for (i = 0; i < k; i++)
      result->push_back(A[i] + 1);

    if (A[k - 1] < n - 1) A[k - 1]++;
    else
    {
      for (p = k - 1; p > 0; p--)
        if (A[p] - A[p - 1] > 1) break;

      if (p == 0)
      {
        end = 1;
        return result;
      }

      A[p - 1] ++;

      for (i = p; i < k; i++) A[i] = A[i - 1] + 1;
    }

    return result;
  }
};


vector<vector<int>*>* getArrayCopy(vector<vector<int>*>* array);

vector<vector<int>*>* absSubtractArrays(
  vector<vector<int>*>* arrayOne, vector<vector<int>*>* arrayTwo);

vector<vector<int>*>* getReshuffles(int n, int k);

vector<int>* getSynapsesPlacingPlan(int n, int k);


vector<bool>* byteAnd(vector<bool>*result, vector<bool>* newData);