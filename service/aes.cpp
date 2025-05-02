/* -*- c++ -*- */
///////////////////////////////////////////
// Advanced Encryption Standard
// -------------------------------------
// file       : aes.cpp
// author     : Ben Kietzman
// begin      : 2022-03-18
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
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
        if (ptRequest->m.find("Secret") != ptRequest->m.end() && !ptRequest->m["Secret"]->v.empty())
        {
          if (ptJson->m["Function"]->v == "decrypt")
          {
            Json *ptResponse = new Json;
            bProcessed = true;
            ptResponse->insert("Payload", manip.decryptAes(manip.decodeBase64(ptRequest->m["Payload"]->v, strValue), ptRequest->m["Secret"]->v, strValue, strError));
            response.push_back(ptResponse);
          }
          else if (ptJson->m["Function"]->v == "encrypt")
          {
            Json *ptResponse = new Json;
            bProcessed = true;
            ptResponse->insert("Payload", manip.encodeBase64(manip.encryptAes(ptRequest->m["Payload"]->v, ptRequest->m["Secret"]->v, strValue, strError), strValue));
            response.push_back(ptResponse);
          }
          else
          {
            strError = "Please provide a valid Function:  decrypt, encrypt.";
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
