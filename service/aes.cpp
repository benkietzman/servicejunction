// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Advanced Encryption Standard
// -------------------------------------
// file       : aes.cpp
// author     : Ben Kietzman
// begin      : 2022-03-18
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

/*! \file aes.cpp
* \brief Advanced Encryption Standard
*
* Processes Advanced Encryption Standard
*/
#include <iostream>
#include <string>
using namespace std;
#include <Json>
#include <StringManip>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  list<Json *> request, response;
  string strError, strValue;
  Json *ptJson;
  StringManip manip;

  loadRequest(request);
  if (request.size() == 2)
  {
    Json *ptRequest = new Json(request.back());
    ptJson = new Json(request.front());
    if (ptRequest->m.find("Payload") != ptRequest->m.end())
    {
      if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
      {
        if (ptJson->m.find("Secret") != ptJson->m.end() && !ptJson->m["Secret"]->v.empty())
        {
          if (ptJson->m["Function"]->v == "decode")
          {
            Json *ptResponse = new Json;
            bProcessed = true;
            ptResponse->insert("Payload", manip.decryptAes(manip.decodeBase64(ptRequest->m["Payload"]->v, strValue), ptRequest->m["Secret"]->v, strValue, strError));
            response.push_back(ptResponse);
          }
          else if (ptJson->m["Function"]->v == "encode")
          {
            Json *ptResponse = new Json;
            bProcessed = true;
            ptResponse->insert("Payload", manip.encodeBase64(manip.encryptAes(ptRequest->m["Payload"]->v, ptRequest->m["Secret"]->v, strValue, strError), strValue));
            response.push_back(ptResponse);
          }
          else
          {
            strError = "Please provide a valid Function:  decode, encode.";
          }
        }
        else
        {
          strError = "Please provide the Secret.";
        }
      }
      else
      {
        strError = "Please provide the Function.";
      }
    }
    else
    {
      strError = "Please provide the Payload.";
    }
    delete ptRequest;
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
