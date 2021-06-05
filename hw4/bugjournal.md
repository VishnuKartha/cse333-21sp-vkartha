# Bug 1

## A) How is your program acting differently than you expect it to?
- Our program fails a ServerSocket test due to a file descriptor
  being negative.

## B) Brainstorm a few possible causes of the bug
- The test doesn't fail on every run, so it might be an uninitialized value.
- Might not be handling a network-related error properly.
- Could be bad memory access or pointer usage.

## C) How you fixed the bug and why the fix was necessary
- We realized that we never set the return parameter accepted_fd in 
  ServerSocket::Accept, leaving its value in an uninitalized state.
  This explained why the test only failed sometimes. The fix was
  simply to return client_fd through the return parameter.


# Bug 2

## A) How is your program acting differently than you expect it to?
- The server segfaults when we try to GET a file that doesn't exist.

## B) Brainstorm a few possible causes of the bug
- The segfault error message seems to be related to std::string. We could be
  passing null into a string or trying to access a character out of bounds.
- realpath might not work with nonexistent paths.
- Might be a problem in FileReader.

## C) How you fixed the bug and why the fix was necessary
- We ran the server with gdb and found that realpath() returns nullptr
  when the path doesn't exist, but we were passing the result directly
  into the constructor of std::string. The fix was to check the result
  of realpath() for nullptr first before making the strings in IsPathSafe.


# Bug 3

## A) How is your program acting differently than you expect it to?
- The server segfaults only on the second GET request when connected 
  through telnet.

## B) Brainstorm a few possible causes of the bug
- We just added a nullptr check in IsPathSafe, but we might be misunderstanding
  the functionality of realpath and causing a segfault.
- The behavior is inconsistent, so it might be an uninitialized value.
- It seems to always be on the second query, so the cause should be something
  that is used across queries.

## C) How you fixed the bug and why the fix was necessary
- We ran the server with gdb and found that the crash was happening in
  HttpConnection::GetNextRequest and not in HttpServer like we thought.
  Printing buffer_ revealed that some queries (like the second) from telnet
  started with "\r\n" due to pressing the Enter key. Our parsing code
  didn't work correctly with the leading whitespace and crashed. To fix
  this, we just added some boost::trim calls in HttpConnection::ParseRequest.
