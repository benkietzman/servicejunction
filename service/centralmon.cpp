// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Service Junction
// -------------------------------------
// file       : centralmon.cpp
// author     : Ben Kietzman
// begin      : 2025-06-30
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
// {{{ includes
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;
#include <Json>
using namespace common;
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  string strJson;

  if (getline(cin, strJson))
  {
    Json *ptJson = new Json(strJson);
    if (ptJson->m.find("processes") != ptJson->m.end() && !ptJson->m["processes"]->v.empty() && ptJson->m.find("maxProcesses") != ptJson->m.end() && !ptJson->m["maxProcesses"]->v.empty())
    {
      size_t unMaxProcesses, unProcesses;
      stringstream ssMaxProcesses(ptJson->m["maxProcesses"]->v), ssProcesses(ptJson->m["processes"]->v);
      ssMaxProcesses >> unMaxProcesses;
      ssProcesses >> unProcesses;
      if (unProcesses > unMaxProcesses)
      {
        system("/usr/bin/systemctl restart junction");
      }
    }
    delete ptJson;
  }

  return 0;
}
// }}}
