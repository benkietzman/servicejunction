// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// MythTV
// -------------------------------------
// file       : mythtv.cpp
// author     : Ben Kietzman
// begin      : 2025-04-24
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

/*! \file mythtv.cpp
* \brief MythTV Interface
*
* Interfaces with MythTV via their Restful API.
*/
#include <arpa/inet.h>
#include <iostream>
#include <list>
#include <map>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;
#include <Json>
#include <ServiceJunction>
#include <StringManip>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  list<Json *> request, response;
  string strError;
  ServiceJunction junction(strError);
  StringManip manip;

  setlocale(LC_ALL, "");
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  loadRequest(request);
  if (request.size() == 2)
  {
    response.push_back(new Json(request.back()));
    ptJson = new Json(request.front());
    if (ptJson->m.find("reqApp") != ptJson->m.end() && !ptJson->m["reqApp"]->v.empty())
    {
      junction.setApplication(ptJson->m["reqApp"]->v);
    }
    if (ptJson->m.find("Server") != ptJson->m.end() && !ptJson->m["Server"]->v.empty())
    {
      string strPort = "6544", strProxy;
      stringstream ssURL;
      Json *ptReq = new Json(request.back());
      ssURL << "http://" << ptJson->m["Server"]->v;
      if (ptJson->m.find("Port") != ptJson->m.end() && !ptJson->m["Port"]->v.empty())
      {
        strPort = ptJson->m["Port"]->v;
      }
      if (ptJson->m.find("Proxy") != ptJson->m.end() && !ptJson->m["Proxy"]->v.empty())
      {
        strProxy = ptJson->m["Proxy"]->v;
      }
      ssURL << ":" << strPort;
      if (ptReq->m.find("Service") != ptReq->m.end() && !ptReq->m["Service"]->v.empty())
      {
        ssURL << "/" << ptReq->m["Service"]->v;
        if (ptReq->m.find("Command") != ptReq->m.end() && !ptReq->m["Command"]->v.empty())
        {
          string strContent, strCookies, strHeader;
          ssURL << "/" << ptReq->m["Command"]->v;
          if (junction.curl(ssURL.str(), "", NULL, ((ptReq->m.find("Get") != ptReq->m.end())?ptReq->m["Get"]:NULL), ((ptReq->m.find("Post") != ptReq->m.end())?ptReq->m["Post"]:NULL), ((ptReq->m.find("Put") != ptReq->m.end())?ptReq->m["Put"]:NULL), strProxy, strCookies, strHeader, strContent, strError))
          {
            size_t unPosition;
            if ((unPosition = strContent.find("?>")) != string::npos)
            {
              bProcessed = true;
              strContent.erase(0, (unPosition + 2));
              response.push_back(new Json(strContent));
            }
            else
            {
              strError = "Invalid response.";
            }
          }
        }
        else
        {
          strError = "Please provide the Command.";
        }
      }
      else
      {
        strError = "Please provide the Service.";
      }
      delete ptReq;
    }
    else
    {
      strError = "Please provide the Server.";
    }
  }
  else
  {
    ptJson = new Json;
    strError = "Invalid number of lines in the request.";
  }
  for (auto &i : request)
  {
    delete i;
  }
  request.clear();
  ptJson->insert("Status", (string)((bProcessed)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->insert("Error", strError);
  }
  cout << ptJson << endl;
  delete ptJson;
  for (auto &i : response)
  {
    cout << i << endl;
    delete i;
  }
  response.clear();

  return 0;
}
