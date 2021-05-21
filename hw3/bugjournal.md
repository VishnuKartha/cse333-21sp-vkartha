# Bug 1

## A) How is your program acting differently than you expect it to?
- We are getting a segfault in WriteDocidToDocnameFn due to strlen.

## B) Brainstorm a few possible causes of the bug
- A strlen segfault means the input string must be bad. Maybe the null
  terminator is missing?
- The input string could also be a bad pointer, which would make
  strlen perform an illegal access.
- The input HTKeyValue_t might be malformed. This would mean either
  the value wasn't set properly or the actual pointer is bad.

## C) How you fixed the bug and why the fix was necessary
- We carefully traced through the callstack with gdb to see what was
  originally passed into WriteDocidToDocnameFn as kv. Changing the type of kv
  in WriteHTBucket from HTKeyValue_t to HTKeyValue_t* fixed the bug. We
  misunderstood the type being stored in the LinkedList chains.


# Bug 2

## A) How is your program acting differently than you expect it to?
- The f_stat size isn't matching up with the header's sizes in 
  FileIndexReader's constructor.

## B) Brainstorm a few possible causes of the bug
- There could be an uninitialized value because the header's sizes are
  unrealistically enormous.
- We could be using fread incorrectly, which would also result in
  garbage values in the header.
- We might be using setvbuf incorrectly due to not understanding the interface
  completely. This could also mess up the read.

## C) How you fixed the bug and why the fix was necessary
- After narrowing down the bug to the first couple of steps in the constructor,
  we noticed that we fread into header instead of header_ :(. The result was
  that header_ was uninitialized and contained garbage values. Changing
  all instances of header to header_ fixed the bug.


# Bug 3

## A) How is your program acting differently than you expect it to?
- HashTableReader is producing a 0 length chain when it should be 1 
  according to test_hashtablereader.cc.

## B) Brainstorm a few possible causes of the bug
- The problem must be with the BucketRecord, since the chain's length
  is dictated by chain_num_elements.
- Something might be wrong with freading: the size, the number of elements, etc.
- Might have read wrong in the constructor. The errors could not show up
  until trying to read something later by chance.

## C) How you fixed the bug and why the fix was necessary
- We forgot to seek to the correct place in LookupElementPositions before
  reading in the BucketRecord. There has to be a seek because other seeks
  and reads can happen between calls to LookupElementPositions. Adding
  an fseek to the given bucket_rec_offset fixed the bug.
