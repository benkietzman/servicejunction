// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2011-12-09
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

#include "functions"

// {{{ dump()
void dump(const char *text, FILE *stream, unsigned char *ptr, size_t size)
{
  size_t i;
  size_t c;
  unsigned int width = 0x40;

  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n", text, (long)size, (long)size);
  for (i = 0; i < size; i += width)
  {
    fprintf(stream, "%4.4lx: ", (long)i);
    for (c = 0; (c < width) && (i + c < size); c++)
    {
      /* check for 0D0A; if found, skip past and start a new line of output */
      if ((i + c + 1 < size) && ptr[i + c] == 0x0D && ptr[i + c + 1] == 0x0A)
      {
        i += (c + 2 - width);
        break;
      }
      fprintf(stream, "%c", (ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80)?ptr[i + c]:'.');
      /* check again for 0D0A, to avoid an extra \n if it's at width */
      if ((i + c + 2 < size) && ptr[i + c + 1] == 0x0D && ptr[i + c + 2] == 0x0A)
      {
        i += (c + 3 - width);
        break;
      }
    }
    fputc('\n', stream);
  }
  fflush(stream);
}
// }}}
// {{{ fetchPage()
bool fetchPage(string &strUrl, const string strType, map<string, string> auth, const string strCookies, const string strPost, const string strPut, const string strProxy, string &strHeader, string &strContent, string &strError, const string strUserAgent, const bool bMobile, const bool bFailOnError, const bool bDebug, const string strCustomRequest)
{
  bool bHeader = false, bResult = false, bRedirect = true;
  char szError[CURL_ERROR_SIZE] = "\0";
  curl_slist *headers = NULL;
  CURL *ch;
  CURLcode response;
  memoryStruct chunk;
  unsigned int unAttempt[2] = {0, 0};

  curl_global_init(CURL_GLOBAL_ALL);
  ch = curl_easy_init();
  if (bDebug)
  {
    curl_easy_setopt(ch, CURLOPT_DEBUGFUNCTION, mytrace);
    curl_easy_setopt(ch, CURLOPT_VERBOSE, true);
  }
  chunk.memory = NULL;
  chunk.size = 0;
  curl_easy_setopt(ch, CURLOPT_ERRORBUFFER, szError);
  if (!strUserAgent.empty())
  {
    if (strUserAgent != "none")
    {
      curl_easy_setopt(ch, CURLOPT_USERAGENT, strUserAgent.c_str());
    }
  }
  else if (bMobile)
  {
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16");
  }
  else
  {
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "Mozilla/5.0 (Ubuntu; X11; Linux i686; rv:8.0) Gecko/20100101 Firefox/8.0");
  }
  curl_easy_setopt(ch, CURLOPT_HEADER, true);
  if (!strHeader.empty())
  {
    string strLine;
    stringstream ssHeader(strHeader);
    bHeader = true;
    while (getline(ssHeader, strLine))
    {
      headers = curl_slist_append(headers, strLine.c_str());
    }
  }
  if (auth.find("Type") != auth.end())
  {
    if (auth["Type"] == "basic")
    {
      curl_easy_setopt(ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
      if (auth.find("User") != auth.end())
      {
        curl_easy_setopt(ch, CURLOPT_USERNAME, auth["User"].c_str());
      }
      if (auth.find("Password") != auth.end())
      {
        curl_easy_setopt(ch, CURLOPT_PASSWORD, auth["Password"].c_str());
      }
    }
  }
  curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, true);
  if (!strPost.empty() && strPost != "null" && strPost != "\"\"")
  {
    if (strType == "json" || strType == "plain")
    {
      stringstream ssLength;
      bHeader = true;
      curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
      if (strType == "json")
      {
        headers = curl_slist_append(headers, "Content-Type: application/json");
      }
      else
      {
        headers = curl_slist_append(headers, "Content-Type: text/plain");
      }
      ssLength << "Content-Length: " << strPost.size();
      headers = curl_slist_append(headers, ssLength.str().c_str());
    }
    else
    {
      curl_easy_setopt(ch, CURLOPT_POST, true);
    }
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, strPost.c_str());
  }
  else if (!strPut.empty() && strPut != "null" && strPut != "\"\"")
  {
    if (strType == "json")
    {
      stringstream ssLength;
      bHeader = true;
      curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "PUT");
      headers = curl_slist_append(headers, "Content-Type: application/json");
      ssLength << "Content-Length: " << strPut.size();
      headers = curl_slist_append(headers, ssLength.str().c_str());
    }
    else
    {
      curl_easy_setopt(ch, CURLOPT_PUT, true);
    }
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, strPut.c_str());
  }
  if (!strCustomRequest.empty())
  {
    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, strCustomRequest.c_str());
  }
  if (bHeader)
  {
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
  }
  curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
  curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *)&chunk);
  if (!strCookies.empty())
  {
    curl_easy_setopt(ch, CURLOPT_COOKIEJAR, strCookies.c_str());
    curl_easy_setopt(ch, CURLOPT_COOKIEFILE, strCookies.c_str());
  }
  if (!strProxy.empty())
  {
    string strProxyUrl, strProxyUserPassword;
    stringstream ssProxy(strProxy);
    getline(ssProxy, strProxyUrl);
    curl_easy_setopt(ch, CURLOPT_PROXY, strProxyUrl.c_str());
    getline(ssProxy, strProxyUserPassword);
    if (!strProxyUserPassword.empty())
    {
      curl_easy_setopt(ch, CURLOPT_PROXYUSERPWD, strProxyUserPassword.c_str());
    }
  }
  while (bRedirect && unAttempt[0]++ < 5)
  {
    bRedirect = false;
    curl_easy_setopt(ch, CURLOPT_URL, strUrl.c_str());
    if (bFailOnError)
    {
      curl_easy_setopt(ch, CURLOPT_FAILONERROR, true);
    }
    if ((response = curl_easy_perform(ch)) == 0)
    {
      if (chunk.memory)
      {
        bool bContinue = true;
        size_t nPosition[3] = {0, 0, 0};
        string strValue;
        stringstream ssData;
        StringManip manip;
        ssData.str(chunk.memory);
        if (!strProxy.empty())
        {
          if ((nPosition[0] = ssData.str().find("\r\n\r\n")) != string::npos)
          {
            string strProxyHeader;
            strProxyHeader = manip.trim(strValue, ssData.str().substr(0, nPosition[0]));
            if ((nPosition[1] = strProxyHeader.find("\r\n")) == string::npos)
            {
              nPosition[1] = strProxyHeader.size();
            }
            if ((nPosition[2] = strProxyHeader.find(" ")) != string::npos)
            {
              string strStatus = manip.trim(strValue, strProxyHeader.substr(nPosition[2] + 1, nPosition[1] - (nPosition[2] + 1)));
              if (strProxyHeader == "200 Connection established" || strStatus == "200 Connection established")
              {
                string strTemp = ssData.str();
                strTemp.erase(0, nPosition[0] + 4);
                ssData.str(strTemp);
              }
              else
              {
                strError = strStatus;
              }
            }
            else
            {
              strError = "Failed to parse error code.";
            }
          }
          else
          {
            strError = "Malformed HTML document.";
          }
        }
        unAttempt[1] = 0;
        while (bContinue && unAttempt[1]++ < 5)
        {
          bContinue = false;
          if ((nPosition[0] = ssData.str().find("\r\n\r\n")) != string::npos)
          {
            string strLowerHeader;
            manip.toLower(strLowerHeader, (strHeader = manip.trim(strValue, ssData.str().substr(0, nPosition[0]))));
            strContent = ssData.str().substr(nPosition[0] + 4, ssData.str().size() - (nPosition[0] + 4));
            if ((nPosition[1] = strLowerHeader.find("location: ")) != string::npos && (nPosition[2] = strLowerHeader.find("\n", nPosition[1])) != string::npos)
            {
              bRedirect = true;
              strUrl = manip.trim(strValue, strHeader.substr(nPosition[1] + 10, nPosition[2] - (nPosition[1] + 10)));
            }
            else
            {
              if ((nPosition[1] = strHeader.find("\r\n")) == string::npos)
              {
                nPosition[1] = strHeader.size();
              }
              if ((nPosition[2] = strHeader.find(" ")) != string::npos)
              {
                string strLowerStatus, strStatus = manip.trim(strValue, strHeader.substr(nPosition[2] + 1, nPosition[1] - (nPosition[2] + 1)));
                manip.toLower(strLowerStatus, strStatus);
                if (strLowerHeader == "200 ok" || strLowerHeader == "201 created" || strLowerHeader == "204 no content" || strLowerStatus == "200 ok" || strLowerStatus == "201 created" || strLowerStatus == "204 no content" || strStatus == "200" || strStatus == "201" || strStatus == "204")
                {
                  bResult = true;
                }
                else if (strLowerHeader == "100 continue" || strLowerStatus == "100 continue")
                {
                  string strTemp = ssData.str();
                  bContinue = true;
                  strTemp.erase(0, nPosition[0] + 4);
                  ssData.str(strTemp);
                }
                else
                {
                  strError = strStatus;
                }
              }
              else
              {
                strError = "Failed to parse error code.";
              }
            }
          }
          else
          {
            strError = "Malformed HTML document.";
          }
        }
      }
      else
      {
        strError = "Failed to allocate memory for results.";
      }
    }
    else
    {
      strError = szError;
    }
    if (chunk.memory)
    {
      free(chunk.memory);
    }
    chunk.memory = NULL;
    chunk.size = 0;
  }
  if (unAttempt[0] >= 5)
  {
    strError = "Encountered more than five header redirects.";
  }
  else if (unAttempt[1] >= 5)
  {
    strError = "Encountered more than five HTTP continue statements.";
  }
  if (bHeader)
  {
    curl_slist_free_all(headers);
  }
  curl_easy_cleanup(ch);
  curl_global_cleanup();

  return bResult;
}
// }}}
// {{{ loadRequest()
void loadRequest(list<string> &request)
{
  bool bExit = false;
  char szBuffer[65536];
  int nReturn;
  size_t unPosition;
  string strBuffer, strError, strLine;
  StringManip manip;

  while (!bExit)
  {
    pollfd fds[1];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    if ((nReturn = poll(fds, 1, 250)) > 0)
    {
      if (fds[0].revents & (POLLIN | POLLHUP))
      {
        if ((nReturn = read(0, szBuffer, 65536)) > 0)
        {
          strBuffer.append(szBuffer, nReturn);
          while ((unPosition = strBuffer.find("\n")) != string::npos)
          {
            manip.trim(strLine, strBuffer.substr(0, unPosition));
            strBuffer.erase(0, (unPosition + 1));
            if (strLine == "end")
            {
              bExit = true;
            }
            else
            {
              request.push_back(strLine);
            }
          }
        }
        else
        {
          bExit = true;
        }
      }
    }
    else if (nReturn < 0)
    {
      bExit = true;
    }
  }
}
void loadRequest(list<Json *> &request)
{
  list<string> req;

  loadRequest(req);
  while (!req.empty())
  {
    request.push_back(new Json(req.front()));
    req.pop_front();
  }
}
void loadRequest(map<string, string> &requestArray, list<string> &request)
{
  loadRequest(request);
  if (!request.empty())
  {
    Json *ptJson = new Json(request.front());
    ptJson->flatten(requestArray, false, false);
    delete ptJson;
  }
}
// }}}
// {{{ myrealloc()
void *myrealloc(void *ptr, size_t size)
{
  if (ptr)
  {
    return realloc(ptr, size);
  }
  else
  {
    return malloc(size);
  }
}
// }}}
// {{{ mytrace()
int mytrace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
  const char *text;
  (void)handle; /* prevent compiler warning */

  switch (type)
  {
    case CURLINFO_TEXT: fprintf(stderr, "== Info: %s", data);
    default: /* in case a new one is introduced to shock us */ return 0;
    case CURLINFO_HEADER_OUT: text = "=> Send header"; break;
    case CURLINFO_DATA_OUT: text = "=> Send data"; break;
    case CURLINFO_SSL_DATA_OUT: text = "=> Send SSL data"; break;
    case CURLINFO_HEADER_IN: text = "<= Recv header"; break;
    case CURLINFO_DATA_IN: text = "<= Recv data"; break;
    case CURLINFO_SSL_DATA_IN: text = "<= Recv SSL data"; break;
  }
  dump(text, stderr, (unsigned char *)data, size);

  return 0;
}
// }}}
// {{{ writeMemoryCallback()
size_t writeMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  memoryStruct *mem = (memoryStruct *)data;

  mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory)
  {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }

  return realsize;
}
// }}}
