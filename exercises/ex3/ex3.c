//  Copyright 2021 Vishnu Kartha
//  Author: Vishnu Kartha
//  CSE Email: vkartha@cs.washington.edu
//  Student #: 1873639

#include "Point3d.h"
#include <stdbool.h>

//  main method using to test the methods defined in Point3d.c
int main(int argc, char* argv[]) {
  // AllocatePoint3d test
  Point3d* pointPtr = AllocatePoint3d(1, 2, 3);
  if (pointPtr -> x != 1 || pointPtr -> y != 2 || pointPtr -> z != 3) {
    printf("ALLOCATEPOINT3d FAILED\n");
    return EXIT_FAILURE;
  }
  // ScalePoint3d test
  ScalePoint3d(pointPtr, 3);
  if (pointPtr -> x != 3 || pointPtr -> y != 6 || pointPtr -> z != 9) {
    printf("SCALEPOINT3d FAILED\n");
  }
  // GetOrigin() test
  Point3d origin = GetOrigin();
  if (origin.x != 0 || origin.y != 0 || origin.z != 0) {
    printf("GETORIGIN FAILED\n");
    return EXIT_FAILURE;
  }
  free(pointPtr);
  return EXIT_SUCCESS;
}


