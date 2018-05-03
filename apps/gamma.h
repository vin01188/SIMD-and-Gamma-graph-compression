// This code is part of the project "Smaller and Faster: Parallel
// Processing of Compressed Graphs with Ligra+", presented at the IEEE
// Data Compression Conference, 2015.
// Copyright (c) 2015 Julian Shun, Laxman Dhulipala and Guy Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#ifndef BYTECODE_H
#define BYTECODE_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <bitset>

#include "parallel.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include "streamvbytedelta.h"
#include "gettime.h"

#define LAST_BIT_SET(b) (b & (0x80))
#define EDGE_SIZE_PER_BYTE 7

typedef unsigned char uchar;

uchar bits_store[100];

int get_bit(uchar c, int bit) {
  return (c >> bit) & 0x1;
}

/*
  The main decoding work-horse. First eats the specially coded first
  edge, and then eats the remaining |d-1| many edges that are normally
  coded.
*/
template <class T>
  inline void decode(T t, uchar* edgeStart, const uintE &source, const uintT &degree, const bool par=true) {
  size_t edgesRead = 0;
  int curOffset = 0;
  if (degree > 0) {
    uchar curChar = edgeStart[curOffset];
    int curCharOffset = 0;
    uintE startEdge = -1;
    for (int i = 0; i < degree; i++) {
      uintE edge_diff = 0;
      int length = 0;
      while (true) {
        int bit = get_bit(curChar, curCharOffset);
        length++;
        curCharOffset++;
        if (curCharOffset == 8) {
          curCharOffset = 0;
          curOffset++;
          curChar = edgeStart[curOffset];
        }
        if (bit == 1) {
          break;
        }
      }
      for (int j = 0; j < length - 1; j++) {
        int bit = get_bit(curChar, curCharOffset);
        curCharOffset++;
        if (curCharOffset == 8) {
          curCharOffset = 0;
          curOffset++;
          curChar = edgeStart[curOffset];
        }
        edge_diff |= bit << j;
      }
      edge_diff |= 1 << (length - 1);
      uintE edge = startEdge + edge_diff;
      startEdge = edge;
      if (!t.srcTarg(source, edge, i)) {
          break;
      }
    }
  }
}

//decode edges for weighted graph
template <class T>
  inline void decodeWgh(T t, uchar* edgeStart, const uintE &source, const uintT &degree, const bool par=true) {
  // Unimplemented
  return;
}



// Alternate implementation of packOutNgh, which does both the decoding and
// reencoding in one pass of the edges.
template <class P>
inline size_t pack(P pred, uchar* edge_start, const uintE &source, const uintE &degree) {
  // Unimplemented
  return 0;
}

/* Taken from stackoverflow */
int highest_bit_index(uint32_t value)
{ 
  if(value == 0) return 0;
  int depth = 0;
  int exponent = 16;

  while(exponent > 0)
  {
    int shifted = value >> (exponent);
    if(shifted > 0)
    {
      depth += exponent;
      if(shifted == 1) return depth + 1;
      value >>= exponent;
    }
    exponent /= 2;
  }

  return depth + 1;
}

void set_bitset(int offset, int bit) {
  int char_num = offset / 8;
  int rem = offset % 8;
  if (bit == 1) {
    bits_store[char_num] |= (uchar) 1 << rem;
  } else {
    bits_store[char_num] &= (uchar) ~(1 << rem);
  }
}

long compressEdge(uintE bitsetOffset, uintE e) {
  int hib = highest_bit_index(e) - 1;
  uintE curOffset = bitsetOffset;
  for (int i = 0; i < hib; i++) {
    set_bitset(curOffset, 0);
    curOffset++;
  }
  set_bitset(curOffset, 1);
  curOffset++;
  // This is like a backwards gamma code
  for (int i = 0; i < hib; i++) {
    set_bitset(curOffset, e & 0x1);
    e >>= 1;
    curOffset++;
  }
  return curOffset;
}

long flush_bit_set(uintE bitsetOffset, uchar *edgeArray, long currentOffset, bool full) {
  int num_flush = bitsetOffset / 8;
  if (full && bitsetOffset % 8 != 0) {
    int next = (num_flush + 1) * 8;
    for (int i = bitsetOffset; i < next; i++) {
      set_bitset(i, 0);
    }
    num_flush++;
  }
  for (int i = 0; i < num_flush; i++) {
    edgeArray[currentOffset] = bits_store[i];
    currentOffset++;
  }
  bits_store[0] = bits_store[num_flush];
  return currentOffset;
}

