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

#include <stdio.h>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <string.h>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.
#include <sstream>       // for std::sstream
#include <string>      


#include "./ServerSocket.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw4 {

// According to https://devblogs.microsoft.com/oldnewthing/20120412-00/?p=7873,
// even 256 is enough. We'll use 512 to be extra safe.
static constexpr int kDNSLength = 512;

// Gets the IP address and port from a sockaddr struct. On success, 
// returns true and sets the return parameters. Returns false on failure.
static bool RetrieveAddrPort(struct sockaddr *addr, std::string *ret_addr,
                             uint16_t *ret_port);

// Does a reverse DNS lookup on the given addr struct. On success, returns true 
// and sets the dns_name return parameter. Returns false on failure.
static bool ReverseDNSLookup(struct sockaddr *addr, size_t addrlen, 
                             std::string *dns_name);

// Gets the IP address and DNS name of the server. Returns false on failure.
// On success, returns true and sets the return parameters.
static bool ExtractServerInfo(int client_fd, int sock_family, 
                              std::string *ret_addr, 
                              std::string *ret_dns_name);


ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::BindAndListen(int ai_family, int *listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd"
  // and set the ServerSocket data member "listen_sock_fd_"

  // Populate the "hints" addrinfo structure for getaddrinfo().
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = ai_family;       // IPv6 (also handles IPv4 clients)
  hints.ai_socktype = SOCK_STREAM;   // stream
  hints.ai_flags = AI_PASSIVE;       // use wildcard "in6addr_any" address
  hints.ai_flags |= AI_V4MAPPED;     // use v4-mapped v6 if no v6 found
  hints.ai_protocol = IPPROTO_TCP;   // tcp protocol
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  // Convert port to string.
  std::string port = std::to_string(port_);

  // Address structures via the output parameter "result".
  struct addrinfo *result;
  int res = getaddrinfo(nullptr, port.c_str(), &hints, &result);

  // Did addrinfo() fail?
  if (res != 0) {
    return false;
  }

  // Loop through the returned address structures until we are able
  // to create a socket and bind to one.  The address structures are
  // linked in a list through the "ai_next" field of result.
  int fd = -1;
  for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
    fd = socket(rp->ai_family,
                       rp->ai_socktype,
                       rp->ai_protocol);
    if (fd == -1) {
      // Creating this socket failed.  So, loop to the next returned
      // result and try again.
      fd = -1;
      continue;
    }

    // Configure the socket; we're setting a socket "option."  In
    // particular, we set "SO_REUSEADDR", which tells the TCP stack
    // so make the port we bind to available again as soon as we
    // exit, rather than waiting for a few tens of seconds to recycle it.
    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof(optval));

    // Try binding the socket to the address and port number returned
    // by getaddrinfo().
    if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0) {
      // Return to the caller the address family.
      sock_family_ = rp->ai_family;
      break;
    }

    // The bind failed.  Close the socket, then loop back around and
    // try the next address/port returned by getaddrinfo().
    close(fd);
    fd = -1;
  }
  // Free the structure returned by getaddrinfo().
  freeaddrinfo(result);

  // If we failed to bind, return failure.
  if (fd == -1)
    return false;

  // Success. Tell the OS that we want this to be a listening socket.
  if (listen(fd, SOMAXCONN) != 0) {
    close(fd);
    return false;
  }

  // succesfully bound
  *listen_fd = fd;
  listen_sock_fd_ = fd;

  // Return that a listening socket was correctly bound.
  return true;
}

bool ServerSocket::Accept(int *accepted_fd,
                          std::string *client_addr,
                          uint16_t *client_port,
                          std::string *client_dns_name,
                          std::string *server_addr,
                          std::string *server_dns_name) const {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // STEP 2:
  while (true) {
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    int client_fd = accept(listen_sock_fd_,
                            reinterpret_cast<struct sockaddr *>(&caddr),
                            &caddr_len);
    if (client_fd < 0) {
      if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        continue;
      } else {
        return false;
      }
    }

    // Past this point, we're just retrieving some information and don't
    // really want to break the connection if anything fails. We will
    // set return values to default values in the case of failure.
    if (!RetrieveAddrPort(reinterpret_cast<struct sockaddr *>(&caddr),
                          client_addr, client_port)) {
      *client_addr = "";
      *client_port = 0;
    }
    if (!ReverseDNSLookup(reinterpret_cast<struct sockaddr *>(&caddr), caddr_len, 
                          client_dns_name)) {
      // Default to client_addr if this lookup failed.
      *client_dns_name = *client_addr;
    }
    if (!ExtractServerInfo(client_fd, sock_family_, server_addr, 
                           server_dns_name)) {
      *server_addr = "";
      *server_dns_name = *server_addr;
    }
    *accepted_fd = client_fd;
    return true;    
  } 
  
}

static bool RetrieveAddrPort(struct sockaddr *addr, std::string *ret_addr,
                             uint16_t *ret_port) {
  std::stringstream ss;
  if (addr->sa_family == AF_INET) {
    // Extract the IPV4 address and port.
    char astring[INET_ADDRSTRLEN];
    struct sockaddr_in *in4 = reinterpret_cast<struct sockaddr_in *>(addr);
    inet_ntop(AF_INET, &(in4->sin_addr), astring, INET_ADDRSTRLEN);
    ss << astring;
    *ret_addr = ss.str();
    *ret_port = ntohs(in4->sin_port);

  } else if (addr->sa_family == AF_INET6) {
    // Extract the IPV6 address and port.
    char astring[INET6_ADDRSTRLEN];
    struct sockaddr_in6 *in6 = reinterpret_cast<struct sockaddr_in6 *>(addr);
    inet_ntop(AF_INET6, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
    ss << astring;
    *ret_addr = ss.str();
    *ret_port = ntohs(in6->sin6_port);

  } else {
    // Retrieving info failed.
    return false;
  }
  return true;
}

static bool ReverseDNSLookup(struct sockaddr *addr, size_t addrlen, 
                             std::string *dns_name) {
  char hostname[kDNSLength];
  if (getnameinfo(addr, addrlen, hostname, kDNSLength, nullptr, 0, 0) != 0) {
    return false;
  } else {
    std::stringstream ss;
    ss << hostname;
    *dns_name = ss.str();
    return true;
  }
}

static bool ExtractServerInfo(int client_fd, int sock_family, 
                              std::string *ret_addr, 
                              std::string *ret_dns_name) {
  
  // Determine which sockaddr_* struct to use based on the sock_family.
  struct sockaddr_in srvr;
  struct sockaddr_in6 srvr6;
  struct sockaddr *addr;
  socklen_t addr_len;
  if (sock_family == AF_INET) {
    addr_len = sizeof(srvr);
    addr = reinterpret_cast<struct sockaddr *>(&srvr);
  } else {
    addr_len = sizeof(srvr6);
    addr = reinterpret_cast<struct sockaddr *>(&srvr6);
  }

  // Get the sockaddr struct for our side of the connection.
  if (getsockname(client_fd, addr, &addr_len) == -1) {
    return false;
  }
  uint16_t port;
  RetrieveAddrPort(addr, ret_addr, &port);
  // Get the server's dns name, or return it's IP address as
  // a substitute if the dns lookup fails.
  if (!ReverseDNSLookup(addr, addr_len, ret_dns_name)) {
    *ret_dns_name = *ret_addr;
  }
  return true;
}

}  // namespace hw4
