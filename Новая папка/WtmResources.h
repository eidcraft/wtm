
#pragma once

#include <vector>
#include <string>
using std::vector;
using std::string;

struct WtmSourceData
{
  string title;
  vector<vector<bool>*> inputData;
  vector<vector<bool>*> outputData;
};

vector<vector<int>*>* getArrayCopy(vector<vector<int>*>* array);

vector<vector<int>*>* absSubtractArrays(
  vector<vector<int>*>* arrayOne, vector<vector<int>*>* arrayTwo);

vector<vector<int>*>* getReshuffles(int n, int k);

vector<int>* getSynapsesPlacingPlan(int n, int k);


vector<bool>* byteAnd(vector<bool>*result, vector<bool>* newData);