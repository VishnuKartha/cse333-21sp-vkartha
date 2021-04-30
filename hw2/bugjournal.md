# Bug 1

## A) How is your program acting differently than you expect it to?
- Our program is failing the test suite because ParseIntoWordPositionsTable
  returns NULL instead of the populated HashTable.

## B) Brainstorm a few possible causes of the bug
- One way ParseIntoWordPositionsTable could have returned NULL is if the memory
  allocation failed, but this should be caught by the call to Verify333.
- The only other way is if the table had nothing in it after the call to
  InsertContent, which means the problem is that InsertContent doesn't
  actually populate the table.
- There might be a bug with the positions of null terminators, causing words
  to not be actually inserted in AddWordPosition.

## C) How you fixed the bug and why the fix was necessary
- We first used gdb to try to find the offending line of code, but there were
  no segfaults to crash at.
- Reading carefully through AddWordPosition, we realized that we forgot to
  actually insert into tab after doing all of the setup for the WordPositions
  struct. The fix was just to set up an HTKeyValue_t with an appropriate key
  and value equal to the WordPositions pointer, and insert the kv into tab.


# Bug 2

## A) How is your program acting differently than you expect it to?
- searchshell crashes with a double free error.

## B) Brainstorm a few possible causes of the bug
- We might be misunderstanding the posix functions and the way they
  allocate (or don't allocate) memory for certain things. Suspects:
  getline, strtok_r.
- Maybe in the event of an error in the searchshell loop, 
  free is called too many times.
- Could be mixing _Free functions with normal free or could have passed
  the wrong deallocator to a _Free function.

## C) How you fixed the bug and why the fix was necessary
- We narrowed the double free down to strtok_r by commenting out different
  lines. Upon re-reading the documentation, we realized that strtok_r builds
  the tokens from the original string and does not allocate new space. Removing
  the calls to free each token fixed the double free.


# Bug 3

## A) How is your program acting differently than you expect it to?
- When the query contains more than one word, the rank sometimes differs
  from the solution binary's rank. Our rank is always higher than the
  correct rank.

## B) Brainstorm a few possible causes of the bug
- We might be tallying up the rank incorrectly. If this is the cause,
  the most likely source of error is the second loop in MemIndex_Search.
- A file's contributions to rank are being double counted.
- There is an uninitialized variable somewhere related to the rank computation.

## C) How you fixed the bug and why the fix was necessary
- After examining the state of MemIndex_Search in different locations while
  processing a query, we found that certain doc IDs were being processed twice.
  This turned out to be caused by an incorrect usage of LLIterator_Remove,
  since this function returns the iterator to the predecessor if the element
  removed is the last in the list. The fix was to run the removal loop a fixed
  number of iterations rather than repeating until the iterator became invalid.
