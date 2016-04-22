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



#ifndef __RANDSOURCE_H
#define __RANDSOURCE_H

#include <cstdlib>
#include <vector>
#include <iostream>

/**
   @class RandSource
   @brief Source of random numbers
   @details Generates streams of random numbers that are then reused.

   @author Wee Sun Lee
   @date 26 October 2009
*/
class RandSource
{
 public:
  RandSource(long numStream, long blockSize = 10): numStream(numStream), blockSize(blockSize) 
    { 
      sources.resize(numStream);
      for (long j = 0; j < numStream; j++)
			for (long i=0; i< blockSize; i++){
					sources[j].push_back(rand());
			}
      currStream = 0;
      currNum = 0;
    };
    
    inline static void init(unsigned seed) { srand(seed); };

    inline static void throwSomeRandNum(unsigned numThrown){
    	int temp=0;
    	while (temp < numThrown){
    		rand();
    		temp++;
    	}
    };
    
    inline void clearStream(long streamNum)
    {
    	sources[streamNum].clear();
    	for (long i=0; i< blockSize; i++) sources[streamNum].push_back(rand());
    };

    inline unsigned get()
      {
				unsigned out = sources[currStream][currNum];
				currNum++;
				if (((unsigned)currNum) == sources[currStream].size()) {
					for (long i=0; i< blockSize; i++) sources[currStream].push_back(rand());
				}
				//std::cout << "Random number = " << out << std::endl;
				return out;
      };

    inline void startStream(long streamNum)
      {
				currNum = 0;
				currStream = streamNum;
      };

    inline void setStreamPos(long streamNum, long pos)
      {
				currNum = pos;
				currStream = streamNum;
      };

    inline long getStreamNum() { return currStream; };
    inline long getPosInStream() { return currNum; };

    inline void reset()
      {
				for (long i=0; i< numStream; i++)
					sources[i].resize(0);
				for (long j = 0; j < numStream; j++)
					for (long i=0; i< blockSize; i++){
						sources[j].push_back(rand());
					}
				currStream = 0;
				currNum = 0;
      };
    
    inline void putAwayStream(long streamNum)
    {
    	backupSource = sources[streamNum];
    	clearStream(streamNum);
    };

    inline void restoreStream(long streamNum)
    {
    	sources[streamNum] = backupSource;
    };

 public:
// private:
    long numStream;
    long currStream;
    long currNum;
    std::vector<std::vector<unsigned> > sources;
    long blockSize;
    std::vector<unsigned> backupSource;
};

#endif //  __RANDSOURCE_H
