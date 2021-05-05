// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Service Junction
// -------------------------------------
// file       : junction.cpp
// author     : Ben Kietzman
// begin      : 2011-04-13
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

/*! \file junction.cpp
* \brief Junction Daemon
*
* Provides socket-level access to services.
*/
// {{{ includes
#include <arpa/inet.h>
#include <cerrno>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <ctype.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pcrecpp.h>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;
#include "../common/Central"
#include "../common/Json"
#include "../common/SignalHandling"
#include "../common/Syslog"
using namespace common;
// }}}
// {{{ defines
#ifdef VERSION
#undef VERSION
#endif
/*! \def VERSION
* \brief Contains the application version number.
*/
#define VERSION "0.1"
/*! \def mUSAGE(A)
* \brief Prints the usage statement.
*/
#define mUSAGE(A) cout << endl << "Usage:  "<< A << " [options]"  << endl << endl << " -d, --daemon" << endl << "     Turns the process into a daemon." << endl << endl << "     --data" << endl << "     Sets the data directory." << endl << endl << " -e EMAIL, --email=EMAIL" << endl << "     Provides the email address for default notifications." << endl << endl << " -h, --help" << endl << "     Displays this usage screen." << endl << endl << "           --max-buffer=[MAX BUFFER]" << endl << "     Provides the maximum input buffer limit in MBs." << endl << endl << "           --max-lines=[MAX LINES]" << endl << "     Provides the maximum input lines limit." << endl << endl << "     --syslog" << endl << "     Enables syslog." << endl << endl << " -v, --version" << endl << "     Displays the current version of this software." << endl << endl
/*! \def mVER_USAGE(A,B)
* \brief Prints the version number.
*/
#define mVER_USAGE(A,B) cout << endl << A << " Version: " << B << endl << endl
/*! \def CERTIFICATE
* \brief Contains the certificate path.
*/
#define CERTIFICATE "/server.crt"
/*! \def CHILD_TIMEOUT
* \brief Supplies the child timeout.
*/
#define CHILD_TIMEOUT 3600
/*! \def maxfd()
* \brief Determines the maximum file descriptor.
*/  
#define maxfd(x,y) ((x) > (y) ? (x) : (y))
/*! \def PID
* \brief Contains the PID path.
*/
#define PID "/.pid"
/*! \def PRIVATE_KEY
* \brief Contains the key path.
*/
#define PRIVATE_KEY "/server.key"
/*! \def SECURE_PORT
* \brief Supplies the SSL port.
*/
#define SECURE_PORT "5863"
/*! \def STANDARD_PORT
* \brief Supplies the standard port.
*/
#define STANDARD_PORT "5862"
/*! \def CONCENTRATOR_PORT
* \brief Supplies the concentrator port.
*/
#define CONCENTRATOR_PORT "5864"
/*! \def SERVICE_CONFIG
* \brief Contains the service configuration path.
*/
#define SERVICE_CONFIG "/services.conf"
/*! \def START
* \brief Contains the start path.
*/
#define START "/.start"
#define PARENT_READ  readpipe[0]
#define CHILD_WRITE  readpipe[1]
#define CHILD_READ   writepipe[0]
#define PARENT_WRITE writepipe[1]
// }}}
// {{{ structs
struct connection
{
  int readpipe;
  int writepipe;
  pid_t childPid;
  string strBuffer[2];
  string strCommand;
  time_t CStartTime;
  time_t CEndTime;
  time_t CTimeout;
  Json *ptRequest;
};
// }}}
// {{{ global variables
extern char **environ;
static bool gbDaemon = false; //!< Global daemon variable.
static bool gbShutdown = false; //!< Global shutdown variable.
static size_t gunMaxBuffer = 0; //!< Global buffer limit.
static size_t gunMaxLines = 0; //!< Global lines limit.
static string gstrApplication = "Service Junction"; //!< Global application name.
static string gstrData = "/data/servicejunction"; //!< Global data path.
static string gstrEmail; //!< Global notification email address.
static Central *gpCentral = NULL; //!< Contains the Central class.
static Syslog *gpSyslog = NULL; //!< Contains the Syslog class.
// }}}
// {{{ prototypes
/*! \fn void sighandle(const int nSignal)
* \brief Establishes signal handling for the application.
* \param nSignal Contains the caught signal.
*/
void sighandle(const int nSignal);
// }}}
// {{{ main()
/*! \fn int main(int argc, char *argv[])
* \brief This is the main function.
* \return Exits with a return code for the operating system.
*/
int main(int argc, char *argv[])
{
  string strError;

  gpCentral = new Central(strError);
  // {{{ set signal handling
  sethandles(sighandle);
  sigignore(SIGBUS);
  sigignore(SIGCHLD);
  sigignore(SIGCONT);
  sigignore(SIGPIPE);
  sigignore(SIGSEGV);
  sigignore(SIGWINCH);
  // }}}
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-d" || strArg == "--daemon")
    {
      gbDaemon = true;
    }
    else if (strArg.size() > 7 && strArg.substr(0, 7) == "--data=")
    {
      gstrData = strArg.substr(7, strArg.size() - 7);
      gpCentral->manip()->purgeChar(gstrData, gstrData, "'");
      gpCentral->manip()->purgeChar(gstrData, gstrData, "\"");
    }
    else if (strArg == "-e" || (strArg.size() > 8 && strArg.substr(0, 8) == "--email="))
    {
      if (strArg == "-e" && i + 1 < argc && argv[i+1][0] != '-')
      {
        gstrEmail = argv[++i];
      }
      else
      {
        gstrEmail = strArg.substr(8, strArg.size() - 8);
      }
      gpCentral->manip()->purgeChar(gstrEmail, gstrEmail, "'");
      gpCentral->manip()->purgeChar(gstrEmail, gstrEmail, "\"");
    }
    else if (strArg == "-h" || strArg == "--help")
    {
      mUSAGE(argv[0]);
      return 0;
    }
    else if (strArg.size() > 13 && strArg.substr(0, 13) == "--max-buffer=")
    {
      string strMaxBuffer = strArg.substr(13, strArg.size() - 13);
      stringstream ssMaxBuffer;
      gpCentral->manip()->purgeChar(strMaxBuffer, strMaxBuffer, "'");
      gpCentral->manip()->purgeChar(strMaxBuffer, strMaxBuffer, "\"");
      ssMaxBuffer.str(strMaxBuffer);
      ssMaxBuffer >> gunMaxBuffer;
    }
    else if (strArg.size() > 12 && strArg.substr(0, 12) == "--max-lines=")
    {
      string strMaxLines = strArg.substr(12, strArg.size() - 12);
      stringstream ssMaxLines;
      gpCentral->manip()->purgeChar(strMaxLines, strMaxLines, "'");
      gpCentral->manip()->purgeChar(strMaxLines, strMaxLines, "\"");
      ssMaxLines.str(strMaxLines);
      ssMaxLines >> gunMaxLines;
    }
    else if (strArg == "--syslog")
    {
      gpSyslog = new Syslog(gstrApplication, "junction");
    }
    else if (strArg == "-v" || strArg == "--version")
    {
      mVER_USAGE(argv[0], VERSION);
      return 0;
    }
    else
    {
      cout << endl << "Illegal option, '" << strArg << "'." << endl;
      mUSAGE(argv[0]);
      return 0;
    }
  }
  // }}}
  gpCentral->utility()->sslInit();
  gpCentral->setApplication(gstrApplication);
  gpCentral->setEmail(gstrEmail);
  gpCentral->setLog(gstrData, "junction_", "daily", true, true);
  gpCentral->setRoom("#system");
  // {{{ normal run
  if (!gstrEmail.empty())
  {
    if (!gbShutdown)
    {
      pid_t nConcentrator;
      if (gbDaemon)
      {
        gpCentral->utility()->daemonize();
      }
      setlocale(LC_ALL, "");
      ofstream outPid((gstrData + PID).c_str());
      if (outPid.good())
      {
        outPid << getpid() << endl;
      }
      outPid.close();
      ofstream outStart((gstrData + START).c_str());
      outStart.close();
      if ((nConcentrator = fork()) >= 0)
      {
        bool bChild = false, bConcentrator = false, bSecure = false, bStandard = false;
        SSL_CTX *ctx = NULL;
        if (nConcentrator > 0)
        {
          pid_t nFork;
          if ((nFork = fork()) > 0)
          {
            bConcentrator = true;
            bSecure = true;
          }
          else if (nFork == 0)
          {
            bChild = true;
            bSecure = true;
          }
          else
          {
            gpCentral->alert((string)"fork() error [concentrator->secure]: " + strerror(errno));
          }
        }
        else
        {
          bChild = true;
          bStandard = true;
        }
        if (bSecure)
        {
          stringstream ssMessage;
          if ((ctx = gpCentral->utility()->sslInitServer((gstrData + CERTIFICATE), (gstrData + PRIVATE_KEY), strError)) != NULL)
          {
            ssMessage.str("");
            ssMessage << "CentralAddons::utility()->sslInitServer():  SSL initialization was successful.";
            gpCentral->log(ssMessage.str());
          }
          else
          {
            gbShutdown = true;
            ssMessage.str("");
            ssMessage << "CentralAddons::utility()->sslInitServer() error:  " << strError;
            gpCentral->notify(ssMessage.str());
          }
        }
        if (!gbShutdown)
        {
            struct addrinfo hints;
            struct addrinfo *result;
            int fdSocket = -1, nReturn;
            time_t ulModifyTime = 0;
            map<string, map<string, string> > service;
            memset(&hints, 0, sizeof(struct addrinfo));
            hints.ai_family = AF_INET6;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;
            if ((nReturn = getaddrinfo(NULL, ((bConcentrator)?CONCENTRATOR_PORT:((bStandard)?STANDARD_PORT:SECURE_PORT)), &hints, &result)) == 0)
            {
              bool bBound = false;
              struct addrinfo *rp;
              for (rp = result; !bBound && rp != NULL; rp = rp->ai_next)
              {
                if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
                {
                  int nOn = 1;
                  setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&nOn, sizeof(nOn));
                  if (bind(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                  {
                    bBound = true;
                  }
                  else
                  {
                    close(fdSocket);
                  }
                }
              }
              freeaddrinfo(result);
              if (bBound)
              {
                gpCentral->log((string)"Bound to the " + (string)((bConcentrator)?"concentrator":((bStandard)?"standard":"secure")) + (string)" socket.");
                if (listen(fdSocket, SOMAXCONN) == 0)
                {
                  int fdData;
                  string strSystem;
                  socklen_t clilen;
                  sockaddr_in cli_addr;
                  gpCentral->log((string)"Listening to the " + (string)((bConcentrator)?"concentrator":((bStandard)?"standard":"secure")) + (string)" socket.");
                  clilen = sizeof(cli_addr);
                  while (!gbShutdown && (fdData = accept(fdSocket, (struct sockaddr *)&cli_addr, &clilen)) >= 0)
                  {
                    if (gpCentral->file()->fileExist(gstrData + "/.shutdown"))
                    {
                      ofstream outShutdown;
                      gpCentral->file()->remove(gstrData + "/.shutdown");
                      outShutdown.open((gstrData + "/.shutdown_standard").c_str());
                      outShutdown.close();
                      outShutdown.open((gstrData + "/.shutdown_secure").c_str());
                      outShutdown.close();
                    }
                    if (bConcentrator)
                    {
                      if (gpCentral->file()->fileExist(gstrData + "/.shutdown_concentrator"))
                      {
                        gpCentral->file()->remove(gstrData + "/.shutdown_concentrator");
                        gbShutdown = true;
                      }
                    }
                    else if (bStandard && gpCentral->file()->fileExist(gstrData + "/.shutdown_standard"))
                    {
                      gpCentral->file()->remove(gstrData + "/.shutdown_standard");
                      gbShutdown = true;
                    }
                    else if (bSecure && gpCentral->file()->fileExist(gstrData + "/.shutdown_secure"))
                    {
                      gpCentral->file()->remove(gstrData + "/.shutdown_secure");
                      gbShutdown = true;
                    }
                    // {{{ reload service configuration
                    if (!gpCentral->file()->fileExist(gstrData + (string)"/.lock"))
                    {
                      struct stat tStat;
                      if (stat((gstrData + SERVICE_CONFIG).c_str(), &tStat) == 0)
                      {
                        if (ulModifyTime != tStat.st_mtime)
                        {
                          ifstream inFile;
                          gpCentral->log((string)((ulModifyTime == 0)?"L":"Rel") + (string)"oaded the configuration file.");
                          inFile.open((gstrData + SERVICE_CONFIG).c_str());
                          if (inFile.good())
                          {
                            string strConf;
                            ulModifyTime = tStat.st_mtime;
                            service.clear();
                            while (getline(inFile, strConf).good())
                            {
                              map<string, string> serviceMap;
                              Json *ptJson = new Json(strConf);
                              ptJson->flatten(serviceMap, true, false);
                              delete ptJson;
                              if (!serviceMap.empty())
                              {
                                if (serviceMap.find("Service") != serviceMap.end() && !serviceMap["Service"].empty())
                                {
                                  if (serviceMap.find("Command") != serviceMap.end() && !serviceMap["Command"].empty())
                                  {
                                    if (serviceMap.find("Port") == serviceMap.end() || serviceMap["Port"].empty())
                                    {
                                      serviceMap["Port"] = "both";
                                    }
                                    service[serviceMap["Service"]] = serviceMap;
                                  }
                                  else
                                  {
                                    gpCentral->notify((string)"Invalid service configuration.  Please provide the Command.  " + strConf);
                                  }
                                }
                                else
                                {
                                  gpCentral->notify((string)"Invalid service configuration.  Please provide the Service.  " + strConf);
                                }
                              }
                              else
                              {
                                gpCentral->notify((string)"Invalid JSON formatting in service configuration.  " + strConf);
                              }
                              serviceMap.clear();
                            }
                          }
                          else
                          {
                            gpCentral->alert((string)"Unable to open service configuation for reading.  " + gstrData + (string)SERVICE_CONFIG);
                          }
                          inFile.close();
                        }
                      }
                      else
                      {
                        gpCentral->alert((string)"Unable to locate service configuration.  " + gstrData + (string)SERVICE_CONFIG);
                      }
                    }
                    // }}}
                    if (fork() == 0)
                    {
                      char szIP[INET6_ADDRSTRLEN];
                      SSL *ssl = NULL;
                      sockaddr_storage addr;
                      socklen_t len = sizeof(addr);
                      getpeername(fdData, (sockaddr*)&addr, &len);
                      if (addr.ss_family == AF_INET)
                      {
                        sockaddr_in *s = (sockaddr_in *)&addr;
                        inet_ntop(AF_INET, &s->sin_addr, szIP, sizeof(szIP));
                      }
                      else if (addr.ss_family == AF_INET6)
                      {
                        sockaddr_in6 *s = (sockaddr_in6 *)&addr;
                        inet_ntop(AF_INET6, &s->sin6_addr, szIP, sizeof(szIP));
                      }
                      if (bSecure)
                      {
                        ERR_clear_error();
                        ssl = SSL_new(ctx);
                        SSL_set_fd(ssl, fdData);
                      }
                      if (bStandard || SSL_accept(ssl) == 1)
                      {
                        bool bExit = false;
                        list<string> buffer;
                        list<connection *> queue;
                        pollfd *fds;
                        size_t unPosition, unfdSize;
                        string strBuffer[2];
                        while (!bExit)
                        {
                          fds = new pollfd[(queue.size() * 2) + 1];
                          unfdSize = 0;
                          fds[unfdSize].fd = fdData;
                          fds[unfdSize].events = POLLIN;
                          if (!strBuffer[1].empty())
                          {
                            fds[unfdSize].events |= POLLOUT;
                          }
                          unfdSize++;
                          for (list<connection *>::iterator i = queue.begin(); i != queue.end(); i++)
                          {
                            fds[unfdSize].fd = (*i)->readpipe;
                            fds[unfdSize].events = POLLIN;
                            unfdSize++;
                            fds[unfdSize].fd = -1;
                            if (!(*i)->strBuffer[1].empty())
                            {
                              fds[unfdSize].fd = (*i)->writepipe;
                              fds[unfdSize].events = POLLOUT;
                            }
                            unfdSize++;
                          }
                          if ((nReturn = poll(fds, unfdSize, 2000)) > 0)
                          {
                            for (size_t unfdIndex = 0; unfdIndex < unfdSize; unfdIndex++)
                            {
                              list<list<connection *>::iterator> removeList;
                              // {{{ client socket
                              if (fds[unfdIndex].fd == fdData)
                              {
                                // {{{ read
                                if (fds[unfdIndex].revents & POLLIN)
                                {
                                  if ((bStandard && gpCentral->utility()->fdRead(fdData, strBuffer[0], nReturn)) || (bSecure && gpCentral->utility()->sslRead(ssl, strBuffer[0], nReturn)))
                                  {
                                    bool bHaveRequest = true;
                                    while ((unPosition = strBuffer[0].find("\n")) != string::npos)
                                    {
                                      buffer.push_back(strBuffer[0].substr(0, unPosition));
                                      strBuffer[0].erase(0, (unPosition + 1));
                                    }
                                    while (bHaveRequest && !buffer.empty())
                                    {
                                      list<string> lines;
                                      string strTrim;
                                      bHaveRequest = false;
                                      for (list<string>::iterator i = buffer.begin(); !bHaveRequest && i != buffer.end(); i++)
                                      {
                                        gpCentral->manip()->trim(strTrim, (*i));
                                        if (strTrim == "end")
                                        {
                                          bHaveRequest = true;
                                        }
                                        lines.push_back(*i);
                                      }
                                      if (bHaveRequest)
                                      {
                                        string strError, strRequest = lines.front();
                                        Json *ptRequest;
                                        for (size_t i = 0; i < lines.size(); i++)
                                        {
                                          buffer.pop_front();
                                        }
                                        gpCentral->manip()->trim(strRequest, strRequest);
                                        ptRequest = new Json(strRequest);
                                        if (ptRequest->m.find("Service") != ptRequest->m.end() && !ptRequest->m["Service"]->v.empty())
                                        {
                                          string strCommand;
                                          if (service.find(ptRequest->m["Service"]->v) != service.end())
                                          {
                                            string strThrottle;
                                            if (ptRequest->m.find("Throttle") != ptRequest->m.end() && !ptRequest->m["Throttle"]->v.empty() && atoi(ptRequest->m["Throttle"]->v.c_str()) > 0)
                                            {
                                              strThrottle = ptRequest->m["Throttle"]->v;
                                            }
                                            else if (service[ptRequest->m["Service"]->v].find("Throttle") != service[ptRequest->m["Service"]->v].end() && !service[ptRequest->m["Service"]->v]["Throttle"].empty() && atoi(service[ptRequest->m["Service"]->v]["Throttle"].c_str()) > 0)
                                            {
                                              strThrottle = service[ptRequest->m["Service"]->v]["Throttle"];
                                            }
                                            if (!bConcentrator && !strThrottle.empty() && service.find("portConcentrator") != service.end())
                                            {
                                              string strReqApp, strService = ptRequest->m["Service"]->v, strTimeout;
                                              if (ptRequest->m.find("reqApp") != ptRequest->m.end() && !ptRequest->m["reqApp"]->v.empty())
                                              {
                                                strReqApp = ptRequest->m["reqApp"]->v;
                                              }
                                              if (ptRequest->m.find("Timeout") != ptRequest->m.end() && !ptRequest->m["Timeout"]->v.empty())
                                              {
                                                strTimeout = ptRequest->m["Timeout"]->v;
                                              }
                                              delete ptRequest;
                                              ptRequest = new Json;
                                              ptRequest->insert("Service", "portConcentrator");
                                              ptRequest->insert("SubService", strService);
                                              ptRequest->insert("Throttle", strThrottle);
                                              if (!strReqApp.empty())
                                              {
                                                ptRequest->insert("reqApp", strReqApp);
                                              }
                                              if (!strTimeout.empty())
                                              {
                                                ptRequest->insert("Timeout", strTimeout);
                                              }
                                              ptRequest->json(strRequest);
                                              lines.push_front(strRequest);
                                            }
                                            strCommand = service[ptRequest->m["Service"]->v]["Command"];
                                          }
                                          if (!strCommand.empty())
                                          {
                                            if (bConcentrator || service[ptRequest->m["Service"]->v]["Port"] == "both" || (bStandard && service[ptRequest->m["Service"]->v]["Port"] == "standard") || (bSecure && service[ptRequest->m["Service"]->v]["Port"] == "secure"))
                                            {
                                              char *args[100], *pszArgument;
                                              int readpipe[2] = {-1, -1}, writepipe[2] = {-1, -1};
                                              pid_t childPid;
                                              string strArgument;
                                              stringstream ssCommand, ssMessage;
                                              time_t CStartTime = 0, CEndTime = 0, CTimeout = CHILD_TIMEOUT;
                                              unsigned int unIndex = 0;
                                              time(&CStartTime);
                                              if (ptRequest->m.find("Timeout") != ptRequest->m.end() && !ptRequest->m["Timeout"]->v.empty())
                                              {
                                                bool bNumeric = true;
                                                for (unsigned int i = 0; i < ptRequest->m["Timeout"]->v.size(); i++)
                                                {
                                                  if (!isdigit(ptRequest->m["Timeout"]->v[i]))
                                                  {
                                                    bNumeric = false;
                                                  }
                                                }
                                                if (bNumeric)
                                                {
                                                  CTimeout = atoi(ptRequest->m["Timeout"]->v.c_str());
                                                }
                                              }
                                              ssCommand.str(strCommand);
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
                                                  if ((childPid = fork()) == 0)
                                                  {
                                                    close(PARENT_WRITE);
                                                    close(PARENT_READ);
                                                    dup2(CHILD_READ, 0);
                                                    close(CHILD_READ);
                                                    dup2(CHILD_WRITE, 1);
                                                    close(CHILD_WRITE);
                                                    if (gpSyslog != NULL)
                                                    {
                                                      gpSyslog->connectionStartedCommandLaunched(strCommand, "Accepted an incoming request.", fdData);
                                                    }
                                                    execve(args[0], args, environ);
                                                    _exit(1);
                                                  }
                                                  else if (childPid > 0)
                                                  {
                                                    connection *ptConnection = new connection;
                                                    string strLine;
                                                    close(CHILD_READ);
                                                    close(CHILD_WRITE);
                                                    ptConnection->readpipe = PARENT_READ;
                                                    ptConnection->writepipe = PARENT_WRITE;
                                                    ptConnection->ptRequest = new Json(ptRequest);
                                                    ptConnection->childPid = childPid;
                                                    ptConnection->CStartTime = CStartTime;
                                                    ptConnection->CEndTime = CEndTime;
                                                    ptConnection->CTimeout = CTimeout;
                                                    ptConnection->strCommand = strCommand;
                                                    while (!lines.empty())
                                                    {
                                                      ptConnection->strBuffer[1].append(lines.front() + "\n");
                                                      lines.pop_front();
                                                    }
                                                    queue.push_back(ptConnection);
                                                  }
                                                  else
                                                  {
                                                    strError = (string)"Failed to fork process to system call.  " + (string)strerror(errno);
                                                  }
                                                }
                                                else
                                                {
                                                  strError = (string)"Failed to establish write pipe to system call.  " + (string)strerror(errno);
                                                }
                                              }
                                              else
                                              {
                                                strError = (string)"Failed to establish read pipe to system call.  " + (string)strerror(errno);
                                              }
                                              for (unsigned int i = 0; i < unIndex; i++)
                                              {
                                                delete[] args[i];
                                              }
                                            }
                                            else if (bStandard && service[ptRequest->m["Service"]->v]["Port"] == "standard")
                                            {
                                              strError = "The request arrived on the secure port when the service requires the standard port.";
                                            }
                                            else if (bSecure && service[ptRequest->m["Service"]->v]["Port"] == "secure")
                                            {
                                              strError = "The request arrived on the standard port when the service requires the secure port.";
                                            }
                                            else
                                            {
                                              strError = "Invalid configuration of Port value in service configuration.";
                                            }
                                          }
                                          else
                                          {
                                            strError = "The requested service does not exist.";
                                          }
                                        }
                                        else
                                        {
                                          strError = "Please provide the Service.";
                                        }
                                        if (!strError.empty())
                                        {
                                          string strJson;
                                          ptRequest->insert("Status", "error");
                                          ptRequest->insert("Error", strError);
                                          ptRequest->json(strJson);
                                          strJson += "\nend\n";
                                          strBuffer[1].append(strJson);
                                        }
                                        delete ptRequest;
                                      }
                                      lines.clear();
                                    }
                                    if ((gunMaxBuffer > 0 && strBuffer[0].size() > (gunMaxBuffer * 1024 * 1024)) || (gunMaxLines > 0 && buffer.size() > gunMaxLines))
                                    {
                                      bExit = true;
                                    }
                                  }
                                  else
                                  {
                                    bExit = true;
                                  }
                                }
                                // }}}
                                // {{{ write
                                if (fds[unfdIndex].revents & POLLOUT)
                                {
                                  if ((bStandard && !gpCentral->utility()->fdWrite(fdData, strBuffer[1], nReturn)) || (bSecure && !gpCentral->utility()->sslWrite(ssl, strBuffer[1], nReturn)))
                                  {
                                    bExit = true;
                                  }
                                }
                                // }}}
                              }
                              // }}}
                              // {{{ service pipes
                              for (list<connection *>::iterator i = queue.begin(); i != queue.end(); i++)
                              {
                                bool bDone = false;
                                string strError;
                                // {{{ read
                                if (fds[unfdIndex].fd == (*i)->readpipe && (fds[unfdIndex].revents & (POLLHUP | POLLIN)))
                                {
                                  char szBuffer[65536];
                                  ssize_t nSubReturn;
                                  if ((nSubReturn = read((*i)->readpipe, szBuffer, 65536)) > 0)
                                  {
                                    (*i)->strBuffer[0].append(szBuffer, nSubReturn);
                                  }
                                  else
                                  {
                                    bDone = true;
                                    if (nSubReturn < 0)
                                    {
                                      stringstream ssError;
                                      ssError << "read(" << errno << ") " << strerror(errno);
                                      strError = ssError.str();
                                    }
                                  }
                                }
                                // }}}
                                // {{{ write
                                if (fds[unfdIndex].fd == (*i)->writepipe && (fds[unfdIndex].revents & (POLLHUP | POLLOUT)))
                                {
                                  ssize_t nSubReturn;
                                  if ((nSubReturn = write((*i)->writepipe, (*i)->strBuffer[1].c_str(), (*i)->strBuffer[1].size())) > 0)
                                  {
                                    (*i)->strBuffer[1].erase(0, nSubReturn);
                                  }
                                  else
                                  {
                                    bDone = true;
                                    if (nSubReturn < 0)
                                    {
                                      stringstream ssError;
                                      ssError << "write(" << errno << ") " << strerror(errno);
                                      strError = ssError.str();
                                    }
                                  }
                                }
                                // }}}
                                time(&((*i)->CEndTime));
                                if (((*i)->CEndTime - (*i)->CStartTime) > (*i)->CTimeout)
                                {
                                  bDone = true;
                                  strError = "Request timed out.";
                                  kill((*i)->childPid, SIGTERM);
                                }
                                if (bDone)
                                {
                                  stringstream ssMessage;
                                  close((*i)->readpipe);
                                  close((*i)->writepipe);
                                  strBuffer[1].append((*i)->strBuffer[0] + "end\n");
                                  (*i)->strBuffer[0].clear();
                                  ssMessage << "[Service:" << (*i)->ptRequest->m["Service"]->v << ",Port:" << (string)((bConcentrator)?"concentrator":((bStandard)?"standard":"secure")) << ",IP:" << szIP << ",Duration:" << ((*i)->CEndTime - (*i)->CStartTime) << "]  ";
                                  if (!strError.empty())
                                  {
                                    (*i)->ptRequest->insert("Error", strError);
                                  }
                                  if ((*i)->ptRequest->m.find("Password") != (*i)->ptRequest->m.end())
                                  {
                                    (*i)->ptRequest->insert("Password", "******");
                                  }
                                  ssMessage << (*i)->ptRequest;
                                  gpCentral->log(ssMessage.str());
                                  delete (*i)->ptRequest;
                                  delete *i;
                                  removeList.push_back(i);
                                }
                              }
                              // }}}
                              for (list<list<connection *>::iterator>::iterator i = removeList.begin(); i != removeList.end(); i++)
                              {
                                queue.erase(*i);
                              }
                              removeList.clear();
                            }
                          }
                          else if (nReturn < 0)
                          {
                            bExit = true;
                            gpCentral->log((string)"poll() error: " + strerror(errno));
                          }
                          delete[] fds;
                        }
                        for (list<connection *>::iterator i = queue.begin(); i != queue.end(); i++)
                        {
                          close((*i)->readpipe);
                          close((*i)->writepipe);
                          (*i)->strBuffer[0].clear();
                          (*i)->strBuffer[1].clear();
                          delete (*i)->ptRequest;
                          delete *i;
                          gpCentral->log("Removed unaccounted for request from queue.");
                        }
                        queue.clear();
                      }
                      else
                      {
                        gpCentral->log(gpCentral->utility()->sslstrerror());
                      }
                      if (bSecure)
                      {
                        SSL_shutdown(ssl);
                        SSL_free(ssl);
                      }
                      close(fdData);
                      _exit(1);
                    }
                    else
                    {
                      close(fdData);
                    }
                  }
                  if (!gbShutdown)
                  {
                    gbShutdown = true;
                  }
                  gpCentral->alert((string)"Lost connection to the " + (string)((bConcentrator)?"concentrator":((bStandard)?"standard":"secure")) + (string)" socket!  Exiting...");
                }
                else
                {
                  gpCentral->alert((string)"Could not listen to the " + (string)((bConcentrator)?"concentrator":((bStandard)?"standard":"secure")) + (string)" socket!  Exiting...");
                }
                close(fdSocket);
                gpCentral->log((string)"Closed the " + (string)((bConcentrator)?"concentrator":((bStandard)?"standard":"secure")) + (string)" socket.");
              }
              else
              {
                gpCentral->alert((string)"Could not bind to the " + (string)((bConcentrator)?"concentrator":((bStandard)?"standard":"secure")) + (string)" socket!  Exiting...");
              }
            }
            else
            {
              gpCentral->alert((string)"Could not get address information for the " + (string)((bConcentrator)?"concentrator":((bStandard)?"standard":"secure")) + (string)" socket!  (" + (string)gai_strerror(nReturn) + (string)")  Exiting...");
            }
        }
        else
        {
          gpCentral->alert("Error setting up SSL context keys.");
        }
        if (bSecure)
        {
          SSL_CTX_free(ctx);
        }
        if (bChild)
        {
          _exit(1);
        }
      }
      else
      {
        gpCentral->alert((string)"fork() error [concentrator->standard]: " + strerror(errno));
      }
      // {{{ check pid file
      if (gpCentral->file()->fileExist((gstrData + PID).c_str()))
      {
        gpCentral->file()->remove((gstrData + PID).c_str());
      }
      // }}}
    }
  }
  // }}}
  // {{{ usage statement
  else
  {
    mUSAGE(argv[0]);
  }
  // }}}
  if (gpSyslog != NULL)
  {
    delete gpSyslog;
  }
  gpCentral->utility()->sslDeinit();
  delete gpCentral;

  return 0;
}
// }}}
// {{{ sighandle()
void sighandle(const int nSignal)
{
  string strError, strSignal;
  stringstream ssSignal;

  sethandles(sigdummy);
  gbShutdown = true;
  if (nSignal != SIGINT && nSignal != SIGTERM)
  {
    ssSignal << nSignal;
    gpCentral->alert((string)"The program's signal handling caught a " + (string)sigstring(strSignal, nSignal) + (string)"(" + ssSignal.str() + (string)")!  Exiting...");
  }
  exit(1);
}
// }}}
