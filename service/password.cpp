// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Password
// -------------------------------------
// file       : password.cpp
// author     : Ben Kietzman
// begin      : 2014-01-29
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

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <mysql/mysql.h>
#include <netdb.h>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
using namespace std;
#include <Json>
using namespace common;
#include "include/functions"

string escape(const string strIn, string &strOut);

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  map<string, string> requestArray;
  list<string> request;
  string strError, strLine;

  loadRequest(requestArray, request);
  if (requestArray.find("Function") != requestArray.end() && (requestArray["Function"] == "delete" || (requestArray["Function"] == "update" && requestArray.find("NewPassword") != requestArray.end() && !requestArray["NewPassword"].empty()) || requestArray["Function"] == "verify") && requestArray.find("Application") != requestArray.end() && !requestArray["Application"].empty() && requestArray.find("User") != requestArray.end() && !requestArray["User"].empty() && requestArray.find("Password") != requestArray.end() && !requestArray["Password"].empty())
  {
    ifstream inFile;
    string strConf = "/etc/central.conf";
    Json *ptConf = NULL;
    if (requestArray.find("Conf") != requestArray.end() && !requestArray["Conf"].empty())
    {
      strConf = requestArray["Conf"];
    }
    inFile.open(strConf.c_str());
    if (inFile.good())
    {
      string strLine;
      if (getline(inFile, strLine))
      {
        ptConf = new Json(strLine);
      }
    }
    inFile.close();
    if (ptConf != NULL)
    {
      if (ptConf->m.find("Database") != ptConf->m.end() && !ptConf->m["Database"]->v.empty() && ptConf->m.find("Database Password") != ptConf->m.end() && !ptConf->m["Database Password"]->v.empty() && ptConf->m.find("Database Server") != ptConf->m.end() && !ptConf->m["Database Server"]->v.empty() && ptConf->m.find("Database User") != ptConf->m.end() && !ptConf->m["Database User"]->v.empty())
      {
        MYSQL *conn;
        string strValue;
        if ((conn = mysql_init(NULL)) != NULL)
        {
          bool bRetry;
          size_t unAttempt = 0, unPosition;
          string strPort, strServer = ptConf->m["Database Server"]->v;
          unsigned int unPort = 0, unTimeoutConnect = 5, unTimeoutRead = 5, unTimeoutWrite = 5;
          mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &unTimeoutConnect);
          mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &unTimeoutRead);
          mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &unTimeoutWrite);
          if ((unPosition = strServer.find(":", 0)) != string::npos)
          {
            strPort = strServer.substr(unPosition + 1, strServer.size() - (unPosition + 1));
            strServer.erase(unPosition, strServer.size() - unPosition);
          }
          if (!strPort.empty())
          {
            unPort = atoi(strPort.c_str());
          }
          do
          {
            bRetry = false;
            strError.clear();
            if (mysql_real_connect(conn, strServer.c_str(), ptConf->m["Database User"]->v.c_str(), ptConf->m["Database Password"]->v.c_str(), ptConf->m["Database"]->v.c_str(), unPort, NULL, 0) != NULL)
            {
              stringstream ssQuery;
              ssQuery << "select b.aes, b.encrypt, b.id, b.password";
              if (ptConf->m.find("Aes") != ptConf->m.end() && !ptConf->m["Aes"]->v.empty())
              {
                ssQuery << ", aes_decrypt(from_base64(b.password), sha2('" << escape(ptConf->m["Aes"]->v, strValue) << "', 512)) decrypted_password";
              }
              ssQuery << ", c.type from application a, application_account b, account_type c where a.id=b.application_id and b.type_id = c.id and a.name = '" << escape(requestArray["Application"], strValue) << "' and b.user_id = '" << escape(requestArray["User"], strValue) << "'";
              if (requestArray.find("Type") != requestArray.end() && !requestArray["Type"].empty())
              {
                ssQuery << " and c.type = '" << escape(requestArray["Type"], strValue) << "'";
              }
              if (mysql_query(conn, ssQuery.str().c_str()) == 0)
              {
                list<map<string, string> > getAccount;
                MYSQL_FIELD *field;
                MYSQL_RES *result = mysql_store_result(conn);
                MYSQL_ROW row;
                vector<string> fieldVector;
                while ((field = mysql_fetch_field(result)) != NULL)
                {
                  string strValue;
                  strValue.assign(field->name, field->name_length);
                  fieldVector.push_back(strValue);
                }
                while ((row = mysql_fetch_row(result)))
                {
                  map<string, string> getAccountRow;
                  for (unsigned int i = 0; i < fieldVector.size(); i++)
                  {
                    getAccountRow[fieldVector[i]] = (row[i] != NULL)?row[i]:"";
                  }
                  getAccount.push_back(getAccountRow);
                  getAccountRow.clear();
                }
                if (getAccount.size() == 1)
                {
                  bool bVerified = false;
                  map<string, string> getAccountRow = getAccount.front();
                  if (getAccountRow["encrypt"] == "1")
                  {
                    ssQuery.str("");
                    ssQuery << "select id from application_account where id = " << getAccountRow["id"] << " and (`password` = concat('*',upper(sha1(unhex(sha1('" << escape(requestArray["Password"], strValue) << "'))))) or `password` = concat('!',upper(sha2(unhex(sha2('" << escape(requestArray["Password"], strValue) << "', 512)), 512))))";
                    if (mysql_query(conn, ssQuery.str().c_str()) == 0)
                    {
                      MYSQL_RES *subresult = mysql_store_result(conn);
                      if ((row = mysql_fetch_row(subresult)))
                      {
                        bVerified = true;
                      }
                      mysql_free_result(subresult);
                    }
                  }
                  else if (getAccountRow["aes"] == "1")
                  {
                    if (getAccountRow.find("decrypted_password") != getAccountRow.end() && getAccountRow["decrypted_password"] == requestArray["Password"])
                    {
                      bVerified = true;
                    }
                  }
                  else if (getAccountRow["password"] == requestArray["Password"])
                  {
                    bVerified = true;
                  }
                  if (bVerified)
                  {
                    if (requestArray["Function"] == "delete")
                    {
                      ssQuery.str("");
                      ssQuery << "delete from application_account where id = " << getAccountRow["id"];
                      if (mysql_real_query(conn, ssQuery.str().c_str(), ssQuery.str().size()) >= 0)
                      {
                        bProcessed = true;
                      }
                      else
                      {
                        stringstream ssError;
                        ssError << "mysql_real_query(" << mysql_errno(conn) << "):  " << mysql_error(conn);
                        strError = ssError.str();
                      }
                    }
                    else if (requestArray["Function"] == "update")
                    {
                      ssQuery.str("");
                      ssQuery << "update application_account set `password` = ";
                      if (getAccountRow["encrypt"] == "1")
                      {
                        ssQuery << "concat('!',upper(sha2(unhex(sha2('" << escape(requestArray["NewPassword"], strValue) << "', 512)), 512)))";
                      }
                      else if (ptConf->m.find("Aes") != ptConf->m.end() && !ptConf->m["Aes"]->v.empty())
                      {
                        ssQuery << "to_base64(aes_encrypt('" << escape(requestArray["NewPassword"], strValue) << "', sha2('" << escape(ptConf->m["Aes"]->v, strValue) << "', 512))), aes = 1";
                      }
                      else
                      {
                        ssQuery << "'" << escape(requestArray["NewPassword"], strValue) << "'";
                      }
                      ssQuery << " where id = " << getAccountRow["id"];
                      if (mysql_real_query(conn, ssQuery.str().c_str(), ssQuery.str().size()) >= 0)
                      {
                        bProcessed = true;
                      }
                      else
                      {
                        stringstream ssError;
                        ssError << "mysql_real_query(" << mysql_errno(conn) << "):  " << mysql_error(conn);
                        strError = ssError.str();
                      }
                    }
                    else if (requestArray["Function"] == "verify")
                    {
                      bProcessed = true;
                    }
                  }
                  else
                  {
                    strError = "Failed password verification.";
                  }
                  getAccountRow.clear();
                }
                else if (getAccount.empty())
                {
                  strError = "Failed to find the account.";
                }
                else
                {
                  stringstream ssError;
                  ssError << getAccount.size() << " accounts match this criteria.";
                  strError = ssError.str();
                }
                for (auto &i : getAccount)
                {
                  i.clear();
                }
                getAccount.clear();
                mysql_free_result(result);
                fieldVector.clear();
              }
              else
              {
                stringstream ssError;
                ssError << "mysql_query(" << mysql_errno(conn) << "):  " << mysql_error(conn);
                strError = ssError.str();
              }
            }
            else
            {
              stringstream ssError;
              ssError << "mysql_real_connect(" << mysql_errno(conn) << "):  " << mysql_error(conn);
              strError = ssError.str();
              if (mysql_errno(conn) == 2005 || mysql_errno(conn) == 2013 || mysql_errno(conn) == 2026)
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
      else if (ptConf->m.find("Database") == ptConf->m.end() || ptConf->m["Database"]->v.empty())
      {
        strError = (string)"Failed to read the Database field from the " + strConf + (string)" file.";
      }
      else if (ptConf->m.find("Database Password") == ptConf->m.end() || ptConf->m["Database Password"]->v.empty())
      {
        strError = (string)"Failed to read the Database Password field from the " + strConf + (string)" file.";
      }
      else if (ptConf->m.find("Database Server") == ptConf->m.end() || ptConf->m["Database Server"]->v.empty())
      {
        strError = (string)"Failed to read the Database Server field from the " + strConf + (string)" file.";
      }
      else if (ptConf->m.find("Database User") == ptConf->m.end() || ptConf->m["Database User"]->v.empty())
      {
        strError = (string)"Failed to read the Database User field from the " + strConf + (string)" file.";
      }
      else
      {
        strError = (string)"Failed to read an unknown field from the " + strConf + (string)" file.";
      }
      delete ptConf;
    }
    else
    {
      strError = (string)"Failed to load the " + strConf + (string)" file for reading.";
    }
  }
  else if (requestArray.find("Function") == requestArray.end() || requestArray["Function"].empty())
  {
    strError = "Please provide the Function with a value of delete, update, or verify.";
  }
  else if (requestArray["Function"] != "update" && (requestArray.find("NewPassword") == requestArray.end() || requestArray["NewPassword"].empty()))
  {
    strError = "Please provide the NewPassword when using the update Function.";
  }
  else if (requestArray.find("Application") == requestArray.end() || requestArray["Application"].empty())
  {
    strError = "Please provide the Application.";
  }
  else if (requestArray.find("User") == requestArray.end() || requestArray["User"].empty())
  {
    strError = "Please provide the User.";
  }
  else if (requestArray.find("Password") == requestArray.end() || requestArray["Password"].empty())
  {
    strError = "Please provide the Password.";
  }
  else
  {
    strError = "Encountered an uncaught error.";
  }
  requestArray["Status"] = (string)((bProcessed)?"okay":"error");
  if (!strError.empty())
  {
    requestArray["Error"] = strError;
  }
  ptJson = new Json(requestArray);
  cout << ptJson << endl;
  delete ptJson;
  request.clear();

  return 0;
}

string escape(const string strIn, string &strOut)
{
  size_t unSize = strIn.size();
  stringstream ssResult;

  for (size_t i = 0; i < unSize; i++)
  {
    switch (strIn[i])
    {
      case 0    : ssResult << "\\0";  break;
      case '\n' : ssResult << "\\n";  break;
      case '\r' : ssResult << "\\r";  break;
      case '\\' : ssResult << "\\\\"; break;
      case '\'' : ssResult << "\\'";  break;
      case '"'  : ssResult << "\\\""; break;
      default   : ssResult << strIn[i];
    };
  }

  return (strOut = ssResult.str());
}
