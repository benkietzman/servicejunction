// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// CURL
// -------------------------------------
// file       : curl.cpp
// author     : Ben Kietzman
// begin      : 2013-03-08
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

/*! \file curl.cpp
* \brief CURL
*
* A CURL program capable of interfacing with websites.
*/
#include <cstdio>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
using namespace std;
#include <Json>
#include <StringManip>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bDebug = false, bProcessed = false;
  Json *ptJson;
  map<string, string> requestArray;
  list<string> request;
  string strError, strValue;
  stringstream ssExtra;
  StringManip manip;

  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "--debug")
    {
      bDebug = true;
    }
  }
  loadRequest(requestArray, request);
  request.pop_front();
  if (!request.empty())
  {
    bool bDisplayCookies = false;
    stringstream ssCookies;
    bProcessed = true;
    ssCookies << "/tmp/curl_" << getpid() << ".cookies";
    if (requestArray.find("Cookies") != requestArray.end() && !requestArray["Cookies"].empty())
    {
      ofstream outCookies(ssCookies.str().c_str());
      if (outCookies)
      {
        string strLine;
        stringstream ssData(requestArray["Cookies"]);
        while (getline(ssData, strLine))
        {
          outCookies << strLine << endl;
        }
      }
      outCookies.close();
    }
    for (auto i = request.begin(); bProcessed && i != request.end(); i++)
    {
      map<string, string> data;
      ptJson = new Json(*i);
      bProcessed = false;
      ptJson->flatten(data, false, false);
      if (data.find("URL") != data.end())
      {
        bool bFailOnError = true, bMobile = false;
        map<string, bool> display;
        map<string, string> auth;
        string strContent, strCustomRequest, strHeader, strPost, strProxy, strPut, strType, strURL = data["URL"], strUserAgent;
        if (ptJson->m.find("Auth") != ptJson->m.end())
        {
          for (auto &j : ptJson->m["Auth"]->m)
          {
            auth[j.first] = j.second->v;
          }
        }
        if (data.find("Cookies") != data.end() && !data["Cookies"].empty())
        {
          ofstream outCookies(ssCookies.str().c_str());
          if (outCookies)
          {
            string strLine;
            stringstream ssData(data["Cookies"]);
            while (getline(ssData, strLine))
            {
              outCookies << strLine << endl;
            }
          }
          outCookies.close();
        }
        if (data.find("Content") != data.end() && !data["Content"].empty())
        {
          strContent = data["Content"];
        }
        if (data.find("FailOnError") != data.end() && data["FailOnError"] == "no")
        {
          bFailOnError = false;
        }
        if (data.find("Header") != data.end() && !data["Header"].empty())
        {
          strHeader = data["Header"];
        }
        if (data.find("UserAgent") != data.end() && !data["UserAgent"].empty())
        {
          strUserAgent = data["UserAgent"];
        }
        if (data.find("Mobile") != data.end() && data["Mobile"] == "yes")
        {
          bMobile = true;
        }
        if (data.find("CustomRequest") != data.end() && !data["CustomRequest"].empty())
        {
          strCustomRequest = data["CustomRequest"];
        }
        if (data.find("Type") != data.end() && !data["Type"].empty())
        {
          strType = data["Type"];
        }
        display["Content"] = false;
        display["Cookies"] = false;
        display["Header"] = false;
        if (data.find("Display") != data.end() && !data["Display"].empty())
        {
          string strToken;
          stringstream ssDisplay(data["Display"]);
          while (getline(ssDisplay, strToken, ','))
          {
            display[strToken] = true;
          }
        }
        if (ptJson->m.find("Get") != ptJson->m.end())
        {
          string strSymbol = "?";
          if (strURL.find("?", 0) != string::npos)
          {
            strSymbol = "&";
          }
          for (auto &j : ptJson->m["Get"]->m)
          {
            strURL += strSymbol + manip.urlEncode(strValue, j.first) + (string)"=" + manip.urlEncode(strValue, j.second->v);
            strSymbol = "&";
          }
        }
        if (data.find("Proxy") != data.end() && !data["Proxy"].empty())
        {
          strProxy = data["Proxy"];
        }
        if (ptJson->m.find("Post") != ptJson->m.end())
        {
          if (strType == "json")
          {
            ptJson->m["Post"]->json(strPost);
          }
          else if (strType == "plain")
          {
            strPost = ptJson->m["Post"]->v;
          }
          else
          {
            for (auto j = ptJson->m["Post"]->m.begin(); j != ptJson->m["Post"]->m.end(); j++)
            {
              if (j != ptJson->m["Post"]->m.begin())
              {
                strPost += "&";
              }
              strPost += manip.urlEncode(strValue, j->first) + (string)"=" + manip.urlEncode(strValue, j->second->v);
            }
          }
        }
        if (ptJson->m.find("Put") != ptJson->m.end())
        {
          if (strType == "json")
          {
            ptJson->m["Put"]->json(strPut);
          }
          else
          {
            for (auto j = ptJson->m["Put"]->m.begin(); j != ptJson->m["Put"]->m.end(); j++)
            {
              if (j != ptJson->m["Put"]->m.begin())
              {
                strPut += "&";
              }
              strPut += manip.urlEncode(strValue, j->first) + (string)"=" + manip.urlEncode(strValue, j->second->v);
            }
          }
        }
        if (fetchPage(strURL, strType, auth, ssCookies.str(), strPost, strPut, strProxy, strHeader, strContent, strError, strUserAgent, bMobile, bFailOnError, bDebug, strCustomRequest))
        {
          bProcessed = true;
        }
        else if (strError.empty())
        {
          strError = "Caught an unknown CURL error.";
        }
        data["Status"] = (string)((bProcessed)?"okay":"error");
        if (!strError.empty())
        {
          data["Error"] = strError;
        }
        if (display["Cookies"])
        {
          ifstream inCookies(ssCookies.str().c_str());
          if (inCookies)
          {
            string strLine;
            stringstream ssData;
            while (getline(inCookies, strLine))
            {
              ssData << strLine << endl;
            }
            if (!ssData.str().empty())
            {
              data["Cookies"] = ssData.str();
            }
          }
          inCookies.close();
        }
        if (display["Header"] && !strHeader.empty())
        {
          data["Header"] = strHeader;
        }
        if (display["Content"] && !strContent.empty())
        {
          data["Content"] = strContent;
        }
        display.clear();
        ptJson->insert(data);
        ssExtra << ptJson << endl;
        auth.clear();
      }
      data.clear();
      delete ptJson;
    }
    if (requestArray.find("Display") != requestArray.end() && !requestArray["Display"].empty())
    {
      string strToken;
      stringstream ssDisplay(requestArray["Display"]);
      while (getline(ssDisplay, strToken, ','))
      {
        if (strToken == "cookies")
        {
          bDisplayCookies = true;
        }
      }
    }
    if (bDisplayCookies)
    {
      ifstream inCookies(ssCookies.str().c_str());
      if (inCookies)
      {
        string strLine;
        stringstream ssData;
        while (getline(inCookies, strLine))
        {
          ssData << strLine << endl;
        }
        if (!ssData.str().empty())
        {
          requestArray["Cookies"] = ssData.str();
        }
      }
      inCookies.close();
    }
    remove(ssCookies.str().c_str());
  }
  else
  {
    strError = "Please provide a URL request.";
  }
  requestArray["Status"] = (string)((bProcessed)?"okay":"error");
  if (!strError.empty())
  {
    requestArray["Error"] = strError;
  }
  ptJson = new Json(requestArray);
  cout << ptJson << endl;
  delete ptJson;
  if (!ssExtra.str().empty())
  {
    cout << ssExtra.str();
  }
  ssExtra.clear();
  requestArray.clear();
  request.clear();

  return 0;
}
