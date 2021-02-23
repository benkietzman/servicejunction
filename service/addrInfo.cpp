// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Address Information
// -------------------------------------
// file       : addrInfo.cpp
// author     : Ben Kietzman
// begin      : 2012-12-09
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
**************************************************************************/

/*! \file addrInfo.cpp
* \brief Address Information Program
*
* Obtains IPv4 and IPv6 address information.
*/
#include <arpa/inet.h>
#include <iostream>
#include <list>
#include <map>
#include <netdb.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;
#include <Json>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  list<Json *> request, response;
  string strError;

  loadRequest(request);
  if (request.size() == 1)
  {
    ptJson = new Json(request.front());
    if (ptJson->m.find("Server") != ptJson->m.end() && !ptJson->m["Server"]->v.empty())
    {
      int nReturn;
      struct addrinfo hints;
      struct addrinfo *result;
      memset(&hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = 0;
      hints.ai_protocol = 0;
      if ((nReturn = getaddrinfo(ptJson->m["Server"]->v.c_str(), NULL, &hints, &result)) == 0)
      {
        struct addrinfo *rp;
        bProcessed = true;
        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
          char szIP[INET6_ADDRSTRLEN];
          string strFamily;
          void *ptr = NULL;
          switch (rp->ai_family)
          {
            case AF_INET  : strFamily = "IPv4"; ptr = &((struct sockaddr_in *)rp->ai_addr)->sin_addr; break;
            case AF_INET6 : strFamily = "IPv6"; ptr = &((struct sockaddr_in6 *)rp->ai_addr)->sin6_addr; break;
          }
          if (ptr != NULL)
          {
            inet_ntop(rp->ai_family, ptr, szIP, sizeof(szIP));
          }
          if (ptJson->m.find(strFamily) == ptJson->m.end())
          {
            ptJson->m[strFamily] = new Json;
          }
          ptJson->m[strFamily]->push_back(szIP);
        }
        freeaddrinfo(result);
      }
      else
      {
        strError = "Unable to retrieve address information.";
      }
    }
    else
    {
      strError = "Please provide the Server name.";
    }
  }
  else
  {
    ptJson = new Json;
    strError = "Invalid number of lines in the request.";
  }
  for (list<Json *>::iterator i = request.begin(); i != request.end(); i++)
  {
    delete *i;
  }
  request.clear();
  ptJson->insert("Status", (string)((bProcessed)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->insert("Error", strError);
  }
  cout << ptJson << endl;
  delete ptJson;
  for (list<Json *>::iterator i = response.begin(); i != response.end(); i++)
  {
    cout << (*i) << endl;
    delete (*i);
  }
  response.clear();

  return 0;
}
