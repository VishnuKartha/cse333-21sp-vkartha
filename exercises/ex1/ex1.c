//  Copyright 2021 Vishnu Kartha
//  Author: Vishnu Kartha
//  CSE Email: vkartha@cs.washington.edu
//  Student #: 1873639

#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

bool isValidNonNegativeInteger(char str[], int len);
double neelApprox(int n);

// Prints out an estimate of pi using an infinite series discovered by
// Nilakantha in the 15th century.
// Parses a single command line argument, which should be a non-negative,
// call it n.
// The estimate of pi uses up to the nth term of the
// Nilakantha series(0 indexed).
// Prints out an error message if the user provides an invalid input.
int main(int argc, char* argv[]) {
    if (argc == 2 && isValidNonNegativeInteger(argv[1], strlen(argv[1]))) {
      int n;
      sscanf(argv[1], "%i\n", &n);
      double result = neelApprox(n);
      printf("Our estimate of Pi is ");
      printf("%.20lf\n", result);
    } else {
      printf("Error: Requires a single valid non-negative integer argument\n");
    }
}

// A helper method which takes in a string and the length of the string and
// checks whether the string represents a non-negative integer.
bool isValidNonNegativeInteger(char str[], int len) {
  if (len <= 0) {
    return false;
  } else if (len > 1 && str[0] == '0') {  // edge case where input starts with 0
    return false;
  } else {
    for (int i = 0; i < len; i++) {
      if (!isdigit(str[i])) {
        return false;
      }
    }
    return true;
  }
}

// Takes in a parameter n which is a non-negative integer.
// Returns The estimate of pi which uses up to the nth
// term of the Nilakantha series(0 indexed).
double neelApprox(int n) {
      double result = 3;
      bool negative = false;
      for (int i = 1; i <= n; i++) {
        double newTerm =(4.0 / (2.0 * i * (2.0 * i + 1.0) * (2.0 * i + 2.0)));
        if (negative) {
          newTerm *= -1;
        }
        negative = !negative;
        result =  result + newTerm;
      }
      return result;
}