/*
  Takes:
    1. The edge array of chars to write into
    2. The current offset into this array
    3. The vertices degree
    4. The vertices vertex number
    5. The array of saved out-edges we're compressing
  Returns:
    The new offset into the edge array
*/
long sequentialCompressEdgeSet(uchar *edgeArray, long currentOffset, uintT degree,
   
                                uintE vertexNum, uintE *savedEdges) {
  if (degree > 0) {
    int bitsetOffset = 0;
    // This does not work when numbers are too big.
    bitsetOffset = compressEdge(bitsetOffset, savedEdges[0] + 1);
    for (uintT edgeI=1; edgeI < degree; edgeI++) {
      // Store difference between cur and prev edge.
      uintE difference = savedEdges[edgeI] -
                        savedEdges[edgeI - 1];
      bitsetOffset = compressEdge(bitsetOffset, difference);
      if (bitsetOffset >= 128) {
        currentOffset = flush_bit_set(bitsetOffset, edgeArray, currentOffset, false);
        bitsetOffset = bitsetOffset % 8;
      }
    }
    currentOffset = flush_bit_set(bitsetOffset, edgeArray, currentOffset, true);
  }
  return currentOffset;
}

/*
  Compresses the edge set in parallel.
*/
uintE *parallelCompressEdges(uintE *edges, uintT *offsets, long n, long m, uintE* Degrees) {
  cout << "parallel compressing, (n,m) = (" << n << "," << m << ")" << endl;

  timer t = timer();
  t.start();
  uintE **edgePts = newA(uintE*, n);
  long *charsUsedArr = newA(long, n);
  long *compressionStarts = newA(long, n+1);
  {parallel_for(long i=0; i<n; i++) {
    charsUsedArr[i] = ceil((Degrees[i] * 9) / 8) + 4;
  }}
  long toAlloc = sequence::plusScan(charsUsedArr,charsUsedArr,n);
  uintE* iEdges = newA(uintE,toAlloc);

  timer t2 = timer();
  t2.start();
  {parallel_for(long i=0; i<n; i++) {
      edgePts[i] = iEdges+charsUsedArr[i];
      long charsUsed =
  sequentialCompressEdgeSet((uchar *)(iEdges+charsUsedArr[i]),
          0, Degrees[i],
          i, edges + offsets[i]);
      charsUsedArr[i] = charsUsed;
  }}
  t2.stop();
  t2.reportTotal("Inside Running Time: ");
  // produce the total space needed for all compressed lists in chars.
  long totalSpace = sequence::plusScan(charsUsedArr, compressionStarts, n);
  compressionStarts[n] = totalSpace;
  free(charsUsedArr);

  uchar *finalArr = newA(uchar, totalSpace);
  cout << "total space requested is : " << totalSpace << endl;
  float avgBitsPerEdge = (float)totalSpace*8 / (float)m;
  cout << "Average bits per edge: " << avgBitsPerEdge << endl;

  {parallel_for(long i=0; i<n; i++) {
      long o = compressionStarts[i];
    memcpy(finalArr + o, (uchar *)(edgePts[i]), compressionStarts[i+1]-o);
    offsets[i] = o;
  }}
  offsets[n] = totalSpace;
  free(iEdges);
  free(edgePts);
  free(compressionStarts);
  t.stop();
  t.reportTotal("Running time: ");
  cout << "finished compressing, bytes used = " << totalSpace << endl;
  cout << "would have been, " << (m * 4) << endl;
  return ((uintE *)finalArr);
}

typedef pair<uintE,intE> intEPair;

/*
  Takes:
    1. The edge array of chars to write into
    2. The current offset into this array
    3. The vertices degree
    4. The vertices vertex number
    5. The array of saved out-edges we're compressing
  Returns:
    The new offset into the edge array
*/
long sequentialCompressWeightedEdgeSet
(uchar *edgeArray, long currentOffset, uintT degree,
 uintE vertexNum, intEPair *savedEdges) {
  // Unimplemented
  return 0;
}

/*
  Compresses the weighted edge set in parallel.
*/
uchar *parallelCompressWeightedEdges(intEPair *edges, uintT *offsets, long n, long m, uintE* Degrees) {
  cout << "parallel compressing, (n,m) = (" << n << "," << m << ")" << endl;
  // Unimplemented
  return NULL;
}

#endif
