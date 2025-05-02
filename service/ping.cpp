/* -*- c++ -*- */
///////////////////////////////////////////
// Ping
// -------------------------------------
// file       : ping.cpp
// author     : Ben Kietzman
// begin      : 2016-10-20
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file ping.cpp
* \brief Ping Program
*
* Responds with a pong.
*/
#include <iostream>
using namespace std;
#include <Json>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  list<Json *> request;
  string strError;

  loadRequest(request);
  if (request.size() == 1)
  {
    ptJson = new Json(request.front());
    bProcessed = true;
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

  return 0;
}
