//  Copyright 2021 Vishnu Kartha
//  Author: Vishnu Kartha
//  CSE Email: vkartha@cs.washington.edu
//  Student #: 1873639

#include "Point3d.h"

Point3d* AllocatePoint3d(int32_t x, int32_t y, int32_t z) {
    Point3d* pointPtr = (Point3d*) malloc(sizeof(Point3d));
    if (pointPtr == NULL) {
        return NULL;
    }
    pointPtr -> x = x;
    pointPtr -> y = y;
    pointPtr -> z = z;
    return pointPtr;
}

void ScalePoint3d(Point3d* pointPtr, int32_t scaleFactor) {
        if (pointPtr == NULL) {
            return;
        }
        pointPtr -> x *= scaleFactor;
        pointPtr -> y *= scaleFactor;
        pointPtr -> z *= scaleFactor;
}

Point3d GetOrigin() {
        Point3d origin;
        origin.x = 0;
        origin.y = 0;
        origin.z = 0;
        return origin;
}
