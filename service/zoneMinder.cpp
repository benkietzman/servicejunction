// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// ZoneMinder
// -------------------------------------
// file       : zoneMidner.cpp
// author     : Ben Kietzman
// begin      : 2016-12-22
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

/*! \file zoneMinder.cpp
* \brief ZoneMinder Interface
*
* Interfaces with ZoneMinder servers via their Restful API.
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
  if (request.size() == 1)
  {
    ptJson = new Json(request.front());
    if (ptJson->m.find("reqApp") != ptJson->m.end() && !ptJson->m["reqApp"]->v.empty())
    {
      junction.setApplication(ptJson->m["reqApp"]->v);
    }
    if (ptJson->m.find("Secure") != ptJson->m.end() && ptJson->m["Secure"]->v == "no")
    {
      junction.useSecureJunction(false);
    }
    if (ptJson->m.find("Base") != ptJson->m.end() && !ptJson->m["Base"]->v.empty())
    {
      bool bReady = true;
      string strContent, strCookies, strHeader, strProxy, strURL = ptJson->m["Base"]->v + "/index.php";
      if (ptJson->m.find("Proxy") != ptJson->m.end() && !ptJson->m["Proxy"]->v.empty())
      {
        strProxy = ptJson->m["Proxy"]->v;
      }
      if (ptJson->m.find("User") != ptJson->m.end() && !ptJson->m["User"]->v.empty() && ptJson->m.find("Password") != ptJson->m.end() && !ptJson->m["Password"]->v.empty())
      {
        Json *ptPost = new Json;
        bReady = false;
        ptPost->insert("username", ptJson->m["User"]->v);
        ptPost->insert("password", ptJson->m["Password"]->v);
        ptPost->insert("action", "login");
        ptPost->insert("view", "console");
        if (junction.curl(strURL, "", NULL, NULL, ptPost, NULL, strProxy, strCookies, strHeader, strContent, strError))
        {
          bReady = true;
        }
        delete ptPost;
      }
      if (bReady)
      {
        if (ptJson->m.find("Action") != ptJson->m.end() && !ptJson->m["Action"]->v.empty())
        {
          strURL = ptJson->m["Base"]->v + (string)"/api" + ptJson->m["Action"]->v;
          if (junction.curl(strURL, "", NULL, NULL, NULL, NULL, strProxy, strCookies, strHeader, strContent, strError))
          {
            string strJson, strLine, strTrim;
            stringstream ssContent(strContent);
            bProcessed = true;
            while (getline(ssContent, strLine))
            {
              manip.trim(strTrim, strLine);
              if (!strTrim.empty())
              {
                strJson += strTrim;
              }
            }
            if (!strJson.empty())
            {
              response.push_back(new Json(strJson));
            }
          }
        }
        else
        {
          strError = "Please provide the Action.";
        }
      }
    }
    else
    {
      strError = "Please provide the Base.";
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
