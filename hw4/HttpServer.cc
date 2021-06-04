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

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

#include "./FileReader.h"
#include "./HttpConnection.h"
#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpServer.h"
#include "./libhw3/QueryProcessor.h"

using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::map;
using std::string;
using std::to_string;
using std::stringstream;
using std::unique_ptr;

namespace hw4 {
///////////////////////////////////////////////////////////////////////////////
// Constants, internal helper functions
///////////////////////////////////////////////////////////////////////////////
static const char *kThreegleStr =
  "<html><head><title>333gle</title></head>\n"
  "<body>\n"
  "<center style=\"font-size:500%;\">\n"
  "<span style=\"position:relative;bottom:-0.33em;color:orange;\">3</span>"
    "<span style=\"color:red;\">3</span>"
    "<span style=\"color:gold;\">3</span>"
    "<span style=\"color:blue;\">g</span>"
    "<span style=\"color:green;\">l</span>"
    "<span style=\"color:red;\">e</span>\n"
  "</center>\n"
  "<p>\n"
  "<div style=\"height:20px;\"></div>\n"
  "<center>\n"
  "<form action=\"/query\" method=\"get\">\n"
  "<input type=\"text\" size=30 name=\"terms\" />\n"
  "<input type=\"submit\" value=\"Search\" />\n"
  "</form>\n"
  "</center><p>\n";

// static
const int HttpServer::kNumThreads = 100;

static const char *kDefaultContentType = "text/plain";
static const std::map<string, string> kContentTypes = {
  { "css", "text/css" },
  { "gif", "image/gif" },
  { "html", "text/html" },
  { "htm", "text/html" },
  { "jpg", "image/jpeg" },
  { "jpeg", "image/jpeg" },
  { "js", "text/javascript" },
  { "png", "image/png" },
  { "txt", "text/plain" },
  { "xml", "text/xml" }
};

// This is the function that threads are dispatched into
// in order to process new client connections.
static void HttpServer_ThrFn(ThreadPool::Task *t);

// Given a request, produce a response.
static HttpResponse ProcessRequest(const HttpRequest &req,
                            const string &base_dir,
                            const list<string> *indices);

// Process a file request.
static HttpResponse ProcessFileRequest(const string &uri,
                                const string &base_dir);

// Process a query request.
static HttpResponse ProcessQueryRequest(const string &uri,
                                 const list<string> *indices);

static string GetContentType(const string &uri);

static bool IsWebLink(const string &document_name);


///////////////////////////////////////////////////////////////////////////////
// HttpServer
///////////////////////////////////////////////////////////////////////////////
bool HttpServer::Run(void) {
  // Create the server listening socket.
  int listen_fd;
  cout << "  creating and binding the listening socket..." << endl;
  if (!socket_.BindAndListen(AF_INET6, &listen_fd)) {
    cerr << endl << "Couldn't bind to the listening socket." << endl;
    return false;
  }

  // Spin, accepting connections and dispatching them.  Use a
  // threadpool to dispatch connections into their own thread.
  cout << "  accepting connections..." << endl << endl;
  ThreadPool tp(kNumThreads);
  while (1) {
    HttpServerTask *hst = new HttpServerTask(HttpServer_ThrFn);
    hst->base_dir = static_file_dir_path_;
    hst->indices = &indices_;
    if (!socket_.Accept(&hst->client_fd,
                    &hst->c_addr,
                    &hst->c_port,
                    &hst->c_dns,
                    &hst->s_addr,
                    &hst->s_dns)) {
      // The accept failed for some reason, so quit out of the server.
      // (Will happen when kill command is used to shut down the server.)
      break;
    }
    // The accept succeeded; dispatch it.
    tp.Dispatch(hst);
  }
  return true;
}

static void HttpServer_ThrFn(ThreadPool::Task *t) {
  // Cast back our HttpServerTask structure with all of our new
  // client's information in it.
  unique_ptr<HttpServerTask> hst(static_cast<HttpServerTask *>(t));
  cout << "  client " << hst->c_dns << ":" << hst->c_port << " "
       << "(IP address " << hst->c_addr << ")" << " connected." << endl;

  // Read in the next request, process it, write the response.

  // Use the HttpConnection class to read and process the next
  // request from our current client, then write out our response.  If
  // the client sends a "Connection: close\r\n" header, then shut down
  // the connection -- we're done.
  //
  // Hint: the client can make multiple requests on our single connection,
  // so we should keep the connection open between requests rather than
  // creating/destroying the same connection repeatedly.

  // STEP 1:
  HttpConnection conn(hst->client_fd);
  while (true) {
    HttpRequest req;
    if (!conn.GetNextRequest(&req)) {
      break;
    } else if (req.GetHeaderValue("connection") == "close\r\n") {
      break;
    }

    HttpResponse res = ProcessRequest(req, hst->base_dir, hst->indices);
    if (!conn.WriteResponse(res)) {
      break;
    }
  }
}

static HttpResponse ProcessRequest(const HttpRequest &req,
                            const string &base_dir,
                            const list<string> *indices) {
  // Is the user asking for a static file?
  if (req.uri().substr(0, 8) == "/static/") {
    return ProcessFileRequest(req.uri(), base_dir);
  }

  // The user must be asking for a query.
  return ProcessQueryRequest(req.uri(), indices);
}

static HttpResponse ProcessFileRequest(const string &uri,
                                const string &base_dir) {
  // The response we'll build up.
  HttpResponse ret;

  // Steps to follow:
  //  - use the URLParser class to figure out what filename
  //    the user is asking for. Note that we identify a request
  //    as a file request if the URI starts with '/static/'
  //
  //  - use the FileReader class to read the file into memory
  //
  //  - copy the file content into the ret.body
  //
  //  - depending on the file name suffix, set the response
  //    Content-type header as appropriate, e.g.,:
  //      --> for ".html" or ".htm", set to "text/html"
  //      --> for ".jpeg" or ".jpg", set to "image/jpeg"
  //      --> for ".png", set to "image/png"
  //      etc.
  //    You should support the file types mentioned above,
  //    as well as ".txt", ".js", ".css", ".xml", ".gif",
  //    and any other extensions to get bikeapalooza
  //    to match the solution server.
  //
  // be sure to set the response code, protocol, and message
  // in the HttpResponse as well.

  // STEP 2:
  URLParser parser;
  parser.Parse(uri);

  string file_name = parser.path();
  const string kReqIdentifier = "/static/";
  file_name = file_name.substr(file_name.find(kReqIdentifier) + 
                               kReqIdentifier.size());
  
  FileReader reader(base_dir, file_name);
  string contents;

  if (IsPathSafe(base_dir, base_dir + "/" + file_name) 
      && reader.ReadFile(&contents)) {
    ret.set_protocol("HTTP/2");
    ret.set_response_code(200);
    ret.set_message("OK");
    ret.set_content_type(GetContentType(file_name));
    ret.AppendToBody(contents);
  } else {
    // If you couldn't find the file, return an HTTP 404 error.
    ret.set_protocol("HTTP/1.1");
    ret.set_response_code(404);
    ret.set_message("Not Found");
    ret.AppendToBody("<html><body>Couldn't find file \""
                    + EscapeHtml(file_name)
                    + "\"</body></html>");
  }
  return ret;
}

static HttpResponse ProcessQueryRequest(const string &uri,
                                 const list<string> *indices) {
  // The response we're building up.
  HttpResponse ret;

  // Your job here is to figure out how to present the user with
  // the same query interface as our solution_binaries/http333d server.
  // A couple of notes:
  //
  //  - no matter what, you need to present the 333gle logo and the
  //    search box/button
  //
  //  - if the user had previously typed in a search query, you also
  //    need to display the search results.
  //
  //  - you'll want to use the URLParser to parse the uri and extract
  //    search terms from a typed-in search query.  convert them
  //    to lower case.
  //
  //  - you'll want to create and use a hw3::QueryProcessor to process
  //    the query against the search indices
  //
  //  - in your generated search results, see if you can figure out
  //    how to hyperlink results to the file contents, like we did
  //    in our solution_binaries/http333d.

  // STEP 3:
  ret.set_protocol("HTTP/2");
  ret.set_response_code(200);
  ret.set_message("OK");
  ret.set_content_type("text/html");
  ret.AppendToBody(string(kThreegleStr));

  URLParser parser;
  parser.Parse(uri);
  const auto args = parser.args();
  if (auto it = args.find("terms"); it != args.end()) {
    vector<string> terms;
    string escaped_terms = EscapeHtml(it->second);
    boost::split(terms, escaped_terms, boost::is_any_of(" "), 
                                    boost::token_compress_on);

    hw3::QueryProcessor processor(*indices, false);
    const auto results = processor.ProcessQuery(terms);

    string result_count = "<p><br>" + to_string(results.size())
                          + " results found for <b>" + escaped_terms 
                          + "</b></p>\n";
    ret.AppendToBody(result_count);
    ret.AppendToBody("<ul>");

    for (const auto &res : results) {
      string li = "<li> <a href=\"";
      if (!IsWebLink(res.document_name)) {
        li += "/static/";
      }
      li += res.document_name + "\">" 
                  + res.document_name + "</a> [" + to_string(res.rank) 
                  + "]<br>\n";
      ret.AppendToBody(li);
    }
    ret.AppendToBody("</ul>\n</body>\n</html>\n");
  }

  return ret;
}

static string GetContentType(const string &uri) {
  size_t extension_pos = uri.find_last_of(".");
  if (extension_pos == string::npos) {
    return kDefaultContentType;
  }

  string extension = uri.substr(extension_pos + 1);
  if (auto it = kContentTypes.find(extension); it != kContentTypes.end()) {
    return it->second;
  } else {
    return kDefaultContentType;
  }
}

static bool IsWebLink(const string &document_name) {
  return document_name.find("http") == 0;
}

}  // namespace hw4
