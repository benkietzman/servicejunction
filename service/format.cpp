// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Format
// -------------------------------------
// file       : format.cpp
// author     : Ben Kietzman
// begin      : 2015-05-15
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

/*! \file format.cpp
* \brief Format
*
* Provides a format service.
*/

#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
using namespace std;
#include <Json>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  list<Json *> request;
  list<string> response;
  string strError;
  Json *ptRequest;

  loadRequest(request);
  ptRequest = request.front();
  if (ptRequest->m.find("Function") != ptRequest->m.end() && !ptRequest->m["Function"]->v.empty())
  {
    if (ptRequest->m["Function"]->v == "convert")
    {
      if (ptRequest->m.find("Format") != ptRequest->m.end())
      {
        if (ptRequest->m["Format"]->m.find("In") != ptRequest->m["Format"]->m.end() && !ptRequest->m["Format"]->m["In"]->v.empty())
        {
          if (ptRequest->m["Format"]->m["In"]->v == "fcif" || ptRequest->m["Format"]->m["In"]->v == "json" || ptRequest->m["Format"]->m["In"]->v == "xml")
          {
            if (ptRequest->m["Format"]->m.find("Out") != ptRequest->m["Format"]->m.end() && !ptRequest->m["Format"]->m["Out"]->v.empty())
            {
              if (ptRequest->m["Format"]->m["Out"]->v == "fcif" || ptRequest->m["Format"]->m["Out"]->v == "json" || ptRequest->m["Format"]->m["Out"]->v == "xml")
              {
                if (request.size() == 2)
                {
                  string strData;
                  Json *ptData = new Json(request.back());
                  bProcessed = true;
                  if (ptRequest->m["Format"]->m["Out"]->v == "fcif")
                  {
                    response.push_back(ptData->fcif(strData));
                  }
                  else if (ptRequest->m["Format"]->m["Out"]->v == "json")
                  {
                    response.push_back(ptData->json(strData));
                  }
                  else if (ptRequest->m["Format"]->m["Out"]->v == "xml")
                  {
                    response.push_back(ptData->xml(strData));
                  }
                  delete ptData;
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
                strError = "Please provide a valid Out for the Format:  fcif, json, xml.";
              }
            }
            else
            {
              strError = "Please provide the Out for the Format.";
            }
          }
          else
          {
            strError = "Please provide a valid In for the Format:  fcif, json, xml.";
          }
        }
        else
        {
          strError = "Please provide the In for the Format.";
        }
      }
      else
      {
        strError = "Please provide the Format.";
      }
    }
    else
    {
      strError = "Please provide a valid Function:  convert.";
    }
  }
  else
  {
    strError = "Please provide the Function.";
  }
  ptRequest->insert("Status", ((bProcessed)?"okay":"error"));
  if (!strError.empty())
  {
    ptRequest->insert("Error", strError);
  }
  cout << ptRequest << endl;
  for (list<Json *>::iterator i = request.begin(); i != request.end(); i++)
  {
    delete (*i);
  }
  request.clear();
  for (list<string>::iterator i = response.begin(); i != response.end(); i++)
  {
    cout << (*i) << endl;
  }
  response.clear();

  return 0;
}
