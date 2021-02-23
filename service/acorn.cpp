// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Acorn
// -------------------------------------
// file       : acorn.cpp
// author     : Ben Kietzman
// begin      : 2018-12-18
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

/*! \file acorn.cpp
* \brief Acorn
*
* Provides an interface to Acorn.
*/

#include <cerrno>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;
#include <Json>
#include <StringManip>
#include <Utility>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  list<Json *> request, response;
  string strError, strJson, strPort = "22676", strServer = "localhost";
  Utility utility(strError);
  Json *ptJson = NULL;
  StringManip manip;

  loadRequest(request);
  if (request.size() >= 1 && request.size() <= 2)
  {
    bool bConnected[3] = {false, false, false};
    int fdSocket, nReturn;
    struct addrinfo hints, *result;
    ptJson = new Json(request.front());
    if (ptJson->m.find("Server") != ptJson->m.end() && !ptJson->m["Server"]->v.empty())
    {
      strServer = ptJson->m["Server"]->v;
    }
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    if ((nReturn = getaddrinfo(strServer.c_str(), strPort.c_str(), &hints, &result)) == 0)
    {
      bConnected[0] = true;
      for (struct addrinfo *rp = result; !bConnected[1] && rp != NULL; rp = rp->ai_next)
      {
        bConnected[1] = false;
        if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
        {
          bConnected[1] = true;
          if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
          {
            bConnected[2] = true;
          }
          else
          {
            close(fdSocket);
          }
        }
      }
      freeaddrinfo(result);
    }
    if (bConnected[2])
    {
      string strLine;
      Json *ptRequest = new Json(request.front());
      if (request.size() == 2 && ptRequest->m.find("Request") == ptRequest->m.end())
      {
        ptRequest->m["Request"] = new Json(request.back());
      }
      ptRequest->json(strLine);
      strLine += "\n";
      if (write(fdSocket, strLine.c_str(), strLine.size()) == (ssize_t)strLine.size())
      {
        string strResponse;
        if (utility.getLine(fdSocket, strLine, 5000, nReturn))
        {
          Json *ptResponse[2] = {NULL, NULL};
          bProcessed = true;
          ptResponse[0] = new Json(strLine);
          if (ptResponse[0]->m.find("Response") != ptResponse[0]->m.end())
          {
            ptResponse[1] = new Json(ptResponse[0]->m["Response"]);
            delete ptResponse[0]->m["Response"];
            ptResponse[0]->m.erase("Response");
          }
          response.push_back(ptResponse[0]);
          if (ptResponse[1] != NULL)
          {
            response.push_back(ptResponse[1]);
          }
        }
        else
        {
          stringstream ssError;
          ssError << "Utility::getline(" << errno << ") error:  " << strerror(errno);
          strError = ssError.str();
        }
      }
      else
      {
        stringstream ssError;
        ssError << "write(" << errno << ") error:  " << strerror(errno);
        strError = ssError.str();
      }
      close(fdSocket);
    }
    else if (!bConnected[0])
    {
      stringstream ssError;
      ssError << "getaddrinfo(" << nReturn << ") error:  " << gai_strerror(nReturn);
      strError = ssError.str();
    }
    else
    {
      stringstream ssError;
      ssError << ((!bConnected[1])?"socket":"connect") << "(" << errno << ") error:  " << strerror(errno);
      strError = ssError.str();
    }
  }
  else
  {
    strError = "Invalid number of lines in the request.";
  }
  for (list<Json *>::iterator i = request.begin(); i != request.end(); i++)
  {
    delete *i;
  }
  request.clear();
  if (ptJson == NULL)
  {
    ptJson = new Json;
  }
  if (!bProcessed)
  {
    ptJson->insert("Status", "error");
    if (!strError.empty())
    {
      ptJson->insert("Error", strError);
    }
    cout << ptJson << endl;
  }
  delete ptJson;
  for (list<Json *>::iterator i = response.begin(); i != response.end(); i++)
  {
    cout << (*i) << endl;
    delete (*i);
  }
  response.clear();

  return 0;
}
