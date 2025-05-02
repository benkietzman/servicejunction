/* -*- c++ -*- */
///////////////////////////////////////////
// MSSQL
// -------------------------------------
// file       : mssql.cpp
// author     : Ben Kietzman
// begin      : 2015-03-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
#include <iostream>
#include <list>
#include <map>
#include <netdb.h>
#include <sstream>
#include <string>
#include <sybdb.h>
#include <sybfront.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
using namespace std;
#include <Json>
using namespace common;
#include "include/functions"

static string gstrError;

int mssqlHandle(DBPROCESS *dbconn, int nSeverity, int nDBError, int OSError, char *pszDBError, char *pszOSError);

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  map<string, string> requestArray;
  list<map<string, string> > responseArray;
  list<string> request;
  string strError, strLine;

  loadRequest(requestArray, request);
  if (requestArray.find("User") != requestArray.end() && !requestArray["User"].empty() && requestArray.find("Password") != requestArray.end() && !requestArray["Password"].empty() && requestArray.find("Server") != requestArray.end() && !requestArray["Server"].empty() && ((requestArray.find("Query") != requestArray.end() && !requestArray["Query"].empty()) || (requestArray.find("Update") != requestArray.end() && !requestArray["Update"].empty())))
  {
    addrinfo hints, *result;
    bool bConnected[3] = {false, false, false};
    int nReturn;
    string strPort, strServer;
    stringstream ssServerPort(requestArray["Server"]);
    getline(ssServerPort, strServer, ':');
    getline(ssServerPort, strPort, ':');
    if (strPort.empty())
    {
      strPort = "1433";
    }
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((nReturn = getaddrinfo(strServer.c_str(), strPort.c_str(), &hints, &result)) == 0)
    {
      addrinfo *rp;
      int fdSocket;
      bConnected[0] = true;
      for (rp = result; !bConnected[2] && rp != NULL; rp = rp->ai_next)
      {
        bConnected[1] = false;
        if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
        {
          bConnected[1] = true;
          if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
          {
            bConnected[2] = true;
          }
          close(fdSocket);
        }
      }
      freeaddrinfo(result);
    }
    if (bConnected[2])
    {
      dberrhandle(mssqlHandle);
      if (dbinit() != FAIL)
      {
        LOGINREC *login;
        if ((login = dblogin()) != FAIL)
        {
          DBPROCESS *dbconn;
          DBSETLUSER(login, requestArray["User"].c_str());
          DBSETLPWD(login, requestArray["Password"].c_str());
          if ((dbconn = dbopen(login, requestArray["Server"].c_str())) != NULL)
          {
            if (requestArray.find("Query") != requestArray.end() && !requestArray["Query"].empty())
            {
              dbcmd(dbconn, requestArray["Query"].c_str());
              if (dbsqlexec(dbconn) != FAIL)
              {
                bProcessed = true;
                if (dbresults(dbconn) == SUCCEED)
                {
                  int nCols = dbnumcols(dbconn);
                  map<int, string> cols;
                  map<string, DBCHAR *> col;
                  for (int i = 1; i <= nCols; i++)
                  {
                    string strKey = dbcolname(dbconn, i);
                    col[strKey] = new DBCHAR[1024];
                    dbbind(dbconn, i, NTBSTRINGBIND, 0, (BYTE *)col[strKey]);
                  }
                  while (dbnextrow(dbconn) != NO_MORE_ROWS)
                  {
                    map<string, string> row;
                    for (auto &i : col)
                    {
                      row[i.first] = i.second;
                    }
                    responseArray.push_back(row);
                    row.clear();
                  }
                  for (auto &i : col)
                  {
                    delete i.second;
                  }
                  col.clear();
                }
                else
                {
                  stringstream ssError;
                  ssError << "dbresults():  " << gstrError;
                  strError = ssError.str();
                }
              }
              else
              {
                stringstream ssError;
                ssError << "dbsqlexec():  " << gstrError;
                strError = ssError.str();
              }
            }
            else if (requestArray.find("Update") != requestArray.end() && !requestArray["Update"].empty())
            {
              dbcmd(dbconn, requestArray["Update"].c_str());
              if (dbsqlexec(dbconn) != FAIL)
              {
                bProcessed = true;
              }
              else
              {
                stringstream ssError;
                ssError << "dbsqlexec():  " << gstrError;
                strError = ssError.str();
              }
            }
          }
          else
          {
            stringstream ssError;
            ssError << "dbopen():  " << gstrError;
            strError = ssError.str();
          }
          dbloginfree(login);
        }
        else
        {
          stringstream ssError;
          ssError << "dblogin():  " << gstrError;
          strError = ssError.str();
        }
        dbexit();
      }
      else
      {
        stringstream ssError;
        ssError << "dbinit():  " << gstrError;
        strError = ssError.str();
      }
    }
    else
    {
      strError = "Please provide a valid Server.";
    }
  }
  else if (requestArray.find("User") == requestArray.end() || requestArray["User"].empty())
  {
    strError = "Please provide the MSSQL User.";
  }
  else if (requestArray.find("Password") == requestArray.end() || requestArray["Password"].empty())
  {
    strError = "Please provide the MSSQL Password.";
  }
  else if (requestArray.find("Server") == requestArray.end() || requestArray["Server"].empty())
  {
    strError = "Please provide the MSSQL Server.";
  }
  else if ((requestArray.find("Query") == requestArray.end() || requestArray["Query"].empty()) && (requestArray.find("Update") == requestArray.end() || requestArray["Update"].empty()))
  {
    strError = "Please provide the MSSQL Query or Update.";
  }
  requestArray["Status"] = (string)((bProcessed)?"okay":"error");
  if (!strError.empty())
  {
    requestArray["Error"] = strError;
  }
  ptJson = new Json(requestArray);
  cout << ptJson << endl;
  delete ptJson;
  if (!responseArray.empty())
  {
    for (auto &i : responseArray)
    {
      ptJson = new Json(i);
      cout << ptJson << endl;
      delete ptJson;
      i.clear();
    }
    responseArray.clear();
  } 
  requestArray.clear();
  request.clear();

  return 0;
}

int mssqlHandle(DBPROCESS *dbconn, int nSeverity, int nDBError, int OSError, char *pszDBError, char *pszOSError)
{
  gstrError = pszDBError;
  if (OSError != DBNOERR)
  {
    gstrError += (string)" --- " + pszOSError;
  }

  return INT_CANCEL;
}
