
#pragma once

#include <vector>
using std::vector;

#include <string>
using std::string;


class WtmSigma
{
public:

  WtmSigma(float initialYScale, float initialXOffset, float initialExpCoeff,
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



class WtmParameters
{
public:
  int networkPeriod;
  int layersNumber;

  int inputNeuronsNumber;
  int outputNeuronNumber;
  int uniqueNeuronsPerVNet;

  float averageSynapseWeight;

  float activationThreshold;
  float fatigueThreshold;

  WtmSigma* thresholdIncreaseFunc;
  WtmSigma* thresholdDecreaseFunc;

  WtmSigma* fatigueIncreaseFunc;
  WtmSigma* fatigueDecreaseFunc;

  WtmSigma* weightIncreaseFunc;
  WtmSigma* weightDecreaseFunc;


  //float getIncreasedActivationThreshold(
  //  int networkLayerNumber, float currentThreshold, int updateLag);

  //float getDecreasedFatigueSpeed(
  //  int networkLayerNumber, float currentFatigue, int updateLag);

  //float getDecreasedSynapticWeight(
  //  int networkLayerNumber, float currentWeight, int updateLag);


  //float getDecreasedActivationThreshold(
  //  int networkLayerNumber, float currentThreshold);

  //float getIncreasedFatigueSpeed(
  //  int networkLayerNumber, float currentFatigue);

  //float getIncreasedSynapticWeight(
  //  int networkLayerNumber, float currentWeight);


};



