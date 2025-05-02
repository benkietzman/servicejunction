/* -*- c++ -*- */
///////////////////////////////////////////
// MySQL
// -------------------------------------
// file       : google.cpp
// author     : Ben Kietzman
// begin      : 2011-12-22
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
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
  list<map<string, string> > responseArray;
  list<string> request;
  string strError, strLine, strValue;
  StringManip manip;

  loadRequest(requestArray, request);
  if (requestArray.find("Application") != requestArray.end() && !requestArray["Application"].empty())
  {
    if (requestArray["Application"] == "fusion")
    {
      if (((requestArray.find("Query") != requestArray.end() && !requestArray["Query"].empty()) || (requestArray.find("Update") != requestArray.end() && !requestArray["Update"].empty() && ((requestArray.find("Auth") != requestArray.end() && !requestArray["Auth"].empty()) || (requestArray.find("User") != requestArray.end() && !requestArray["User"].empty() && requestArray.find("Password") != requestArray.end() && !requestArray["Password"].empty())))))
      {
        list<string> header;
        map<string, string> auth;
        string strHeader, strContent, strUrl;
        stringstream ssPost;
        if ((requestArray.find("Auth") == requestArray.end() || requestArray["Auth"].empty()) && requestArray.find("User") != requestArray.end() && !requestArray["User"].empty() && requestArray.find("Password") != requestArray.end() && !requestArray["Password"].empty())
        {
          strUrl = "https://www.google.com/accounts/ClientLogin";
          ssPost.str("");
          ssPost << "accountType=HOSTED_OR_GOOGLE&Email=" << manip.urlEncode(strValue, requestArray["User"]) << "&Passwd=" << manip.urlEncode(strValue, requestArray["Password"]) << "&service=fusiontables&source=";
          if (requestArray.find("Source") != requestArray.end() && !requestArray["Source"].empty())
          {
            ssPost << manip.urlEncode(strValue, requestArray["Source"]);
          }
          else
          {
            ssPost << manip.urlEncode(strValue, "kietzman.org-ServiceJunction-1.0");
          }
          if (fetchPage(strUrl, "", auth, "", ssPost.str(), "", "", strHeader, strContent, strError))
          {
            size_t nPosition[2] = {0, 0};
            if ((nPosition[0] = strContent.find("Auth=", 0)) != string::npos && (nPosition[1] = strContent.find("\n", nPosition[0])) != string::npos)
            {
              requestArray["Auth"] = strContent.substr(nPosition[0] + 5, nPosition[1] - (nPosition[0] + 5));
            }
          }
        }
        if (strError.empty())
        {
          strUrl = "https://www.google.com/fusiontables/api/query";
          if (requestArray.find("Auth") != requestArray.end() && !requestArray["Auth"].empty())
          {
            strHeader += (string)"Authorization: GoogleLogin auth=" + requestArray["Auth"] + (string)"\n";
          }
          if (requestArray.find("Query") != requestArray.end() && !requestArray["Query"].empty())
          {
            ssPost.str("");
            ssPost << "?sql=" << manip.urlEncode(strValue, requestArray["Query"]);
            strUrl += ssPost.str();
            if (fetchPage(strUrl, "", auth, "", "", "", "", strHeader, strContent, strError))
            {
              bProcessed = true;
            }
          }
          else if (requestArray.find("Update") != requestArray.end() && !requestArray["Update"].empty() && requestArray.find("Auth") != requestArray.end() && !requestArray["Auth"].empty())
          {
            ssPost.str("");
            ssPost << "sql=" << manip.urlEncode(strValue, requestArray["Update"]);
            if (fetchPage(strUrl, "", auth, "", ssPost.str(), "", "", strHeader, strContent, strError))
            {
              bProcessed = true;
            }
          }
          header.clear();
          if (bProcessed)
          {
            string strLine, strValue;;
            stringstream ssContent(strContent);
            vector<string> field;
            Utility utility(strError);
            utility.getLine(ssContent, strLine);
            manip.trim(strLine, strLine);
            for (int i = 1; !manip.getToken(strValue, strLine, i, ",").empty(); i++)
            {
              field.push_back(strValue);
            }
            while (utility.getLine(ssContent, strLine))
            {
              if (!manip.trim(strLine, strLine).empty())
              {
                map<string, string> row;
                for (int i = 1; !manip.getToken(strValue, strLine, i, ",").empty(); i++)
                {
                  if (field.size() >= (unsigned int)i)
                  {
                    row[field[i-1]] = strValue;
                  }
                }
                responseArray.push_back(row);
                row.clear();
              }
            }
            field.clear();
          }
        }
      }
      else if ((requestArray.find("Query") == requestArray.end() || requestArray["Query"].empty()) && (requestArray.find("Update") == requestArray.end() || requestArray["Update"].empty()))
      {
        strError = "Please provide the Google Fusion Query or Update.";
      }
      else if ((requestArray.find("Auth") == requestArray.end() || requestArray["Auth"].empty()) && (requestArray.find("User") == requestArray.end() || requestArray["User"].empty() || requestArray.find("Password") == requestArray.end() || requestArray["Password"].empty()))
      {
        strError = "Please provide an Auth token or a User and Password for authentication.";
      }
      else
      {
        strError = "Encountered and unknown error.";
      }
    }
    else
    {
      strError = "Please provide a valid Application:  fusion.";
    }
  }
  else
  {
    strError = "Please provide the Application:  fusion.";
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
