// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Transmission Interface
// -------------------------------------
// file       : transmission.cpp
// author     : Ben Kietzman
// begin      : 2013-02-20
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

/*! \file transmission.cpp
* \brief Transmission Interface
*
* A socket level daemon which sends text pages using the WebNotify C library API.
*/
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
using namespace std;
#include <Json>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  map<string, string> requestArray;
  list<map<string, string> > responseArray;
  list<string> request;
  string strError;

  loadRequest(requestArray, request);
  if (requestArray.find("Action") != requestArray.end() && !requestArray["Action"].empty())
  {
    stringstream ssCommand;
    ssCommand << "/usr/bin/transmission-remote";
    if (requestArray.find("Server") != requestArray.end() && !requestArray["Server"].empty())
    {
      ssCommand << " " << requestArray["Server"];
      if (requestArray.find("Port") != requestArray.end() && !requestArray["Port"].empty())
      {
        ssCommand << ":" << requestArray["Port"];
      }
    }
    if (requestArray.find("User") != requestArray.end() && !requestArray["User"].empty())
    {
      ssCommand << " --auth " << requestArray["User"];
      if (requestArray.find("Password") != requestArray.end() && !requestArray["Password"].empty())
      {
        ssCommand << ":" << requestArray["Password"];
      }
    }
    if (requestArray["Action"] == "info")
    {
      FILE *pinProc = NULL;
      string strTorrent = "all";
      if (requestArray.find("Torrent") != requestArray.end() && !requestArray["Torrent"].empty())
      {
        strTorrent = requestArray["Torrent"];
      }
      ssCommand << " --torrent " << strTorrent << " --info";
      if ((pinProc = popen(ssCommand.str().c_str(), "r")) != NULL)
      {
        char szBuffer[1024];
        size_t nPosition, nReturn;
        map<string, string> torrent;
        string strBuffer, strField, strLine;
        while ((nReturn = fread(szBuffer, 1, 1024, pinProc)) > 0)
        {
          strBuffer.append(szBuffer, nReturn);
          while ((nPosition = strBuffer.find("\n", 0)) != string::npos)
          {
            strLine = strBuffer.substr(0, nPosition);
            strBuffer.erase(0, nPosition + 1);
            if (strLine.size() >= 4 && strLine.substr(0, 4) == "NAME")
            {
              if (!torrent.empty())
              {
                bProcessed = true;
                responseArray.push_back(torrent);
                torrent.clear();
              }
            }
            else if (strLine.size() > 2 && strLine.substr(0, 2) == "  " && (nPosition = strLine.find(":", 0)) != string::npos)
            {
              strField = strLine.substr(2, nPosition - 2);
              strLine.erase(0, nPosition + 1);
              while (!strLine.empty() && strLine[0] == ' ')
              {
                strLine.erase(0, 1);
              }
              torrent[strField] = strLine;
            }
          }
        }
        if (!torrent.empty())
        {
          bProcessed = true;
          responseArray.push_back(torrent);
          torrent.clear();
        }
        pclose(pinProc);
      }
      else
      {
        strError = strerror(errno);
      }
    }
  }
  else
  {
    strError = "Please provide the Action.";
  }
  requestArray["Status"] = (string)((bProcessed)?"okay":"error");
  if (!strError.empty())
  {
    requestArray["Error"] = strError;
  }
  ptJson = new Json(requestArray);
  cout << ptJson << endl;
  delete ptJson;
  if (!responseArray.empty())
  {
    for (auto &i : responseArray)
    {
      ptJson = new Json(i);
      cout << ptJson << endl;
      delete ptJson;
      i.clear();
    }
    responseArray.clear();
  }
  requestArray.clear();
  request.clear();

  return 0;
}
