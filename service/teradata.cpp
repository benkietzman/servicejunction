// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Teradata
// -------------------------------------
// file       : teradata.cpp
// author     : Ben Kietzman
// begin      : 2022-10-26
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

#include <iostream>
#include <list>
#include <map>
#include <sql.h>
#include <sqlext.h>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
#include <Json>
#include <StringManip>
using namespace common;
#include "include/functions"

string getError(SQLHANDLE handle, SQLSMALLINT type);

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  map<string, string> requestArray;
  list<map<string, string> > responseArray;
  list<string> request;
  string strDriver, strError, strLine, strServer;
  StringManip manip;

  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg.size() > 9 && strArg.substr(0, 9) == "--driver=")
    {
      strDriver = strArg.substr(9, strArg.size() - 9);
      manip.purgeChar(strDriver, strDriver, "'");
      manip.purgeChar(strDriver, strDriver, "\"");
    }
  }
  loadRequest(requestArray, request);
  if (requestArray.find("Server") != requestArray.end() && !requestArray["Server"].empty())
  {
    if (requestArray["Server"].find("//") != string::npos)
    {
      manip.getToken(strServer, requestArray["Server"], 3, "/", false);
    }
    else
    {
      strServer = requestArray["Server"];
    }
  }
  else if (requestArray.find("URL") != requestArray.end() && !requestArray["URL"].empty())
  {
    manip.getToken(strServer, requestArray["URL"], 3, "/", false);
  }
  if (!strDriver.empty() && requestArray.find("User") != requestArray.end() && !requestArray["User"].empty() && requestArray.find("Password") != requestArray.end() && !requestArray["Password"].empty() && !strServer.empty() && ((requestArray.find("Query") != requestArray.end() && !requestArray["Query"].empty()) || (requestArray.find("Update") != requestArray.end() && !requestArray["Update"].empty())))
  {
    SQLHENV env;
    if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env)))
    {
      SQLHDBC dbc;
      SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
      if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc)))
      {
        SQLCHAR outstr[1024];
        SQLSMALLINT outstrlen;
        stringstream ssConnection;
        ssConnection << "DRIVER=" << strDriver << ";DBCName=" << strServer << ";UID=" << requestArray["User"] << ";PWD=" << requestArray["Password"];
        if (SQL_SUCCEEDED(SQLDriverConnect(dbc, NULL, (SQLCHAR *)ssConnection.str().c_str(), SQL_NTS, outstr, sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE)))
        {
          SQLHSTMT stmt;
          if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt)))
          {
            if (requestArray.find("Query") != requestArray.end() && !requestArray["Query"].empty())
            {
              if (SQL_SUCCEEDED(SQLExecDirect(stmt, (SQLCHAR *)requestArray["Query"].c_str(), SQL_NTS)))
              {
                SQLSMALLINT columns;
                SQLNumResultCols(stmt, &columns);
                bProcessed = true;
                while (SQL_SUCCEEDED(SQLFetch(stmt)))
                {
                  map<string, string> rowMap;
                  for (SQLUSMALLINT i = 1; i <= columns; i++)
                  {
                    bool bGetting = true;
                    string strField, strValue;
                    stringstream ssIndex;
                    SQLCHAR szBuffer[512], szField[512];
                    SQLLEN nIndicator;
                    SQLRETURN nReturn;
                    SQLSMALLINT nDataType, nDecDig, nNameLength, nNull;
                    SQLULEN unColSize;
                    ssIndex << i;
                    strField = ssIndex.str();
                    if (SQL_SUCCEEDED(SQLDescribeCol(stmt, i, szField, 512, &nNameLength, &nDataType, &unColSize, &nDecDig, &nNull)))
                    {
                      strField = (char *)szField;
                    }
                    while (bGetting && ((nReturn = SQLGetData(stmt, i, SQL_C_CHAR, szBuffer, 512, &nIndicator)) == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO))
                    {
                      SQLSMALLINT nStatus, nStatusLength;
                      if (nIndicator != SQL_NULL_DATA)
                      {
                        SQLINTEGER nLength = ((nIndicator > 511 || nIndicator == SQL_NO_TOTAL)?511:nIndicator);
                        strValue.append((char *)szBuffer, nLength);
                      }
                      if (nReturn == SQL_SUCCESS || SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, i, &nStatus, SQL_INTEGER, &nStatusLength) == SQL_NO_DATA)
                      {
                        bGetting = false;
                      }
                    }
                    rowMap[strField] = strValue;
                  }
                  responseArray.push_back(rowMap);
                }
              }
              else
              {
                stringstream ssError;
                ssError << "SQLExecDirect():  " << getError(stmt, SQL_HANDLE_STMT);
                strError = ssError.str();
              }
            }
            else if (requestArray.find("Update") != requestArray.end() && !requestArray["Update"].empty())
            {
              if (SQL_SUCCEEDED(SQLExecDirect(stmt, (SQLCHAR *)requestArray["Update"].c_str(), SQL_NTS)))
              {
                bProcessed = true;
              }
              else
              {
                stringstream ssError;
                ssError << "SQLExecDirect():  " << getError(stmt, SQL_HANDLE_STMT);
                strError = ssError.str();
              }
            }
          }
          else
          {
            stringstream ssError;
            ssError << "SQLAllocHandle(stmt):  " << getError(stmt, SQL_HANDLE_STMT);
            strError = ssError.str();
          }
          SQLFreeHandle(SQL_HANDLE_STMT, stmt);
          SQLDisconnect(dbc);
        }
        else
        {
          stringstream ssError;
          ssError << "SQLDriverConnect():  " << getError(dbc, SQL_HANDLE_DBC);
          strError = ssError.str();
        }
      }
      else
      {
        stringstream ssError;
        ssError << "SQLAllocHandle(dbc):  " << getError(dbc, SQL_HANDLE_DBC);
        strError = ssError.str();
      }
      SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }
    else
    {
      stringstream ssError;
      ssError << "SQLAllocHandle(env):  " << getError(env, SQL_HANDLE_ENV);
      strError = ssError.str();
    }
    SQLFreeHandle(SQL_HANDLE_ENV, env);
  }
  else if (strDriver.empty())
  {
    strError = "Please configure the driver for the service.";
  }
  else if (requestArray.find("User") == requestArray.end() || requestArray["User"].empty())
  {
    strError = "Please provide the Teradata User.";
  }
  else if (requestArray.find("Password") == requestArray.end() || requestArray["Password"].empty())
  {
    strError = "Please provide the Teradata Password.";
  }
  else if (requestArray.find("Server") == requestArray.end() || requestArray["Server"].empty())
  {
    strError = "Please provide the Teradata Server.";
  }
  else if ((requestArray.find("Query") == requestArray.end() || requestArray["Query"].empty()) && (requestArray.find("Update") == requestArray.end() || requestArray["Update"].empty()))
  {
    strError = "Please provide the Query or Update.";
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

string getError(SQLHANDLE handle, SQLSMALLINT type)
{
  bool bFirst = true;
  stringstream ssMessage;
  SQLCHAR state[ 7 ], text[256];
  SQLINTEGER i = 0, native;
  SQLRETURN ret;
  SQLSMALLINT len;

  do
  {
    ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);
    if (SQL_SUCCEEDED(ret))
    {
      if (bFirst)
      {
        bFirst = false;
      }
      else
      {
        ssMessage << " |  ";
      }
      ssMessage << state << ":" << i << ":" << native << ":" << text;
    }
  } while (ret == SQL_SUCCESS);

  return ssMessage.str();
}
