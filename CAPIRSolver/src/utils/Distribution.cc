/*
 * Copyright (c) 2012 Truong-Huy D. Nguyen.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Public License v3.0
 * which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/gpl.html
 * 
 * Contributors:
 *     Truong-Huy D. Nguyen - initial API and implementation
 */



#include "Distribution.h"
#include "Utilities.h"
#include <cmath>

using namespace std;

void Distribution::normalize(vector<double>& distrib) {
  double sum = 0;
  for (unsigned i = 0; i < distrib.size(); i++)
    sum += distrib[i];
  if (sum > 0)
    for (unsigned i = 0; i < distrib.size(); i++)
      distrib[i] /= sum;
}
;

void Distribution::scaleTo_0_1(std::vector<double>& distrib)
{
	double min = getMinValue(distrib);
	double max = getMaxValue(distrib);

	if (min == max){
		// set to normalize if this distrib is non-zero
		if (min != 0 ){
			normalize(distrib);
		}
	}
	else for (unsigned i = 0; i < distrib.size(); i++)
		distrib[i] = (distrib[i] - min) / (max - min);
};

/**
 There must be at least 2 elements in distrib
 */
long Distribution::multinomialSample(const vector<double>& distrib,
    long startIndex, long endIndex, RandSource & randSource) {
  // return Distribution::getMax(distrib, startIndex, endIndex);
  // comment out for testing

  if (startIndex < 0)
    startIndex = 0;
  if (endIndex < 0)
    endIndex = distrib.size() - 1;

  double sumAll = 0;
  vector<double> distribList;

  distribList.resize(endIndex - startIndex, 0);

  // construct distribList and sum distrib to normalize
  distribList[0] = distrib[startIndex];
  sumAll = distrib[startIndex];

  for (unsigned i = 1; i < endIndex - startIndex; i++) {
    sumAll += distrib[startIndex + i];
    distribList[i] = sumAll;
  }

  sumAll += distrib[endIndex];

  // sample
  double value = ((double) randSource.get()) / RAND_MAX;
  //cout << "value=" << value << endl;
  //printDistrib(distribList, 0, endIndex-startIndex-1);
  for (unsigned i = 0; i < endIndex - startIndex; i++) {
    if (value < distribList[i] / sumAll)
      return startIndex + i;
  }

  // the value belongs to the last bin
  return endIndex;

}
;

// max in [startIndex, endIndex]
long Distribution::getMax(const vector<double>& distrib, long startIndex,
    long endIndex, RandSource *randSource) {
  if (startIndex < 0)
    startIndex = 0;
  if (endIndex < 0)
    endIndex = distrib.size() - 1;

  //assert ( (startIndex <= endIndex) && (startIndex >0) && (endIndex < ((long)distrib.size())) );

  long maxIndex = startIndex;

  for (long i = startIndex + 1; i <= endIndex; i++) {
    if (distrib[maxIndex] < distrib[i]) {
      maxIndex = i;
    }
  }

  if (randSource) {
    // Get all index that has max value
    vector<long> maxIndices;
    for (long i = startIndex + 1; i <= endIndex; i++) {
      if (fabs(distrib[maxIndex] - distrib[i]) < __epsilon) {
        maxIndices.push_back(i);
      }
    }
    // if there are more than one max values, return a random max index
    if (maxIndices.size() > 1)
      return maxIndices[randSource->get() % maxIndices.size()];
  }

  return maxIndex;
}
;

double Distribution::getMaxValue(const vector<double>& distrib,
    long startIndex, long endIndex) {

  //assert ( (startIndex <= endIndex) && (startIndex >0) && (endIndex < ((long)distrib.size())) );

  if (startIndex < 0)
    startIndex = 0;
  if (endIndex < 0)
    endIndex = distrib.size() - 1;

  long maxIndex = startIndex;

  for (long i = startIndex + 1; i <= endIndex; i++) {
    if (distrib[maxIndex] < distrib[i]) {
      maxIndex = i;
    }
  }

  return distrib[maxIndex];
}
;

double Distribution::getMinValue(const std::vector<double>& distrib, long startIndex, long endIndex)
{
  if (startIndex < 0)
	startIndex = 0;
  if (endIndex < 0)
	endIndex = distrib.size() - 1;

  long minIndex = startIndex;

  for (long i = startIndex + 1; i <= endIndex; i++) {
	if (distrib[minIndex] > distrib[i]) {
		minIndex = i;
	}
  }

  return distrib[minIndex];
};

long Distribution::getMaxLongDouble(
    const vector<pair<long, double> >& distrib) {
  long maxIndex = 0;

  for (unsigned i = 1; i < distrib.size(); i++) {
    if (distrib[maxIndex].second < distrib[i].second) {
      maxIndex = i;
    }
  }

  return maxIndex;
}
;

