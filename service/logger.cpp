// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Logger
// -------------------------------------
// file       : logger.cpp
// author     : Ben Kietzman
// begin      : 2014-07-24
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

/*! \file logger.cpp
* \brief Logger
*
* Provides a logger service.
*/

#include <cerrno>
#include <ctime>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;
#include <Json>
#include <Utility>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  addrinfo hints, *result;
  bool bCleanRequest = false, bProcessed = false, bWrote = false;
  int nReturn;
  list<Json *> request;
  string strError, strPort = "5646", strServer = "localhost";
  Json *ptJson, *ptRequest;
  Utility utility(strError);

  loadRequest(request);
  ptRequest = request.front();
  ptJson = utility.conf();
  if (ptJson->m.find("Logger") != ptJson->m.end() && !ptJson->m["Logger"]->v.empty())
  {
    strServer = ptJson->m["Logger"]->v;
  }
  memset(&hints, 0, sizeof(addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  if ((nReturn = getaddrinfo(strServer.c_str(), strPort.c_str(), &hints, &result)) == 0)
  {
    addrinfo *rp;
    bool bConnected[2] = {false, false};
    int fdSocket;
    for (rp = result; !bConnected[1] && rp != NULL; rp = rp->ai_next)
    {
      bConnected[0] = false;
      if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
      {
        bConnected[0] = true;
        if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
        {
          bConnected[1] = true;
        }
        else
        {
          close(fdSocket);
        }
      }
    }
    freeaddrinfo(result);
    if (bConnected[1])
    {
      bool bConvertedTime = true;
      if (ptRequest->m.find("Search") != ptRequest->m.end() && ptRequest->m["Search"]->m.find("Time") != ptRequest->m["Search"]->m.end())
      {
        for (map<string, Json *>::iterator i = ptRequest->m["Search"]->m["Time"]->m.begin(); i != ptRequest->m["Search"]->m["Time"]->m.end(); i++)
        {
          if (!i->second->v.empty())
          {
            struct tm tTime;
            if (strptime(i->second->v.c_str(), "%Y-%m-%d %H:%M:%S", &tTime) != NULL)
            {
              time_t CTime;
              if ((CTime = mktime(&tTime)) != -1)
              {
                stringstream ssTime;
                ssTime << CTime;
                i->second->v = ssTime.str();
              }
              else
              {
                bConvertedTime = false;
                strError = "main()->mktime():  Failed to make time.";
              }
            }
            else
            {
              bConvertedTime = false;
              strError = "main()->strptime():  Failed to parse date/time.  FORMAT:  YYYY-MM-DD HH:MM:SS";
            }
          }
        }
      }
      if (bConvertedTime)
      {
        bool bExit = false, bFirst = true;
        char szBuffer[65536];
        size_t unPosition;
        string strBuffer[2];
        ptRequest->json(strBuffer[1]);
        strBuffer[1] += "\n";
        while (!bExit)
        {
          pollfd fds[1];
          fds[0].fd = fdSocket;
          fds[0].events = POLLIN;
          if (!strBuffer[1].empty())
          {
            fds[0].events |= POLLOUT;
          }
          if ((nReturn = poll(fds, 1, 250)) > 0)
          {
            if (fds[0].revents & POLLIN)
            {
              if ((nReturn = read(fdSocket, szBuffer, 65536)) > 0)
              {
                strBuffer[0].append(szBuffer, nReturn);
                while ((unPosition = strBuffer[0].find("\n")) != string::npos)
                {
                  if (bFirst)
                  {
                    bFirst = false;
                    bCleanRequest = true;
                    ptRequest = new Json(strBuffer[0].substr(0, unPosition));
                    strBuffer[0].erase(0, (unPosition + 1));
                    if (ptRequest->m.find("Function") != ptRequest->m.end() && (ptRequest->m["Function"]->v == "log" || ptRequest->m["Function"]->v == "message" || ptRequest->m["Function"]->v == "search"))
                    {
                      if (ptRequest->m.find("Search") != ptRequest->m.end() && ptRequest->m["Search"]->m.find("Time") != ptRequest->m["Search"]->m.end())
                      {
                        for (map<string, Json *>::iterator i = ptRequest->m["Search"]->m["Time"]->m.begin(); i != ptRequest->m["Search"]->m["Time"]->m.end(); i++)
                        {
                          if (!i->second->v.empty())
                          {
                            stringstream  ssTime(i->second->v);
                            struct tm tTime;
                            time_t CTime;
                            ssTime >> CTime;
                            if (localtime_r(&CTime, &tTime) != NULL)
                            {
                              char szTimeStamp[20] = "\0";
                              if (strftime(szTimeStamp, 20, "%Y-%m-%d %H:%M:%S", &tTime) > 0)
                              {
                                i->second->v = szTimeStamp;
                              }
                            }
                          }
                        }
                      }
                      if (ptRequest->m.find("Time") != ptRequest->m.end() && !ptRequest->m["Time"]->v.empty())
                      {
                        stringstream  ssTime(ptRequest->m["Time"]->v);
                        struct tm tTime;
                        time_t CTime;
                        ssTime >> CTime;
                        if (localtime_r(&CTime, &tTime) != NULL)
                        {
                          char szTimeStamp[20] = "\0";
                          if (strftime(szTimeStamp, 20, "%Y-%m-%d %H:%M:%S", &tTime) > 0)
                          {
                            ptRequest->m["Time"]->v = szTimeStamp;
                          }
                        }
                      }
                      if (ptRequest->m.find("Status") != ptRequest->m.end() && ptRequest->m["Status"]->v == "okay")
                      {
                        bProcessed = bWrote = true;
                        cout << ptRequest << endl;
                      }
                    }
                    else
                    {
                      strError = "Please provide a valid Function:  log, message, search.";
                    }
                  }
                  else
                  {
                    ptJson = new Json(strBuffer[0].substr(0, unPosition));
                    strBuffer[0].erase(0, (unPosition + 1));
                    if (ptJson->m.find("Time") != ptJson->m.end() && !ptJson->m["Time"]->v.empty())
                    {
                      stringstream  ssTime(ptJson->m["Time"]->v);
                      struct tm tTime;
                      time_t CTime;
                      ssTime >> CTime;
                      if (localtime_r(&CTime, &tTime) != NULL)
                      {
                        char szTimeStamp[20] = "\0";
                        if (strftime(szTimeStamp, 20, "%Y-%m-%d %H:%M:%S", &tTime) > 0)
                        {
                          ptJson->m["Time"]->v = szTimeStamp;
                        }
                      }
                    }
                    cout << ptJson << endl;
                    delete ptJson;
                  }
                }
              }
              else
              {
                bExit = true;
                if (nReturn < 0)
                {
                  stringstream ssError;
                  ssError << "main()->read(" << errno << "):  " << strerror(errno);
                  strError = ssError.str();
                }
              }
            }
            if (fds[0].revents & POLLOUT)
            {
              if ((nReturn = write(fdSocket, strBuffer[1].c_str(), strBuffer[1].size())) > 0)
              {
                strBuffer[1].erase(0, nReturn);
              }
              else
              {
                bExit = true;
                if (nReturn < 0)
                {
                  stringstream ssError;
                  ssError << "main()->write(" << errno << "):  " << strerror(errno);
                  strError = ssError.str();
                }
              }
            }
          }
          else if (nReturn < 0)
          {
            stringstream ssError;
            ssError << "main()->poll(" << errno << "):  " << strerror(errno);
            strError = ssError.str();
          }
        }
      }
      close(fdSocket);
    }
    else
    {
      stringstream ssError;
      ssError << "main()->" << ((!bConnected[0])?"socket":"connect") << "(" << errno << "):  " << strerror(errno);
      strError = ssError.str();
    }
  }
  else
  {
    stringstream ssError;
    ssError << "main()->getaddrinfo(" << nReturn << "):  " << gai_strerror(nReturn);
    strError = ssError.str();
  }
  if (!bWrote)
  {
    if (!bProcessed)
    {
      ptRequest->insert("Status", "error");
      if (!strError.empty())
      {
        ptRequest->insert("Error", strError);
      }
    }
    cout << ptRequest << endl;
  }
  if (bCleanRequest)
  {
    delete ptRequest;
  }
  for (list<Json *>::iterator i = request.begin(); i != request.end(); i++)
  {
    delete (*i);
  }
  request.clear();

  return 0;
}
