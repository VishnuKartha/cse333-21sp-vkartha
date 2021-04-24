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
- 

## B) Brainstorm a few possible causes of the bug
- 
- 
- 

## C) How you fixed the bug and why the fix was necessary
- 


# Bug 3

## A) How is your program acting differently than you expect it to?
- 

## B) Brainstorm a few possible causes of the bug
- 
- 
- 

## C) How you fixed the bug and why the fix was necessary
- 
