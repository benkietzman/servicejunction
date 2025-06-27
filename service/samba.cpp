/* -*- c++ -*- */
///////////////////////////////////////////
// Samba
// -------------------------------------
// file       : samba.cpp
// author     : Ben Kietzman
// begin      : 2018-05-23
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file samba.cpp
* \brief Samba
*
* Processes Samba requests.
*/
#include <cerrno>
#include <cstring>
#include <iostream>
#include <poll.h>
#include <sstream>
#include <string>
#include <unistd.h>
using namespace std;
#include <Json>
#include <Samba>
#include <StringManip>
using namespace common;
#include "include/functions"

#define PARENT_READ  readpipe[0]
#define CHILD_WRITE  readpipe[1]
#define CHILD_READ   writepipe[0]
#define PARENT_WRITE writepipe[1]

int main(int argc, char *argv[])
{
  bool bProcessed = false;
  Json *ptJson;
  list<Json *> request, response;
  string strError, strValue;
  stringstream ssError;
  StringManip manip;

  loadRequest(request);
  if (request.size() >= 1)
  {
    Json *ptRequest = NULL;
    if (request.size() == 2)
    {
      ptRequest = new Json(request.back());
    }
    else
    {
      ptRequest = new Json;
    }
    ptJson = new Json(request.front());
    if (ptJson->m.find("User") != ptJson->m.end() && !ptJson->m["User"]->v.empty())
    {
      if (ptJson->m.find("Password") != ptJson->m.end() && !ptJson->m["Password"]->v.empty())
      {
        if (ptJson->m.find("Domain") != ptJson->m.end() && !ptJson->m["Domain"]->v.empty())
        {
          if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
          {
            string strIp = ptJson->m["Domain"]->v, strPath, strShare = "netlogon";
            Samba samba;
            if (ptRequest->m.find("IP") != ptRequest->m.end() && !ptRequest->m["IP"]->v.empty())
            {
              strIp = ptRequest->m["IP"]->v;
            }
            if (ptRequest->m.find("Path") != ptRequest->m.end() && !ptRequest->m["Path"]->v.empty())
            {
              strPath = ptRequest->m["Path"]->v;
            }
            if (ptRequest->m.find("Share") != ptRequest->m.end() && !ptRequest->m["Share"]->v.empty())
            {
              strShare = ptRequest->m["Share"]->v;
            }
            if (samba.init(strIp, strShare, ptJson->m["Domain"]->v, ptJson->m["User"]->v, ptJson->m["Password"]->v, strError))
            {
              // {{{ directoryExist
              if (ptJson->m["Function"]->v == "directoryExist")
              {
                if (strPath.empty() || samba.directoryExist(strPath, strError))
                {
                  bProcessed = true;
                }
              }
              // }}}
              // {{{ directoryList
              else if (ptJson->m["Function"]->v == "directoryList")
              {
                list<string> items;
                if (samba.directoryList(strPath, items, strError))
                {
                  bProcessed = true;
                  response.push_back(new Json(items));
                }
                items.clear();
              }
              // }}}
              // {{{ establishDirectory
              else if (ptJson->m["Function"]->v == "establishDirectory")
              {
                if (!strPath.empty())
                {
                  if (samba.establishDirectory(strPath, strError))
                  {
                    bProcessed = true;
                  }
                }
                else
                {
                  strError = "Please provide the Path.";
                }
              }
              // }}}
              // {{{ fileExist
              else if (ptJson->m["Function"]->v == "fileExist")
              {
                if (!strPath.empty())
                {
                  if (samba.fileExist(strPath, strError))
                  {
                    bProcessed = true;
                  }
                }
                else
                {
                  strError = "Please provide the Path.";
                }
              }
              // }}}
              // {{{ fileSize
              else if (ptJson->m["Function"]->v == "fileSize")
              {
                if (!strPath.empty())
                {
                  long lSize;
                  if (samba.fileSize(strPath, lSize, strError))
                  {
                    stringstream ssSize;
                    Json *ptSubJson = new Json;
                    bProcessed = true;
                    ssSize << lSize;
                    ptSubJson->insert("Size", ssSize.str(), 'n');
                    response.push_back(ptSubJson);
                  }
                }
                else
                {
                  strError = "Please provide the Path.";
                }
              }
              // }}}
              // {{{ get
              else if (ptJson->m["Function"]->v == "get")
              {
                if (!strPath.empty())
                {
                  stringstream ssData;
                  if (samba.get(strPath, ssData, strError))
                  {
                    string strData;
                    Json *ptSubJson = new Json;
                    manip.encodeBase64(ssData.str(), strData);
                    bProcessed = true;
                    ptSubJson->insert("Data", strData);
                    response.push_back(ptSubJson);
                  }
                }
                else
                {
                  strError = "Please provide the Path.";
                }
              }
              // }}}
              // {{{ login
              else if (ptJson->m["Function"]->v == "login")
              {
                string strFile = "DOSVER.COM";
                if (ptRequest->m.find("File") != ptRequest->m.end() && !ptRequest->m["File"]->v.empty())
                {
                  strFile = ptRequest->m["File"]->v;
                }
                if (samba.fileExist(strFile, strError) || strError == "Not found.")
                {
                  bProcessed = true;
                }
              }
              // }}}
              // {{{ makeDirectory
              else if (ptJson->m["Function"]->v == "makeDirectory")
              {
                if (!strPath.empty())
                {
                  if (samba.makeDirectory(strPath, strError))
                  {
                    bProcessed = true;
                  }
                }
                else
                {
                  strError = "Please provide the Path.";
                }
              }
              // }}}
              // {{{ password
              else if (ptJson->m["Function"]->v == "password")
              {
                if (ptRequest->m.find("Password") != ptRequest->m.end() && !ptRequest->m["Password"]->v.empty())
                {
                  char *args[100], *pszArgument;
                  int readpipe[2] = {-1, -1}, writepipe[2] = {-1, -1};
                  pid_t nPid;
                  size_t unIndex = 0;
                  string strArgument;
                  stringstream ssCommand;
                  ssCommand << "/usr/bin/smbpasswd -r " << ptJson->m["Domain"]->v << " -U " << ptJson->m["User"]->v;
                  while (ssCommand >> strArgument)
                  {
                    pszArgument = new char[strArgument.size() + 1];
                    strcpy(pszArgument, strArgument.c_str());
                    args[unIndex++] = pszArgument;
                  }
                  args[unIndex] = NULL;
                  if (pipe(readpipe) == 0)
                  {
                    if (pipe(writepipe) == 0)
                    {
                      if ((nPid = fork()) == 0)
                      {
                        close(PARENT_WRITE);
                        close(PARENT_READ);
                        dup2(CHILD_READ, 0);
                        close(CHILD_READ);
                        dup2(CHILD_WRITE, 1);
                        dup2(CHILD_WRITE, 2);
                        close(CHILD_WRITE);
                        execve(args[0], args, environ);
                        _exit(1);
                      }
                      else if (nPid > 0)
                      {
                        bool bExit = false;
                        char szBuffer[1024];
                        int nReturn;
                        size_t unPosition, unSockets;
                        string strBuffer[2], strLine;
                        close(CHILD_READ);
                        close(CHILD_WRITE);
                        while (!bExit)
                        {
                          pollfd fds[2];
                          unSockets = 0;
                          fds[unSockets].fd = PARENT_READ;
                          fds[unSockets].events = POLLIN;
                          unSockets++;
                          if (!strBuffer[1].empty())
                          {
                            fds[unSockets].fd = PARENT_WRITE;
                            fds[unSockets].events = POLLOUT;
                            unSockets++;
                          }
                          if ((nReturn = poll(fds, unSockets, 250)) > 0)
                          {
                            if (fds[0].revents & POLLIN)
                            {
                              if ((nReturn = read(PARENT_READ, szBuffer, 1024)) > 0)
                              {
                                strBuffer[0].append(szBuffer, nReturn);
                                if (strBuffer[0][0] == '\n')
                                {
                                  strBuffer[0].erase(0, 1);
                                }
                                if (strBuffer[0] == "Old SMB password:")
                                {
                                  strBuffer[0].clear();
                                  strBuffer[1] += ptJson->m["Password"]->v + "\n";
                                }
                                else if (strBuffer[0] == "New SMB password:" || strBuffer[0] == "Retype new SMB password:")
                                {
                                  strBuffer[0].clear();
                                  strBuffer[1] += ptRequest->m["Password"]->v + "\n";
                                }
                                else
                                {
                                  if ((unPosition = strBuffer[0].find("\n")) != string::npos)
                                  {
                                    bExit = true;
                                    strLine = strBuffer[0].substr(0, unPosition);
                                    strBuffer[0].erase(0, (unPosition + 1));
                                    if (strLine.find("Error") != string::npos)
                                    {
                                      if ((unPosition = strLine.rfind(" : ")) != string::npos && (unPosition + 3) < strLine.size())
                                      {
                                        unPosition += 3;
                                        strError = strLine.substr(unPosition, (strLine.size() - unPosition));
                                        if (strError[(strError.size() - 1)] == '.')
                                        {
                                          strError.erase((strError.size() - 1), 1);
                                        }
                                      }
                                      else
                                      {
                                        strError = strLine;
                                      }
                                    }
                                    else
                                    {
                                      bProcessed = true;
                                    }
                                  }
                                }
                              }
                              else
                              {
                                bExit = true;
                                if (nReturn < 0)
                                {
                                  ssError.str("");
                                  ssError << "read(" << errno << ") " << strerror(errno);
                                  strError = ssError.str();
                                }
                              }
                            }
                            if (!strBuffer[1].empty() && fds[1].revents & POLLOUT)
                            {
                              if ((nReturn = write(PARENT_WRITE, strBuffer[1].c_str(), strBuffer[1].size())) > 0)
                              {
                                strBuffer[1].erase(0, nReturn);
                              }
                              else
                              {
                                bExit = true;
                                if (nReturn < 0)
                                {
                                  ssError.str("");
                                  ssError << "write(" << errno << ") " << strerror(errno);
                                  strError = ssError.str();
                                }
                              }
                            }
                          }
                          else if (nReturn < 0)
                          {
                            bExit = true;
                            ssError.str("");
                            ssError << "poll(" << errno << ") " << strerror(errno);
                            strError = ssError.str();
                          }
                        }
                        close(PARENT_WRITE);
                        close(PARENT_READ);
                      }
                      else
                      {
                        ssError.str("");
                        ssError << "fork(" << errno << ") " << strerror(errno);
                        strError = ssError.str();
                      }
                    }
                    else
                    {
                      ssError.str("");
                      ssError << "pipe(write," << errno << ") " << strerror(errno);
                      strError = ssError.str();
                    }
                  }
                  else
                  {
                    ssError.str("");
                    ssError << "pipe(read," << errno << ") " << strerror(errno);
                    strError = ssError.str();
                  }
                  for (unsigned int i = 0; i < unIndex; i++)
                  {
                    delete[] args[i];
                  }
                }
                else
                {
                  strError = "Please provide the new Password.";
                }
              }
              // }}}
              // {{{ put
              else if (ptJson->m["Function"]->v == "put")
              {
                if (ptRequest->m.find("Data") != ptRequest->m.end() && !ptRequest->m["Data"]->v.empty())
                {
                  if (!strPath.empty())
                  {
                    string strData;
                    manip.decodeBase64(ptRequest->m["Data"]->v, strData);
                    stringstream ssData(strData);
                    if (samba.put(ssData, strPath, strError))
                    {
                      bProcessed = true;
                    }
                  }
                  else
                  {
                    strError = "Please provide the Path.";
                  }
                }
                else
                {
                  strError = "Please provide the Data.";
                }
              }
              // }}}
              // {{{ remove
              else if (ptJson->m["Function"]->v == "remove")
              {
                if (!strPath.empty())
                {
                  if (samba.remove(strPath, strError))
                  {
                    bProcessed = true;
                  }
                }
                else
                {
                  strError = "Please provide the Path.";
                }
              }
              // }}}
              // {{{ removeDirectory
              else if (ptJson->m["Function"]->v == "removeDirectory")
              {
                if (!strPath.empty())
                {
                  if (samba.removeDirectory(strPath, strError))
                  {
                    bProcessed = true;
                  }
                }
                else
                {
                  strError = "Please provide the Path.";
                }
              }
              // }}}
              // {{{ rename
              else if (ptJson->m["Function"]->v == "rename")
              {
                if (ptRequest->m.find("OldPath") != ptRequest->m.end() && !ptRequest->m["OldPath"]->v.empty())
                {
                  if (ptRequest->m.find("NewPath") != ptRequest->m.end() && !ptRequest->m["NewPath"]->v.empty())
                  {
                    if (samba.rename(ptRequest->m["OldPath"]->v, ptRequest->m["NewPath"]->v, strError))
                    {
                      bProcessed = true;
                    }
                  }
                  else
                  {
                    strError = "Please provide the NewPath.";
                  }
                }
                else
                {
                  strError = "Please provide the OldPath.";
                }
              }
              // }}}
              // {{{ invalid
              else
              {
                strError = "Please provide a valid Function:  directoryExist, directoryList, establishDirectory, fileExist, fileSize, get, login, makeDirectory, password, put, remove, removeDirectory, rename.";
              }
              // }}}
            }
          }
          else
          {
            strError = "Please provide the Function.";
          }
        }
        else
        {
          strError = "Please provide the Domain.";
        }
      }
      else
      {
        strError = "Please provide the Password.";
      }
    }
    else
    {
      strError = "Please provide the User.";
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
