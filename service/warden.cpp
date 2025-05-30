/* -*- c++ -*- */
///////////////////////////////////////////
// Warden
// -------------------------------------
// file       : warden.cpp
// author     : Ben Kietzman
// begin      : 2021-05-05
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file warden.cpp
* \brief Warden
*
* Provides access to the local warden.
*/

#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
using namespace std;
#include <Json>
#include <StringManip>
#include <Warden>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  list<Json *> request;
  list<string> response;
  string strUnix = "/data/warden/socket", strError, strJson;
  Json *ptRequest;
  StringManip manip;

  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg.size() > 7 && strArg.substr(0, 7) == "--unix=")
    {
      strUnix = strArg.substr(7, strArg.size() - 7);
      manip.purgeChar(strUnix, strUnix, "'");
      manip.purgeChar(strUnix, strUnix, "\"");
    }
  }
  loadRequest(request);
  ptRequest = request.front();
  if (ptRequest->m.find("Module") != ptRequest->m.end() && !ptRequest->m["Module"]->v.empty())
  {
    if (ptRequest->m["Module"]->v != "vault")
    {
      if (request.size() == 2)
      {
        Warden warden("Service Junction", strUnix, strError);
        if (strError.empty())
        {
          Json *ptReq = new Json(request.back()), *ptRes = new Json;
          ptReq->insert("Module", ptRequest->m["Module"]->v);
          if (warden.request(ptReq, ptRes, 120, strError))
          {
            bProcessed = true;
          }
          if (ptRes->m.find("Data") != ptRes->m.end())
          {
            response.push_back(ptRes->m["Data"]->json(strJson));
          }
          delete ptReq;
          delete ptRes;
        }
      }
      else
      {
        stringstream ssError;
        ssError << "Received " << request.size() << " lines in the request when expecting 2 lines.";
        strError = ssError.str();
      }
    }
    else
    {
      strError = "Please provide a valid Module.";
    }
  }
  else
  {
    strError = "Please provide the Module.";
  }
  ptRequest->insert("Status", ((bProcessed)?"okay":"error"));
  if (!strError.empty())
  {
    ptRequest->insert("Error", strError);
  }
  cout << ptRequest << endl;
  for (auto &i : request)
  {
    delete i;
  }
  request.clear();
  for (auto &i : response)
  {
    cout << i << endl;
  }
  response.clear();

  return 0;
}
