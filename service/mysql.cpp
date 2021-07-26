// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// MySQL
// -------------------------------------
// file       : mysql.cpp
// author     : Ben Kietzman
// begin      : 2011-12-22
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
#include <mysql/mysql.h>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
#include <Json>
using namespace common;
#include "include/functions"

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  map<string, string> requestArray;
  list<map<string, string> > responseArray;
  list<string> request;
  string strError, strLine;

  loadRequest(requestArray, request);
  if (requestArray.find("User") != requestArray.end() && !requestArray["User"].empty() && requestArray.find("Password") != requestArray.end() && !requestArray["Password"].empty() && requestArray.find("Server") != requestArray.end() && !requestArray["Server"].empty() && requestArray.find("Database") != requestArray.end() && !requestArray["Database"].empty() && ((requestArray.find("Query") != requestArray.end() && !requestArray["Query"].empty()) || (requestArray.find("Update") != requestArray.end() && !requestArray["Update"].empty())))
  {
    MYSQL *conn;
    if ((conn = mysql_init(NULL)) != NULL)
    {
      bool bRetry;
      size_t unAttempt = 0, unPosition;
      unsigned int unPort = 0, unTimeout = 2;
      mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &unTimeout);
      if ((unPosition = requestArray["Server"].find(":", 0)) != string::npos)
      {
        requestArray["Port"] = requestArray["Server"].substr(unPosition + 1, requestArray["Server"].size() - (unPosition + 1));
        requestArray["Server"].erase(unPosition, requestArray["Server"].size() - unPosition);
      }
      if (requestArray.find("Port") != requestArray.end() && !requestArray["Port"].empty())
      {
        unPort = atoi(requestArray["Port"].c_str());
      }
      do
      {
        bRetry = false;
        strError.clear();
        if (mysql_real_connect(conn, requestArray["Server"].c_str(), requestArray["User"].c_str(), requestArray["Password"].c_str(), requestArray["Database"].c_str(), unPort, NULL, 0) != NULL)
        {
          if (requestArray.find("Query") != requestArray.end() && !requestArray["Query"].empty())
          {
            if (mysql_query(conn, requestArray["Query"].c_str()) == 0)
            {
              MYSQL_RES *result = NULL;
              bProcessed = true;
              if ((result = mysql_store_result(conn)) != NULL)
              {
                vector<string> fieldVector;
                MYSQL_ROW row;
                MYSQL_FIELD *field;
                while ((field = mysql_fetch_field(result)) != NULL)
                {
                  string strValue;
                  strValue.assign(field->name, field->name_length);
                  fieldVector.push_back(strValue);
                }
                while ((row = mysql_fetch_row(result)))
                {
                  map<string, string> rowMap;
                  for (unsigned int i = 0; i < fieldVector.size(); i++)
                  {
                    rowMap[fieldVector[i]] = (row[i] != NULL)?row[i]:"";
                  }
                  responseArray.push_back(rowMap);
                  rowMap.clear();
                }
                fieldVector.clear();
                mysql_free_result(result);
              }
            }
            else
            {
              stringstream ssError;
              ssError << "mysql_query(" << mysql_errno(conn) << "):  " << mysql_error(conn);
              strError = ssError.str();
            }
          }
          else if (requestArray.find("Update") != requestArray.end() && !requestArray["Update"].empty())
          {
            if (mysql_real_query(conn, requestArray["Update"].c_str(), requestArray["Update"].size()) == 0)
            {
              stringstream ssRows;
              my_ulonglong ulID = mysql_insert_id(conn), ulRows = mysql_affected_rows(conn);
              bProcessed = true;
              if (ulID > 0)
              {
                stringstream ssID;
                ssID << ulID;
                requestArray["ID"] = ssID.str();
              }
              ssRows << ulRows;
              requestArray["Rows"] = ssRows.str();
            }
            else
            {
              stringstream ssError;
              ssError << "mysql_query(" << mysql_errno(conn) << "):  " << mysql_error(conn);
              strError = ssError.str();
            }
          }
        }
        else
        {
          stringstream ssError;
          ssError << "mysql_real_connect(" << mysql_errno(conn) << "):  " << mysql_error(conn);
          strError = ssError.str();
          if (mysql_errno(conn) == 2005 || mysql_errno(conn) == 2026)
          {
            bRetry = true;
          }
        }
      } while (bRetry && unAttempt++ < 10);
    }
    else
    {
      stringstream ssError;
      ssError << "mysql_init(" << mysql_errno(conn) << "):  " << mysql_error(conn);
      strError = ssError.str();
    }
    mysql_close(conn);
  }
  else if (requestArray.find("User") == requestArray.end() || requestArray["User"].empty())
  {
    strError = "Please provide the MySQL User.";
  }
  else if (requestArray.find("Password") == requestArray.end() || requestArray["Password"].empty())
  {
    strError = "Please provide the MySQL Password.";
  }
  else if (requestArray.find("Server") == requestArray.end() || requestArray["Server"].empty())
  {
    strError = "Please provide the MySQL Server.";
  }
  else if (requestArray.find("Database") == requestArray.end() || requestArray["Database"].empty())
  {
    strError = "Please provide the MySQL Database.";
  }
  else if ((requestArray.find("Query") == requestArray.end() || requestArray["Query"].empty()) && (requestArray.find("Update") == requestArray.end() || requestArray["Update"].empty()))
  {
    strError = "Please provide the MySQL Query or Update.";
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
