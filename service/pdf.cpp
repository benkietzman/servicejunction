// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Portable Document Format
// -------------------------------------
// file       : pdf.cpp
// author     : Ben Kietzman
// begin      : 2020-02-05
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

/*! \file pdf.cpp
* \brief Portable Document Format
*
* Processes PDF documents
* https://poppler.freedesktop.org/
*/
#include <iostream>
#include <string>
using namespace std;
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
using namespace poppler;
#include <Json>
#include <StringManip>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  list<Json *> request, response;
  string strError, strValue;
  StringManip manip;

  loadRequest(request);
  if (request.size() == 2)
  {
    Json *ptRequest = new Json(request.back());
    ptJson = new Json(request.front());
    if (ptRequest->m.find("Data") != ptRequest->m.end() && !ptRequest->m["Data"]->v.empty())
    {
      string strData, strOwnerPassword, strUserPassword;
      document *pDocument;
      if (ptRequest->m.find("Passwords") != ptRequest->m.end())
      {
        if (ptRequest->m["Passwords"]->m.find("Owner") != ptRequest->m["Password"]->m.end() && !ptRequest->m["Password"]->m["Owner"]->v.empty())
        {
          strOwnerPassword = ptRequest->m["Password"]->m["Owner"]->v;
        }
        if (ptRequest->m["Passwords"]->m.find("User") != ptRequest->m["Password"]->m.end() && !ptRequest->m["Password"]->m["User"]->v.empty())
        {
          strUserPassword = ptRequest->m["Password"]->m["User"]->v;
        }
      }
      manip.decodeBase64(ptRequest->m["Data"]->v, strData);
      if ((pDocument = document::load_from_raw_data(strData.c_str(), strData.size(), strOwnerPassword, strUserPassword)) != NULL)
      {
        Json *ptSubJson = new Json;
        bProcessed = true;
        ptSubJson->m["Pages"] = new Json;
        for (int i = 0; i < pDocument->pages(); i++)
        {
          ptSubJson->m["Pages"]->push_back(pDocument->create_page(i)->text().to_latin1());
        }
        response.push_back(ptSubJson);
      }
      else
      {
        strError = "Failed to parse the PDF document.";
      }
    }
    else
    {
      strError = "Please provide the base64 encoded Data.";
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
