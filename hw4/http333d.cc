/*
 * Copyright ©2021 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2021 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <list>

#include "./ServerSocket.h"
#include "./HttpServer.h"

using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::string;
namespace fs = std::filesystem;

// Print out program usage, and exit() with EXIT_FAILURE.
static void Usage(char *prog_name);

// Check if the file at path is a readable file.
static bool IsReadable(const fs::path &p);

// Parses the command-line arguments, invokes Usage() on failure.
// "port" is a return parameter to the port number to listen on,
// "path" is a return parameter to the directory containing
// our static files, and "indices" is a return parameter to a
// list of index filenames.  Ensures that the path is a readable
// directory, and the index filenames are readable, and if not,
// invokes Usage() to exit.
static void GetPortAndPath(int argc,
                    char **argv,
                    uint16_t *port,
                    string *path,
                    list<string> *indices);

int main(int argc, char **argv) {
  // Print out welcome message.
  cout << "Welcome to http333d, the UW cse333 web server!" << endl;
  cout << "  Copyright 2012 Steven Gribble" << endl;
  cout << "  http://www.cs.washington.edu/homes/gribble" << endl;
  cout << endl;
  cout << "initializing:" << endl;
  cout << "  parsing port number and static files directory..." << endl;

  // Ignore the SIGPIPE signal, otherwise we'll crash out if a client
  // disconnects unexpectedly.
  signal(SIGPIPE, SIG_IGN);

  // Get the port number and list of index files.
  uint16_t port_num;
  string static_dir;
  list<string> indices;
  GetPortAndPath(argc, argv, &port_num, &static_dir, &indices);
  cout << "    port: " << port_num << endl;
  cout << "    path: " << static_dir << endl;

  // Run the server.
  hw4::HttpServer hs(port_num, static_dir, indices);
  if (!hs.Run()) {
    cerr << "  server failed to run!?" << endl;
  }

  cout << "server completed!  Exiting." << endl;
  return EXIT_SUCCESS;
}


static void Usage(char *prog_name) {
  cerr << "Usage: " << prog_name << " port staticfiles_directory indices+";
  cerr << endl;
  exit(EXIT_FAILURE);
}

static bool IsReadable(const fs::path &p) {
  // https://en.cppreference.com/w/cpp/filesystem/perms
  std::error_code ec;
  auto perms = fs::status(p, ec).permissions();
  return (perms & fs::perms::owner_read) != fs::perms::none &&
         (perms & fs::perms::group_read) != fs::perms::none &&
         (perms & fs::perms::others_read) != fs::perms::none;
}

static void GetPortAndPath(int argc,
                    char **argv,
                    uint16_t *port,
                    string *path,
                    list<string> *indices) {
  // Be sure to check a few things:
  //  (a) that you have a sane number of command line arguments
  //  (b) that the port number is reasonable
  //  (c) that "path" (i.e., argv[2]) is a readable directory
  //  (d) that you have at least one index, and that all indices
  //      are readable files.

  // STEP 1:

  // Check for at least 4 arguments (prog name, port, static files, indices).
  if (argc < 4) {
    Usage(argv[0]);
  }

  // Check that the port number fits in a uint16_t and is four digits.
  if (sscanf(argv[1], "%hu", port) != 1) {
    cerr << "Couldn't parse port" << endl;
    Usage(argv[0]);
  }
  if (*port < 1000 || *port > 9999) {
    cerr << "Expected port to be in the range [1000, 9999]" << endl;
    Usage(argv[0]);
  }

  // Check that the given path is a readable directory.
  if (!fs::is_directory(fs::status(argv[2]))) {
    cerr << "Couldn't read from the given directory" << endl;
    Usage(argv[0]);
  }
  *path = argv[2];

  // Check that all indices are readable files.
  for (int arg = 3; arg < argc; arg++) {
    std::error_code ec;
    auto path = fs::path(argv[arg]);
    if (!fs::exists(path) || !IsReadable(path) || !fs::is_regular_file(path)) {
      cerr << "Couldn't read from index file '" << argv[arg] << "'" << endl;
      Usage(argv[0]);
    }
    indices->push_back(string(argv[arg]));
  }
}
