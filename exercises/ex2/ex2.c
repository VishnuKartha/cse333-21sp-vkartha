//  Copyright 2021 Vishnu Kartha
//  Author: Vishnu Kartha
//  CSE Email: vkartha@cs.washington.edu
//  Student #: 1873639

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

void DumpBytes(void* pData, int byteLen);
void CopyAndSort(uint8_t* src, uint8_t* dst, int len);

// the main method given from the specs.
// the blanks are filled with the appropriate size parameters.
int main(int argc, char* argv[]) {
  int32_t int_val = 1;
  float float_val = 1.0f;
  uint8_t arr_unsorted[] = {3, 2, 0, 8, 17, 6, 10, 7, 8, 1, 12};
  uint8_t arr_sorted[]   = {0, 0, 0, 0,  0, 0,  0, 0, 0, 0,  0};
  DumpBytes(&int_val, sizeof(int_val));
  DumpBytes(&float_val, sizeof(float_val));
  DumpBytes(arr_unsorted, sizeof(arr_unsorted));
  CopyAndSort(arr_unsorted, arr_sorted, sizeof(arr_unsorted));
  DumpBytes(arr_sorted, sizeof(arr_sorted));
  return EXIT_SUCCESS;
}

// DumpBytes prints out the the given length of data,
// the given address, and the bytes of memory stored at the given
// address as exactly two digits each in lowercase hexadecimal.
// Arguments:
// pData - the address of the memory to be printed.
// byteLen -  the length of the memory in bytes.
void DumpBytes(void* pData, int byteLen) {
  uint8_t* dataPointer = (uint8_t*) pData;
  printf("The %d bytes starting at %p are:", byteLen, dataPointer);
  for (int i = 0; i < byteLen; i++) {
    printf(" %02" PRIx8, dataPointer[i]);
  }
  printf("\n");
}

// CopyAndSort first calls DumpBytes on the given source array.
// It then copies and sorts the values of the given source array
// into the given destination array, leaving the source array
// unaffected.
// Note: the src and dst array must have the same length
// Arguments:
// src - the given source array
// dst - the given destination array
// len - length of the two arrays(must be the same)
void CopyAndSort(uint8_t* src, uint8_t* dst, int len) {
  // checks for bad input
  if (len < 0) {
    exit(EXIT_FAILURE);
  }
  DumpBytes(src, sizeof(src));
  if (len >= 1) {
    dst[0] = src[0];
  }
  // sorting using insertion sort
  for (int i = 1; i < len; i++) {
    int j = i - 1;
    // find appropriate place to insert current value(src[i]).
    while (j >= 0 && dst[j] > src[i]) {
      dst[j + 1] = dst[j];
      j = j - 1;
    }
      dst[j + 1] = src[i];  // inserts current value
  }
}

