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



#ifndef __DISTRIBUTION_H
#define __DISTRIBUTION_H

#include <vector>
#include "RandSource.h"
#include <cstdlib>
#include <cassert>
#include <iostream>

/**
   @class Distribution
   @brief Class dealing with activities related to distribution in the form of vectors of double. 
   @author Truong Huy Nguyen
   @date 05 March 2010
*/


class Distribution
{
  public:
  
    /**
     Samples from distrib, return the bin index which the sampled value belongs to
     @param[in] distrib Distribution from which the value is sampled. Distribution must have at least 2 bins.
     @param[in] randSource Random source to generate random value
     @return The bin index sampled value belongs to
    */
    static long multinomialSample(const std::vector<double>& distrib, long startIndex, long endIndex, RandSource & randSource);
    /**
      Returns the index of the max distrib item. If there are more than one, a random max index is returned.
      @param[in] startIndex the starting index. If set to -1, it'll be 0.
      @param[in] endIndex the ending index. If set to -1, it'll be distrib.size() - 1.
      @param[in] randSource the random source. If set, a random max index is returned when there are more than one candidates. Otherwise, the first max index is returned.
    */
    static long getMax(const std::vector<double>& distrib, long startIndex = -1, long endIndex = -1, RandSource *randSource = 0);
    static double getMaxValue(const std::vector<double>& distrib, long startIndex = -1, long endIndex = -1);
    static double getMinValue(const std::vector<double>& distrib, long startIndex = -1, long endIndex = -1);
    /**
     * @return the index of the pair that has highest probability.
     * */
    static long getMaxLongDouble(const std::vector< std::pair<long, double> >& distrib);
    /**
     * @return the index of the pair sampled with respect to its corresponding probability
     * */
    static long sampleLongDouble(const std::vector< std::pair<long, double> >& distrib, RandSource &randSource);

    static void printDistrib(const std::vector<double>& distrib, long startIndex = -1, long endIndex = -1);
    static void printLong(const std::vector<long>& distrib);
    static void printBool(const std::vector<bool>& distrib);
    static void printLongDouble(const std::vector< std::pair<long, double> >& distrib);

    static void normalize(std::vector<double>& distrib);
    static void scaleTo_0_1(std::vector<double>& distrib);

    static bool isValid(std::vector<double>& distrib);

    static void clone(const std::vector<double>& origDistrib, std::vector<double>& destDistrib);

    static double getDifference(std::vector<double>& distrib1, std::vector<double>& distrib2);

    static double getVariance(std::vector<double>& dataSet);

    static double getSampleStdev(std::vector<double>& dataSet);

    static double getMean(std::vector<double>& dataSet);

    static long getNumTrue(std::vector<bool>& dataSet);
    static long getNumNonNeg(std::vector<int>& dataSet);

    /**
     * @return the standard error of the mean.
     * */
    static double getStandardErrorOfMean(std::vector<double>& dataSet);

	/**
     * return KL(postDistrib, priorDistrib)
     * */
    static double KL_divergence(std::vector<double>& postDistrib, std::vector<double>& priorDistrib);
};
#endif // __DISTRIBUTION_H
