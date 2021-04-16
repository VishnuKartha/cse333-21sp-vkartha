//  Copyright 2021 Vishnu Kartha
//  Author: Vishnu Kartha
//  CSE Email: vkartha@cs.washington.edu
//  Student #: 1873639

#ifndef POINT3D_H_
#define POINT3D_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// a struct which represents a point in 3 dimensions
// it contains 3 integer fields, x, y, and z.
typedef struct Point3d {
  int32_t x;
  int32_t y;
  int32_t z;
} Point3d;

// takes in 3 arguemnts, x, y and z and constructs a
// Point3d struct in the heap with the given values.
// returns a pointer to the Point3d struct created.
// Make sure to free the given pointer after use.
// returns NULL if there is an error in allocation.
Point3d* AllocatePoint3d(int32_t x, int32_t y, int32_t z);

// takes in 2 arguments pointPtr (a pointer to a Point3d struct)
// and an integer scaleFactor.
// Scales the x,y,z fields of the Point3d struct that is pointed
// to in pointPtr by scaleFactor.
void ScalePoint3d(Point3d* pointPtr, int32_t scaleFactor);

// returns a Point3d struct with x,y,z fields set to 0.
Point3d GetOrigin();

#endif  // POINT3D_H_