long Distribution::getNumTrue(std::vector<bool>& dataSet)
{
	long result = 0;
	for (unsigned i=0; i<dataSet.size(); i++)
		if (dataSet[i])
			result++;
	return result;
};


long Distribution::getNumNonNeg(std::vector<int>& dataSet)
{
	long result = 0;
	for (unsigned i=0; i<dataSet.size(); i++)
		if (dataSet[i]>=0)
			result++;
	return result;
};

long Distribution::sampleLongDouble(
    const vector<pair<long, double> >& distrib,
    RandSource &randSource) {

  if (distrib.size() == 1)
    return 0;

  double sumAll = 0;
  vector<double> distribList;

  distribList.resize(distrib.size() - 1, 0);

  // construct distribList and sum distrib to normalize
  distribList[0] = distrib[0].second;
  sumAll = distrib[0].second;

  for (unsigned i = 1; i < distribList.size(); i++) {
    sumAll += distrib[i].second;
    distribList[i] = sumAll;
  }

  sumAll += distrib[distrib.size() - 1].second;

  // sample
  double value = ((double) randSource.get()) / RAND_MAX;

  for (unsigned i = 0; i < distribList.size(); i++) {
    if (value < distribList[i] / sumAll)
      return i;
  }

  // the value belongs to the last bin
  return distribList.size();

}
;

void Distribution::printDistrib(const vector<double>& distrib,
    long startIndex, long endIndex) {
  if (startIndex < 0)
    startIndex = 0;
  if (endIndex < 0)
    endIndex = distrib.size() - 1;

  for (long i = startIndex; i <= endIndex; i++) {
    cout << distrib[i] << " ";
  }
  cout << endl;
}
;

void Distribution::printLong(const vector<long>& distrib) {
  for (unsigned i = 0; i < distrib.size(); i++) {
    cout << distrib[i] << " ";
  }
  cout << endl;
}
;

void Distribution::printBool(const vector<bool>& distrib) {
  for (unsigned i = 0; i < distrib.size(); i++) {
    cout << (distrib[i] ? 1 : 0) << " ";
  }
  cout << endl;
}
;

void Distribution::printLongDouble(const vector< pair<long, double> >& distrib)
{
	for (unsigned i = 0; i < distrib.size(); i++) {
		std::cout << "("<< distrib[i].first << ", " << distrib[i].second << ") ";
	}
	cout << endl;
};

bool Distribution::isValid(vector<double>& distrib) {

  double sum = 0;
  for (unsigned i = 0; i < distrib.size(); i++) {
    sum += distrib[i];
  }

  if (sum > 0)
    return true;
  return false;
}
;

void Distribution::clone(const vector<double>& origDistrib,
    vector<double>& destDistrib) {
  destDistrib.resize(origDistrib.size());
  for (unsigned i =0; i<origDistrib.size(); i++){
    destDistrib[i] = origDistrib[i];
  }
}
;

/**
 * This routine returns the sum of absolute difference
 * */
double Distribution::getDifference(vector<double>& distrib1, vector<double>& distrib2){

  if (distrib1.size() != distrib2.size()){
    exit(1);
  }

  double result = 0;

  for (unsigned i=0; i<distrib1.size(); i++){
    result += fabs(distrib1[i] - distrib2[i]);
  }
  return result;
};

/**
 * logarithm in base 2.
 * */
double Distribution::KL_divergence(vector<double>& postDistrib,
		vector<double>& priorDistrib) {
	double result = 0;

	for (unsigned i = 0; i < postDistrib.size(); i++) {
		if (postDistrib[i] > 0)
			result += postDistrib[i] * log2( postDistrib[i] / priorDistrib[i] );
	}

	if (result < 0){
		cout << "KL_Divergence is negative! No way!" << endl;
		exit(1);
	}

	return result;
}
;

double Distribution::getVariance(vector<double>& dataSet)
{
	if (dataSet.size() <= 1){
		//cout << "Dataset size <= 1";
		return 0.0;
	}

	double mean = getMean(dataSet);
	double sum = 0;
	for (unsigned i = 0; i < dataSet.size(); i++)
		sum += pow(dataSet[i] - mean, 2);

	return sum / (dataSet.size() - 1);
};

double Distribution::getSampleStdev(vector<double>& dataSet)
{
	return sqrt(getVariance(dataSet));
};

double Distribution::getMean(vector<double>& dataSet)
{
	if (dataSet.empty()){
		//cout << "Dataset size = 0";
		return 0.0;
	}

	double sum = 0;
	for (unsigned i = 0; i < dataSet.size(); i++)
		sum += dataSet[i];

	return sum / dataSet.size();
};

double Distribution::getStandardErrorOfMean(vector<double>& dataSet)
{
	if (dataSet.empty()){
		//cout << "Dataset size = 0";
		return 0.0;
	}

	return getSampleStdev(dataSet) / sqrt(dataSet.size());
};
