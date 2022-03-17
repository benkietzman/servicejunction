// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// JSON Web Tokens
// -------------------------------------
// file       : jwt.cpp
// author     : Ben Kietzman
// begin      : 2018-04-06
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

/*! \file jwt.cpp
* \brief JSON Web Tokens
*
* Processes JSON Web Tokens
* https://github.com/pokowaka/jwt-cpp
*/
#include <iostream>
#include <string>
using namespace std;
#include <jwt/jwt_all.h>
using json = nlohmann::json;
#include <Json>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  list<Json *> request, response;
  string strError, strValue;

  loadRequest(request);
  if (request.size() == 2)
  {
    Json *ptRequest = new Json(request.back());
    ptJson = new Json(request.front());
    if (ptRequest->m.find("Signer") != ptRequest->m.end() && !ptRequest->m["Signer"]->v.empty())
    {
      MessageSigner *pSigner = nullptr;
      if (ptRequest->m["Signer"]->v == "HS256" || ptRequest->m["Signer"]->v == "HS384" || ptRequest->m["Signer"]->v == "HS512")
      {
        if (ptRequest->m.find("Secret") != ptRequest->m.end() && !ptRequest->m["Secret"]->v.empty())
        {
          if (ptRequest->m["Signer"]->v == "HS256")
          {
            pSigner = new HS256Validator(ptRequest->m["Secret"]->v);
          }
          else if (ptRequest->m["Signer"]->v == "HS384")
          {
            pSigner = new HS384Validator(ptRequest->m["Secret"]->v);
          }
          else if (ptRequest->m["Signer"]->v == "HS512")
          {
            pSigner = new HS512Validator(ptRequest->m["Secret"]->v);
          }
        }
      }
      else if (ptRequest->m["Signer"]->v == "RS256" || ptRequest->m["Signer"]->v == "RS384" || ptRequest->m["Signer"]->v == "RS512")
      {
        if (ptRequest->m.find("Public Key") != ptRequest->m.end() && !ptRequest->m["Public Key"]->v.empty())
        {
          if (ptRequest->m.find("Private Key") != ptRequest->m.end() && !ptRequest->m["Private Key"]->v.empty())
          {
            if (ptRequest->m["Signer"]->v == "RS256")
            {
              pSigner = new RS256Validator(ptRequest->m["Public Key"]->v, ptRequest->m["Private Key"]->v);
            }
            else if (ptRequest->m["Signer"]->v == "RS384")
            {
              pSigner = new RS384Validator(ptRequest->m["Public Key"]->v, ptRequest->m["Private Key"]->v);
            }
            else if (ptRequest->m["Signer"]->v == "RS512")
            {
              pSigner = new RS512Validator(ptRequest->m["Public Key"]->v, ptRequest->m["Private Key"]->v);
            }
          }
          else
          {
            if (ptRequest->m["Signer"]->v == "RS256")
            {
              pSigner = new RS256Validator(ptRequest->m["Public Key"]->v);
            }
            else if (ptRequest->m["Signer"]->v == "RS384")
            {
              pSigner = new RS384Validator(ptRequest->m["Public Key"]->v);
            }
            else if (ptRequest->m["Signer"]->v == "RS512")
            {
              pSigner = new RS512Validator(ptRequest->m["Public Key"]->v);
            }
          }
        }
        else
        {
          strError = "Please provide the Public Key.";
        }
      }
      if (ptRequest->m.find("Payload") != ptRequest->m.end())
      {
        if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
        {
          if (ptJson->m["Function"]->v == "decode")
          {
            ExpValidator exp;
            json header, payload;
            try
            {
              stringstream ssHeader, ssPayload;
              Json *ptResponse = new Json;
              bProcessed = true;
              tie(header, payload) = JWT::Decode(ptRequest->m["Payload"]->v, pSigner, &exp);
              ssHeader << header;
              ptResponse->m["Header"] = new Json(ssHeader.str());
              ssPayload << payload;
              ptResponse->m["Payload"] = new Json(ssPayload.str());
              response.push_back(ptResponse);
            }
            catch (InvalidTokenError &tfe)
            {
              bProcessed = false;
              strError = tfe.what();
            }
            catch (exception &e)
            {
              bProcessed = false;
              strError = e.what();
            }
          }
          else if (ptJson->m["Function"]->v == "encode")
          {
            json data;
            try
            {
              stringstream ssJson(ptRequest->m["Payload"]->json(strValue));
              Json *ptResponse = new Json;
              bProcessed = true;
              ssJson >> data;
              ptResponse->insert("Payload", JWT::Encode((*pSigner), data));
              response.push_back(ptResponse);
            }
            catch (exception &e)
            {
              bProcessed = false;
              strError = e.what();
            }
          }
          else
          {
            strError = "Please provide a valid Function:  decode, encode.";
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
      if (pSigner != nullptr)
      {
        delete pSigner;
      }
    }
    else
    {
      strError = "Please provide the Signer.";
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
