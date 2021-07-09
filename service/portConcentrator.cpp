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
#include <openssl/err.h>
#include <openssl/ssl.h>
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
      SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();
      if ((ctx = SSL_CTX_new(method)) != NULL)
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
            struct addrinfo *rp;
            for (rp = result; !bConnected && rp != NULL; rp = rp->ai_next)
            {
              if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
              {
                if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                {
                  if (i == 0)
                  {
                    string strJson;
                    ptJson = new Json(request.front());
                    if (ptJson->m.find("SubService") != ptJson->m.end() && !ptJson->m["SubService"]->v.empty())
                    {
                      ptJson->insert("Service", ptJson->m["SubService"]->v);
                      delete ptJson->m["SubService"];
                      ptJson->m.erase("SubService");
                    }
                    ptJson->json(strJson);
                    delete ptJson;
                    write(fdSocket, (strJson + (string)"\n").c_str(), (strJson.size() + 1));
                  }
                  if ((ssl = SSL_new(ctx)) != NULL)
                  {
                    if (SSL_set_fd(ssl, fdSocket) == 1 && SSL_connect(ssl) == 1)
                    {
                      bConnected = true;
                      request.pop_front();
                    }
                    else
                    {
                      SSL_free(ssl);
                      close(fdSocket);
                    }
                  }
                  else
                  {
                    close(fdSocket);
                  }
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
              string strTrim;
              StringManip manip;
              Utility utility(strError);
              for (auto &i : request)
              {
                SSL_write(ssl, (i + (string)"\n").c_str(), (i.size() + 1));
              }
              SSL_write(ssl, ((string)"end\n").c_str(), 4);
              while (!bProcessed && utility.getLine(ssl, strLine))
              {
                manip.trim(strTrim, strLine);
                if (strTrim == "end")
                {
                  bProcessed = true;
                }
                else
                {
                  response.push_back(strLine);
                }
              }
              SSL_free(ssl);
              close(fdSocket);
            }
            else
            {
              strError = "Failed to connect to the Port Concentrator.";
            }
          }
          else
          {
            strError = gai_strerror(nReturn);
          }
        }
        SSL_CTX_free(ctx);
      }
      else
      {
        strError = "Failed to create SSL context.";
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
    strError = "Encountered an uncaught error.";
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
