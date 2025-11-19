/* -*- c++ -*- */
///////////////////////////////////////////
// Ping
// -------------------------------------
// file       : listServices.cpp
// author     : Ben Kietzman
// begin      : 2011-08-15
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file listServices.cpp
* \brief List Services
*
* Responds with the list of configured services.
*/
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;
#include <Json>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  ifstream inFile;
  string strData = "/data/servicejunction", strError, strResponse;
  list<Json *> request, response;
  Json *ptJson;

  loadRequest(request);
  if (request.size() == 1)
  {
    ptJson = new Json(request.front());
    if (argc == 2)
    {
      strData = argv[1];
    }
    inFile.open(strData + "/services.conf");
    if (inFile)
    {
      string strLine;
      stringstream ssData;
      bProcessed = true;
      while (getline(inFile, strLine))
      {
        ssData << strLine << endl;
      }
      strResponse = ssData.str();
    }
    else
    {
      strError = strerror(errno);
    }
    inFile.close();
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
  if (!strResponse.empty())
  {
    cout << strResponse;
  }

  return 0;
}
