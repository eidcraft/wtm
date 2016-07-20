
#include "stdafx.h"
#include "Resources.h"
#include <math.h>
#include <ctime>

vector<vector<int>*>* getArrayCopy(vector<vector<int>*>* array)
{
  vector<vector<int>*>* copy = new vector<vector<int>*>();
  vector<int>* copyRow;

  for (vector<int>* row : *array)
  {
    copyRow = new vector<int>();

    for (int item : *row) copyRow->push_back(item);

    copy->push_back(copyRow);
  }
  return copy;
}


vector<vector<int>*>* absSubtractArrays(
  vector<vector<int>*>* arrayOne, vector<vector<int>*>* arrayTwo)
{
  vector<vector<int>*>* result = getArrayCopy(arrayOne);

  for (int iRow = 0; iRow < result->size(); iRow++)
  {
    for (int iUnit = 0; iUnit < result->size(); iUnit++)
    {
      result->at(iRow)->at(iUnit) = fmax(
        result->at(iRow)->at(iUnit) - arrayTwo->at(iRow)->at(iUnit), 0);
    }
  }
  return result;
}


vector<vector<int>*>* getReshuffles(int n, int k)
{
  vector<vector<int>*>* result = new 
    vector<vector<int>*>();
  vector<int>* resultRow;

  int *A, i, p;

  A = (int *)malloc(k*sizeof(int));

  for (i = 0; i < k; i++) A[i] = i;


  while (true)
  {
    resultRow = new vector<int>();

    for (i = 0; i < k; i++)
      resultRow->push_back(A[i] + 1);

    result->push_back(resultRow);

    if (A[k - 1] < n - 1) A[k - 1]++;
    else
    {
      for (p = k - 1; p > 0; p--)
        if (A[p] - A[p - 1] > 1) break;

      if (p == 0) break;

      A[p - 1] ++;

      for (i = p; i < k; i++) A[i] = A[i - 1] + 1;
    }
  }
  return result;
}


vector<int>* 
getSynapsesPlacingPlan(int n, int k)
{
  int rand = std::rand() % (n-k);

  vector<int>* result = new vector<int>();

  for (int i = rand; i < rand + n; i++)
    result->push_back(i);

  return result;
}


vector<bool>* 
byteAnd(vector<bool>* vOne, vector<bool>* vTwo)
{
  if (vOne->size() != vTwo->size()) return nullptr;

  vector<bool>* result = new vector<bool>(vOne->size());

  for (int i = 0; i < vOne->size(); i++)
    if (vOne->at(i) && vTwo->at(i)) result->at(i) = true;

  return result;
}




float NetworkParameters::getIncreasedSynapticWeight(
  int networkLayerNumber, float currentWeight)
{
  return currentWeight + 1;
  //return weightIncreaseFunc->getSigmaValue(
  //  networkLayerNumber, currentWeight);
}

float NetworkParameters::getDecreasedSynapticWeight(
  int networkLayerNumber, float currentWeight, int updateLag)
{
  return currentWeight;

  //float result = currentWeight;

  //for (int i = 0; i < updateLag; i++)
  //  result = weightDecreaseFunc->getSigmaValue(
  //  networkLayerNumber, result);

  //return result;
}

float NetworkParameters::getIncreasedActivationThreshold(
  int networkLayerNumber, float currentThreshold, int updateLag)
{
  return currentThreshold;

  //float result = currentThreshold;

  //for (int i = 0; i < updateLag; i++)
  //  result = thresholdIncreaseFunc->getSigmaValue(
  //    networkLayerNumber, result);

  //return result;
}

float NetworkParameters::getDecreasedActivationThreshold(
  int networkLayerNumber, float currentThreshold)
{
  return currentThreshold;

  //return thresholdDecreaseFunc->getSigmaValue(
  //  networkLayerNumber, currentThreshold);
}

float NetworkParameters::getIncreasedFatigueSpeed(
  int networkLayerNumber, float currentFatigue)
{
  //return currentFatigue;

  return fatigueIncreaseFunc->getSigmaValue(
    networkLayerNumber, currentFatigue);
}

float NetworkParameters::getDecreasedFatigueSpeed(
  int networkLayerNumber, float currentFatigue, int updateLag)
{
  //return currentFatigue;

  float result = currentFatigue;

  for (int i = 0; i < updateLag; i++)
    result = weightDecreaseFunc->getSigmaValue(
    networkLayerNumber, result);

  return result;
}