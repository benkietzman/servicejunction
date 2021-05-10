<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2011-08-15
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

// {{{ fetchPage()
function fetchPage(&$strUrl, $strCookies, $strPost, &$strHeader, &$strContent, &$strError, $bMobile = false, $bFailOnError = true)
{
  $bResult = false;
  $bRedirect = true;

  $ch = curl_init();
  if ($bMobile)
  {
    curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16');
  }
  else
  {
    curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Ubuntu; X11; Linux i686; rv:8.0) Gecko/20100101 Firefox/8.0');
  }
  curl_setopt($ch, CURLOPT_FOLLOWLOCATION, false);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($ch, CURLOPT_HEADER, true);
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
  if ($strCookies != '')
  {
    curl_setopt($ch, CURLOPT_COOKIEJAR, $strCookies);
    curl_setopt($ch, CURLOPT_COOKIEFILE, $strCookies);
  }
  if ($strPost != '')
  {
    curl_setopt($ch, CURLOPT_POST, true);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $strPost);
  }
  while ($bRedirect)
  {
    $bRedirect = false;
    curl_setopt($ch, CURLOPT_URL, $strUrl);
    if ($bFailOnError)
    {
      curl_setopt($ch, CURLOPT_FAILONERROR, true);
    }
    if (($strData = curl_exec($ch)) !== false)
    {
      if (preg_match('#Location: (.*)#', $strData, $result))
      {
        $bRedirect = true;
        $strUrl = trim($result[1]);
      }
      else
      {
        $nPosition = array(0, 0, 0);
        if (($nPosition[0] = strpos($strData, "\r\n\r\n")) !== false)
        {
          $strHeader = trim(substr($strData, 0, $nPosition[0]));
          $strContent = substr($strData, $nPosition[0] + 4, strlen($strData) - ($nPosition[0] + 4));
          if (($nPosition[1] = strpos($strHeader, "\r\n")) !== false)
          {
            if (($nPosition[2] = strpos($strHeader, ' ')) !== false)
            {
              if (($strStatus = trim(substr($strHeader, $nPosition[2] + 1, $nPosition[1] - ($nPosition[2] + 1)))) == '200 OK' || $strStatus == '200')
              {
                $bResult = true;
              }
              else
              {
                $strError = $strStatus;
              }
            }
            else
            {
              $strError = 'Failed to parse error code.';
            }
          }
          else
          {
            $strError = 'Failed to parse HTML header.';
          }
        }
        else
        {
          $strError = 'Malformed HTML document.';
        }
        unset($nPosition);
      }
    }
    else
    {
      $strError = curl_error($ch);
    }
  }
  curl_close($ch);

  return $bResult;
}
// }}}
// {{{ loadRequest()
function loadRequest(&$requestArray, &$request)
{
  $unIndex = 0;
  $request = array();
  $strError = null;
  if (($handle = fopen('php://stdin', 'r')) !== false)
  {
    $bFirst = true;
    $bFoundEnd = false;
    while (!$bFoundEnd && !feof($handle))
    {
      $strLine = trim(fgets($handle));
      if ($strLine == 'end')
      {
        $bFoundEnd = true;
      }
      else if ($strLine != '')
      {
        if ($bFirst)
        {
          $bFirst = false;
          if (($requestArray = json_decode($strLine, true)) == null)
          {
            $strError = 'Invalid JSON formatting.';
          }
        }
        $request[$unIndex++] = $strLine;
      }
    }
    if (!$bFoundEnd)
    {
      $strError = 'Failed to receive end of request.';
    }
    fclose($handle);
  }
  else
  {
    $strError = 'Failed to open stdin for reading.';
  }
  if ($requestArray == null)
  {
    $requestArray = array();
  }
  if ($strError != null)
  {
    $requestArray['Error'] = $strError;
  }
}
// }}}
?>
