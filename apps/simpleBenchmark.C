#include "streamvbytedelta.h"
#include "gettime.h"

typedef unsigned char uchar;
typedef unsigned int uintE;
typedef long intE;

typedef unsigned int uintT;

#define LAST_BIT_SET(b) (b & (0x80))
#define EDGE_SIZE_PER_BYTE 7


inline intE eatFirstEdge(uchar* &start, uintE source) {
  uchar fb = *start++;
  intE edgeRead = (fb & 0x3f);
  if (LAST_BIT_SET(fb)) {
    int shiftAmount = 6;
    while (1) {
      uchar b = *start;
      edgeRead |= ((b & 0x7f) << shiftAmount);
      start++;
      if (LAST_BIT_SET(b))
        shiftAmount += EDGE_SIZE_PER_BYTE;
      else
        break;
    }
  }
  return (fb & 0x40) ? source - edgeRead : source + edgeRead;
}

/*
  Reads any edge of an out-edge list after the first edge.
*/
inline uintE eatEdge(uchar* &start) {
  uintE edgeRead = 0;
  int shiftAmount = 0;

  while (1) {
    uchar b = *start++;
    edgeRead += ((b & 0x7f) << shiftAmount);
    if (LAST_BIT_SET(b))
      shiftAmount += EDGE_SIZE_PER_BYTE;
    else
      break;
  }
  return edgeRead;
}



long compressFirstEdge(uchar *start, long offset, uintE source, uintE target) {
  uchar* saveStart = start;
  long saveOffset = offset;

  intE preCompress = (intE) target - source;
  int bytesUsed = 0;
  uchar firstByte = 0;
  intE toCompress = abs(preCompress);
  firstByte = toCompress & 0x3f; // 0011|1111
  if (preCompress < 0) {
    firstByte |= 0x40;
  }
  toCompress = toCompress >> 6;
  if (toCompress > 0) {
    firstByte |= 0x80;
  }
  start[offset] = firstByte;
  offset++;

  uchar curByte = toCompress & 0x7f;
  while ((curByte > 0) || (toCompress > 0)) {
    bytesUsed++;
    uchar toWrite = curByte;
    toCompress = toCompress >> 7;
    // Check to see if there's any bits left to represent
    curByte = toCompress & 0x7f;
    if (toCompress > 0) {
      toWrite |= 0x80;
    }
    start[offset] = toWrite;
    offset++;
  }
  return offset;
}

/*
  Should provide the difference between this edge and the previous edge
*/

long compressEdge(uchar *start, long curOffset, uintE e) {
  uchar curByte = e & 0x7f;
  int bytesUsed = 0;
  while ((curByte > 0) || (e > 0)) {
    bytesUsed++;
    uchar toWrite = curByte;
    e = e >> 7;
    // Check to see if there's any bits left to represent
    curByte = e & 0x7f;
    if (e > 0) {
      toWrite |= 0x80;
    }
    start[curOffset] = toWrite;
    curOffset++;
  }
  return curOffset;
}


void compress(uchar *edgeArray, uintT degree, uintE vertexNum, uintE *savedEdges) {
	size_t currentOffset = 0;
	currentOffset = compressFirstEdge(edgeArray, currentOffset,
                                       vertexNum, savedEdges[0]);
  for (uintT edgeI=1; edgeI < degree; edgeI++) {
  	// Store difference between cur and prev edge.
    uintE difference = savedEdges[edgeI] -
                      savedEdges[edgeI - 1];
    currentOffset = compressEdge(edgeArray, currentOffset, difference);
  }
}

void decode(uchar* edgeStart, uintE* dest, const uintE &source,const uintT &degree) {

  // Eat first edge, which is compressed specially
  uintE startEdge = eatFirstEdge(edgeStart,source);
  dest[0] = startEdge;
  for (size_t edgesRead = 1; edgesRead < degree; edgesRead++) {
    // Eat the next 'edge', which is a difference, and reconstruct edge.
    uintE edgeRead = eatEdge(edgeStart);
    uintE edge = startEdge + edgeRead;
    dest[edgesRead] = edge;
    startEdge = edge;
  }
}

// Benchmarks running time of encoding and decoding a sequential array
void benchmarkSequential() {
	uint32_t * arr = (uint32_t*) calloc(50000, sizeof(uint32_t));
  uint32_t* dest = (uint32_t*) calloc(50000, sizeof(uint32_t));

  uchar *arr1 = (uchar *) calloc(300000, sizeof(uchar));

  srand(time(NULL));

  for(int i = 0; i < 50000; i++) {
    arr[i] = i;
  }

  timer t1 = timer();
  t1.start();
  for( int j = 0 ; j < 10000 ; j++) {
  	streamvbyte_delta_encode(arr, 50000, (uint8_t *)arr1,0);
  }
  t1.reportTotal("Sequential Array Encoding time: ");

  timer t3 = timer();
  t3.start();

  for(int j = 0 ; j < 10000; j++) {
    streamvbyte_delta_decode(arr1, dest, 50000, 0);
  }
  t3.reportTotal("Sequential Array Decoding time: ");

  timer t4 = timer();
  t4.start();
  for (int j = 0 ; j < 10000 ; j++) {
  	compress(arr1, 50000, 0, arr);
  }
  t4.reportTotal("Sequential Array Byte encoding time: ");

 	timer t5 = timer();
 	t5.start();
 	for (int j = 0; j < 10000 ;j++) {
 		decode(arr1, dest, 0, 50000);
 	}
 	t5.reportTotal("Sequential Array Byte decoding time: ");

  free(arr);
  free(dest);
}

// Benchmarks running time of encoding and decoding a random array
void benchmarkRandom() {
	uint32_t * arr = (uint32_t*) calloc(50000, sizeof(uint32_t));
  uint32_t* dest = (uint32_t*) calloc(50000, sizeof(uint32_t));

  uchar *arr1 = (uchar *) calloc(300000, sizeof(uchar));

  srand(time(NULL));

  arr[0] = 0;
  for(int i = 1; i < 50000; i++) {
    arr[i] = arr[i-1] + 1 + ((rand()) % 1000);
  }

  timer t1 = timer();
  t1.start();
  for( int j = 0 ; j < 10000 ; j++) {
  	streamvbyte_delta_encode(arr, 50000, (uint8_t *)arr1,0);
  }
  t1.reportTotal("Random Array Encoding time: ");

  timer t3 = timer();
  t3.start();

  for(int j = 0 ; j < 10000; j++) {
    streamvbyte_delta_decode(arr1, dest, 50000, 0);
  }
  t3.reportTotal("Random Array Decoding time: ");

  for(int j = 0; j < 10000; j++) {
  	if (dest[j] != arr[j]) {
  		printf("Mismatch \n");
  	}
  }

  timer t4 = timer();
  t4.start();
  for (int j = 0 ; j < 10000 ; j++) {
  	compress(arr1, 50000, 0, arr);
  }
  t4.reportTotal("Random Array Byte encoding time: ");

 	timer t5 = timer();
 	t5.start();
 	for (int j = 0; j < 10000 ;j++) {
 		decode(arr1, dest, 0, 50000);
 	}
 	t5.reportTotal("Random Array Byte decoding time: ");

  for(int j = 0; j < 10000; j++) {
  	if (dest[j] != arr[j]) {
  		printf("Mismatch %d %d \n", dest[j], arr[j]);
  	}
  }
  free(arr);
  free(dest);
}

int main(int argc, char*argv[]) {
	benchmarkSequential();
	benchmarkRandom();

}