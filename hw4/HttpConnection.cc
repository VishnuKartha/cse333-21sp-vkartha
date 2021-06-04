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

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;

namespace hw4 {

static const char *kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;
static const int kReadLen = 1024;

bool HttpConnection::GetNextRequest(HttpRequest *request) {
  // Use "WrappedRead" to read data into the buffer_
  // instance variable.  Keep reading data until either the
  // connection drops or you see a "\r\n\r\n" that demarcates
  // the end of the request header. Be sure to try and read in
  // a large amount of bytes each time you call WrappedRead.
  //
  // Once you've seen the request header, use ParseRequest()
  // to parse the header into the *request argument.
  //
  // Very tricky part:  clients can send back-to-back requests
  // on the same socket.  So, you need to preserve everything
  // after the "\r\n\r\n" in buffer_ for the next time the
  // caller invokes GetNextRequest()!

  // STEP 1:
  size_t pos = buffer_.find(kHeaderEnd);
  while (pos == string::npos) {
    unsigned char buf[kReadLen];
    int result = WrappedRead(fd_, buf, kReadLen);
    if (result == -1) {
      return false;
    }
    for (int i = 0; i < result; i++) {
      buffer_.push_back(static_cast<char>(buf[i]));
    }
    pos = buffer_.find(kHeaderEnd);
    if (result == 0) {
      break;
    }
  }

  if (pos == string::npos) {
    return false;
  }

  *request = ParseRequest(buffer_.substr(0, pos + kHeaderEndLen));
  buffer_.erase(0, pos + kHeaderEndLen);
  return true;
}

bool HttpConnection::WriteResponse(const HttpResponse &response) const {
  string str = response.GenerateResponseString();
  int res = WrappedWrite(fd_,
                         reinterpret_cast<const unsigned char *>(str.c_str()),
                         str.length());
  if (res != static_cast<int>(str.length()))
    return false;
  return true;
}

HttpRequest HttpConnection::ParseRequest(const string &request) const {
  HttpRequest req("/");  // by default, get "/".

  // Split the request into lines.  Extract the URI from the first line
  // and store it in req.URI.  For each additional line beyond the
  // first, extract out the header name and value and store them in
  // req.headers_ (i.e., HttpRequest::AddHeader).  You should look
  // at HttpRequest.h for details about the HTTP header format that
  // you need to parse.
  //
  // You'll probably want to look up boost functions for (a) splitting
  // a string into lines on a "\r\n" delimiter, (b) trimming
  // whitespace from the end of a string, and (c) converting a string
  // to lowercase.
  //
  // Note that you may assume the request you are parsing is correctly
  // formatted. If for some reason you encounter a header that is
  // malformed, you may skip that line.

  // STEP 2:
  string trimmed = request;
  boost::trim(trimmed);
  vector<string> lines;
  boost::split(lines, trimmed, boost::is_any_of("\r\n"), 
                               boost::token_compress_on);
  vector<string> tokens;
  boost::split(tokens, lines[0], boost::is_any_of(" "), 
                                 boost::token_compress_on);
  boost::trim(tokens[1]);
  req.set_uri(tokens[1]);

  for (size_t i = 1; i < lines.size(); i++) {
    boost::split(tokens, lines[i], boost::is_any_of(": "),
                                   boost::token_compress_on);
    if (tokens.size() != 2) {
      continue;
    }
    boost::algorithm::to_lower(tokens[0]);
    boost::trim(tokens[0]);
    boost::algorithm::to_lower(tokens[1]);
    boost::trim(tokens[1]);
    req.AddHeader(tokens[0], tokens[1]);
  }

  return req;
}

}  // namespace hw4
