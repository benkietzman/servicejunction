// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Bureau of Labor Statistics
// -------------------------------------
// file       : bls.cpp
// author     : Ben Kietzman
// begin      : 2022-10-25
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

/*! \file bls.cpp
* \brief Bureau of Labor Statistics
*
* Provides an interface to BLS' REST service.
*/
// {{{ includes
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <zlib.h>
using namespace std;
#include <DateTime>
#include <Json>
#include <ServiceJunction>
#include <StringManip>
#include <Utility>
using namespace common;
#include "include/functions"
// }}}
// {{{ global variables
bool gbDebug = false;
// }}}
// {{{ prototypes
void addCurlResult(Json *ptCurl, const string strURL, Json *ptGet, Json *ptPost, const string strCookies, const string strHeader, const string strContent);
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  bool bProcessed = false;
  list<Json *> request;
  string strError, strErrorSubType, strErrorType, strValue;
  stringstream ssMessage;
  Json *ptCurl = NULL, *ptData = NULL;
  ServiceJunction *pJunction;
  StringManip manip;

  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "--debug")
    {
      gbDebug = true;
    }
  }
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  loadRequest(request);
  pJunction = new ServiceJunction(strError);
  if (strError.empty())
  {
    map<string, string> label;
    string strApi = "publicAPI", strSubError, strVersion = "v1";
    stringstream ssDuration;
    time_t CEndTime, CStartTime;
    pJunction->setApplication("Service Junction (bls)");
    pJunction->useSecureJunction(false);
    time(&CStartTime);
    label["Service"] = "bls";
    if (request.front()->m.find("Api") != request.front()->m.end() && !request.front()->m["Api"]->v.empty())
    {
      strApi = request.front()->m["Api"]->v;
    }
    label["Api"] = strApi;
    if (request.front()->m.find("Version") != request.front()->m.end() && !request.front()->m["Version"]->v.empty())
    {
      strVersion = request.front()->m["Version"]->v;
    }
    label["Version"] = strVersion;
    if (request.front()->m.find("Resource") != request.front()->m.end() && !request.front()->m["Resource"]->v.empty())
    {
      bool bCurl = false;
      size_t unPosition;
      string strContent, strCookies, strHeader, strPage, strProxy, strSearch, strServer = "api.bls.gov", strURL = (string)"https://" + strServer + (string)"/" + strApi + (string)"/" + strVersion + (string)"/" + request.front()->m["Resource"]->v;
      Json *ptAuth = NULL, *ptGet = NULL, *ptPost = NULL, *ptPut = NULL;
      if (request.front()->m.find("User") != request.front()->m.end() && !request.front()->m["User"]->v.empty() && request.front()->m.find("Password") != request.front()->m.end() && !request.front()->m["Password"]->v.empty())
      {
        label["User"] = request.front()->m["User"]->v;
        ptAuth = new Json;
        ptAuth->insert("Type", "basic");
        ptAuth->insert("User", request.front()->m["User"]->v);
        ptAuth->insert("Password", request.front()->m["Password"]->v);
      }
      if (request.front()->m.find("Proxy") != request.front()->m.end() && !request.front()->m["Proxy"]->v.empty())
      {
        strProxy = request.front()->m["Proxy"]->v;
      }
      if (request.size() == 2)
      {
        if (request.back()->m.find("Get") != request.back()->m.end())
        {
          ptGet = request.back()->m["Get"];
        }
        if (request.back()->m.find("Post") != request.back()->m.end())
        {
          ptPost = request.back()->m["Post"];
        }
        if (request.back()->m.find("Put") != request.back()->m.end())
        {
          ptPut = request.back()->m["Put"];
        }
      }
      bCurl = pJunction->curl(strURL + strPage, "json", ptAuth, ptGet, ptPost, ptPut, strProxy, strCookies, strHeader, strContent, strError, "", false, false);
      if (ptAuth != NULL)
      {
        delete ptAuth;
      }
      if (request.front()->m.find("curlResults") != request.front()->m.end() && request.front()->m["curlResults"]->v == "yes")
      {
        addCurlResult((ptCurl = new Json), strURL + strPage, ptPost, ptPut, strCookies, strHeader, strContent);
      }
      while ((unPosition = strContent.find("\n")) != string::npos || (unPosition = strContent.find("\r")) != string::npos || (unPosition = strContent.find("\\")) != string::npos)
      {
        strContent.erase(unPosition, 1);
      }
      ptData = new Json(strContent);
      if (bCurl)
      {
        bProcessed = true;
      }
      else if (!strContent.empty())
      {
        strErrorType = "bls";
        strErrorSubType = "unknown";
        strError = "unkown";
      }
      else
      {
        stringstream ssError;
        strErrorType = "bls";
        strErrorSubType = request.front()->m["Function"]->v;
        ssError << "[curl," << strURL << strPage << "] " << strError;
        strError = ssError.str();
      }
    }
    else
    {
      strErrorType = "syntax";
      strErrorSubType = "User";
      strError = "Please provide the Resource.";
    }
    label["Status"] = (string)((bProcessed)?"okay":"error");
    label["ErrorType"] = strErrorType;
    label["ErrorSubType"] = strErrorSubType;
    if (request.front()->m.find("reqApp") != request.front()->m.end() && !request.front()->m["reqApp"]->v.empty())
    {
      label["reqApp"] = request.front()->m["reqApp"]->v;
    }
    time(&CEndTime);
    ssDuration << (CEndTime - CStartTime);
    label["Duration"] = ssDuration.str();
    pJunction->setApplication("Service Junction");
    label.clear();
  }
  else
  {
    strErrorType = "junction";
    strErrorSubType = "construct";
  }
  delete pJunction;
  request.front()->insert("Status", ((bProcessed)?"okay":"error"));
  if (!strError.empty())
  {
    Json *ptError = new Json;
    ptError->insert("Message", strError);
    ptError->insert("Type", strErrorType);
    ptError->insert("SubType", strErrorSubType);
    request.front()->insert("Error", ptError);
  }
  cout << request.front() << endl;
  if (ptData != NULL)
  {
    if (ptCurl != NULL)
    {
      ptData->m["curlResults"] = ptCurl;
    }
    cout << ptData << endl;
    delete ptData;
  }
  for (auto &i : request)
  {
    delete i;
  }
  request.clear();

  return 0;
}
// }}}
// {{{ addCurlResult()
void addCurlResult(Json *ptCurl, const string strURL, Json *ptGet, Json *ptPost, const string strCookies, const string strHeader, const string strContent)
{
  Json *ptItem = new Json;
  ptItem->insert("URL", strURL);
  if (ptGet != NULL)
  {
    ptItem->m["Get"] = new Json(ptGet);
  }
  if (ptPost != NULL)
  {
    ptItem->m["Post"] = new Json(ptPost);
  }
  ptItem->insert("Cookies", strCookies);
  ptItem->insert("Header", strHeader);
  ptItem->insert("Content", strContent);
  ptCurl->l.push_back(ptItem);
}
// }}}
