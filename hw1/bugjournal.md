# Bug 1

## A) How is your program acting differently than you expect it to?
- I am failing line 304 of the test in test_linked_list.
My iterator is not apropriately becoming invalid

## B) Brainstorm a few possible causes of the bug
- My iterator next method does not
 set the iterator node to be null at the correct time
- My implementation of a linkedlist has an error, where 
  possibly an extra node is being added
- Maybe a mistake in a conditional statement for next.

## C) How you fixed the bug and why the fix was necessary
-  I added lines  in the next method to set the node of the
  iterator  to null if the next node of the iterator is null.
  This fix allowed me to pass the test and get the
  correct functionality.



# Bug 2

## A) How is your program acting differently than you expect it to?
-  My program is segfaulting when the test calls HashTable_Remove

## B) Brainstorm a few possible causes of the bug
- Misuse of mallocs and frees 
- Trying to dereference a NULL pointer
- Error in the helper function call

## C) How you fixed the bug and why the fix was necessary
-  My helper function returns a NULL pointer, if there is
   no element in the chain, with the given key.
   However, the code outside of the helper function, 
   would always try to dereference of the result.
   In the cases, where it was null, it would segfault.
   I fixed the bug by adding an additional if statement,
   checking if the result from the helper function is NULL.

# Bug 3

## A) How is your program acting differently than you expect it to?
- My program is segfaulting in HTIterator_Get

## B) Brainstorm a few possible causes of the bug
- My HTIterator_Get is not apropriately updating HTIterator fields
- Misuse of frees and mallocs
- My HTIterator_Next is using flawed logic not apropriately updating
  the bucket the iterator is in.

## C) How you fixed the bug and why the fix was necessary
- I fixed the problem by reviewing how the hash function
  works to get the bucket_idx from an input.
  I realized that I had to check all buckets_idx,
  greater than the current bucket_idx when the current
  bucket_it became invalid. I had to completely
  change the logic in my HTIterator_Next,
  in order to fix this problem.
