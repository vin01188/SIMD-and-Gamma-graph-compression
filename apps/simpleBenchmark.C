#include "streamvbytedelta.h"
#include "gettime.h"

typedef unsigned char uchar;

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

  free(arr);
  free(dest);
}

// Benchmarks running time of encoding and decoding a random array
void benchmarkRandom() {
	uint32_t * arr = (uint32_t*) calloc(50000, sizeof(uint32_t));
  uint32_t* dest = (uint32_t*) calloc(50000, sizeof(uint32_t));

  uchar *arr1 = (uchar *) calloc(300000, sizeof(uchar));

  srand(time(NULL));

  for(int i = 0; i < 50000; i++) {
    arr[i] = rand();
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

  free(arr);
  free(dest);
}

int main(int argc, char*argv[]) {
	benchmarkSequential();
	benchmarkRandom();

}