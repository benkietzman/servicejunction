// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Port Concentrator
// -------------------------------------
// file       : portconcentrator.cpp
// author     : Ben Kietzman
// begin      : 2014-02-27
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

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <netdb.h>
#include <poll.h>
#include <sstream>
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
  Json *ptJson;
  map<string, string> requestArray;
  list<string> request, response;
  string strError, strLine;
  stringstream ssMessage;
  Utility utility(strError);

  setlocale(LC_ALL, "");
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  loadRequest(requestArray, request);
  if (requestArray.find("SubService") != requestArray.end() && !requestArray["SubService"].empty() && requestArray.find("Throttle") != requestArray.end() && !requestArray["Throttle"].empty())
  {
    ifstream inFile;
    string strConf = "/etc/central.conf";
    Json *ptConf = NULL;
    if (requestArray.find("Conf") != requestArray.end() && !requestArray["Conf"].empty())
    {
      strConf = requestArray["Conf"];
    }
    inFile.open(strConf.c_str());
    if (inFile.good())
    {
      string strLine;
      if (getline(inFile, strLine))
      {
        ptConf = new Json(strLine);
      }
    }
    inFile.close();
    if (ptConf != NULL)
    {
      if ((requestArray.find("Port Concentrator") == requestArray.end() || requestArray["Port Concentrator"].empty()) && ptConf->m.find("Port Concentrator") != ptConf->m.end() && !ptConf->m["Port Concentrator"]->v.empty())
      {
        requestArray["Port Concentrator"] = ptConf->m["Port Concentrator"]->v;
      }
      delete ptConf;
    }
    if (requestArray.find("Port Concentrator") != requestArray.end() && !requestArray["Port Concentrator"].empty())
    {
      SSL_CTX *ctx;
      if ((ctx = utility.sslInitClient(strError)) != NULL)
      {
        bool bConnected = false;
        for (int i = 0; !bConnected && i < 2; i++)
        {
          int fdSocket, nReturn;
          SSL *ssl;
          string strServer, strPort;
          struct addrinfo hints, *result;
          strError.clear();
          SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
          memset(&hints, 0, sizeof(struct addrinfo));
          hints.ai_family = AF_UNSPEC;
          hints.ai_socktype = SOCK_STREAM;
          hints.ai_flags = 0;
          hints.ai_protocol = 0;
          if (i == 0)
          {
            strServer = requestArray["Port Concentrator"];
            strPort = "7678";
          }
          else
          {
            strServer = "localhost";
            strPort = "5864";
          }
          if ((nReturn = getaddrinfo(strServer.c_str(), strPort.c_str(), &hints, &result)) == 0)
          {
            bool bSocket = false;
            struct addrinfo *rp;
            for (rp = result; !bConnected && rp != NULL; rp = rp->ai_next)
            {
              bSocket = false;
              if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
              {
                bSocket = true;
                if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                {
                  bConnected = true;
                }
                else
                {
                  close(fdSocket);
                }
              }
            }
            freeaddrinfo(result);
            if (bConnected)
            {
              bool bExit = false;
              size_t unPosition;
              string strBuffers[3], strTrim;
              StringManip manip;
              if (i == 0)
              {
                ptJson = new Json(request.front());
                if (ptJson->m.find("SubService") != ptJson->m.end() && !ptJson->m["SubService"]->v.empty())
                {
                  ptJson->insert("Service", ptJson->m["SubService"]->v);
                  delete ptJson->m["SubService"];
                  ptJson->m.erase("SubService");
                }
                ptJson->json(strBuffers[1]);
                delete ptJson;
                strBuffers[1].append("\n");
              }
              request.pop_front();
              for (auto &i : request)
              {
                strBuffers[2].append(i + "\n");
              }
              strBuffers[2].append("end\n");
              while (!bExit)
              {
                pollfd fds[1];
                fds[0].fd = fdSocket;
                fds[0].events = POLLIN;
                if (!strBuffers[1].empty() || !strBuffers[2].empty())
                {
                  fds[0].events |= POLLOUT;
                }
                if ((nReturn = poll(fds, 1, 250)) > 0)
                {
                  if (fds[0].revents & POLLIN)
                  {
                    if (utility.sslRead(ssl, strBuffers[0], nReturn))
                    {
                      while (!bExit && (unPosition = strBuffers[0].find("\n")) != string::npos)
                      {
                        manip.trim(strTrim, (strLine = strBuffers[0].substr(0, unPosition)));
                        strBuffers[0].erase(0, (unPosition + 1));
                        if (strTrim == "end")
                        {
                          bExit = bProcessed = true;
                        }
                        else
                        {
                          response.push_back(strLine);
                        }
                      }
                    }
                    else
                    {
                      bExit = true;
                      if (nReturn < 0)
                      {
                        ssMessage.str("");
                        ssMessage << "Utility::sslRead(" << SSL_get_error(ssl, nReturn) << ") " << utility.sslstrerror(ssl, nReturn);
                        strError = ssMessage.str();
                      }
                    }
                  }
                  if (fds[0].revents & POLLOUT)
                  {
                    if (!strBuffers[1].empty())
                    {
                      if (utility.fdWrite(fdSocket, strBuffers[1], nReturn))
                      {
                        if (strBuffers[1].empty() && (ssl = utility.sslConnect(ctx, fdSocket, strError)) == NULL)
                        {
                          bExit = true;
                          ssMessage.str("");
                          ssMessage << "utility::sslConnect() " << strError;
                          strError = ssMessage.str();
                        }
                      }
                      else
                      {
                        bExit = true;
                        if (nReturn < 0)
                        {
                          ssMessage.str("");
                          ssMessage << "Utility::fdWrite(" << errno << ") " << strerror(errno);
                          strError = ssMessage.str();
                        }
                      }
                    }
                    else if (!utility.sslWrite(ssl, strBuffers[2], nReturn))
                    {
                      bExit = true;
                      if (nReturn < 0)
                      {
                        ssMessage.str("");
                        ssMessage << "Utility::sslWrite(" << SSL_get_error(ssl, nReturn) << ") " << utility.sslstrerror(ssl, nReturn);
                        strError = ssMessage.str();
                      }
                    }
                  }
                }
                else if (nReturn < 0)
                {
                  bExit = true;
                  ssMessage.str("");
                  ssMessage << "poll(" << errno << ") " << strerror(errno);
                  strError = ssMessage.str();
                }
              }
              SSL_shutdown(ssl);
              SSL_free(ssl);
              close(fdSocket);
            }
            else
            {
              ssMessage.str("");
              ssMessage << ((!bSocket)?"socket":"connect") << "(" << errno << ") " << strerror(errno);
              strError = ssMessage.str();
            }
          }
          else
          {
            ssMessage.str("");
            ssMessage << "getaddrinfo(" << nReturn << ") " << gai_strerror(nReturn);
            strError = ssMessage.str();
          }
        }
        SSL_CTX_free(ctx);
      }
      else
      {
        ssMessage.str("");
        ssMessage << "Utility::sslInitClient() " << strError;
        strError = ssMessage.str();
      }
    }
    else if (requestArray.find("Port Concentrator") == requestArray.end() || requestArray["Port Concentrator"].empty())
    {
      strError = "Please provide the Port Concentrator.";
    }
    else
    {
      strError = "Encountered an uncaught error.";
    }
  }
  else if (requestArray.find("SubService") == requestArray.end() || requestArray["SubService"].empty())
  {
    strError = "Please provide the SubService.";
  }
  else if (requestArray.find("Throttle") == requestArray.end() || requestArray["Throttle"].empty())
  {
    strError = "Please provide the Throttle.";
  }
  else
  {
    strError = "Encountered an unknown error.";
  }
  if (bProcessed)
  {
    for (auto &i : response)
    {
      cout << i << endl;
    }
  }
  else
  {
    ptJson = new Json(request.front());
    ptJson->insert("Status", "error");
    if (!strError.empty())
    {
      ptJson->insert("Error", strError);
    }
    cout << ptJson << endl;
    delete ptJson;
    request.pop_front();
    for (auto &i : request)
    {
      cout << i << endl;
    }
  }
  request.clear();
  response.clear();

  return 0;
}
