// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Acronym
// -------------------------------------
// file       : acronym.cpp
// author     : Ben Kietzman
// begin      : 2015-02-13
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

/*! \file acronym.cpp
* \brief Acronym Lookup Service
*
* Retrieves information based on a provided acronym.
*/
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
#include <File>
#include <Json>
#include <StringManip>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  File file;
  Json *ptJson;
  StringManip manip;
  list<Json *> request, response;
  string strData = "/data/servicejunction/acronym", strError;

  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg.size() > 7 && strArg.substr(0, 7) == "--data=")
    {
      strData = strArg.substr(7, strArg.size() - 7);
      manip.purgeChar(strData, strData, "'");
      manip.purgeChar(strData, strData, "\"");
    }
  }
  loadRequest(request);
  if (request.size() == 1)
  {
    list<string> dir;
    string strAcronym, strCategory;
    ptJson = new Json(request.front());
    bProcessed = true;
    if (ptJson->m.find("Acronym") != ptJson->m.end() && !ptJson->m["Acronym"]->v.empty())
    {
      strAcronym = ptJson->m["Acronym"]->v;
    }
    if (ptJson->m.find("Category") != ptJson->m.end() && !ptJson->m["Category"]->v.empty())
    {
      strCategory = ptJson->m["Category"]->v;
    }
    file.directoryList(strData, dir);
    for (auto &i : dir)
    {
      if (i.size() > 5 && i.substr(i.size() - 5, 5) == ".json" && (strCategory.empty() || i.substr(0, i.size() - 5) == strCategory))
      {
        ifstream  inFile((strData + (string)"/" + i).c_str());
        if (inFile)
        {
          string strJson;
          while (getline(inFile, strJson))
          {
            Json *ptSubJson = new Json(strJson);
            if (strAcronym.empty() || (ptSubJson->m.find("Acronym") != ptSubJson->m.end() && ptSubJson->m["Acronym"]->v == strAcronym))
            {
              ptSubJson->insert("Category", i.substr(0, i.size() - 5));
              response.push_back(ptSubJson);
            }
            else
            {
              delete ptSubJson;
            }
          }
          inFile.close();
        }
      }
    }
    dir.clear();
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
