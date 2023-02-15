<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2011-09-20
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

require('functions.php');
$bProcessed = false;
$requestArray = null;
$request = null;
$response = null;
$extras = null;
$strError = null;
loadRequest($requestArray, $request);
if (isset($requestArray['Function']) && $requestArray['Function'] != '' && ($requestArray['Function'] == 'bitcoin' || $requestArray['Function'] == 'commodity' || $requestArray['Function'] == 'federal' || $requestArray['Function'] == 'fidelity' || $requestArray['Function'] == 'stock'))
{
  // {{{  bitcoin
  if ($requestArray['Function'] == 'bitcoin')
  {
    if (!isset($requestArray['Currency']) || $requestArray['Currency'] == '')
    {
      $requestArray['Currency'] = 'usd';
    }
    if (isset($requestArray['Symbol']) && $requestArray['Symbol'] != '')
    {
      if ($requestArray['Symbol'] == 'bitpay')
      {
        if (($strData = @file_get_contents('https://bitpay.com/api/rates')) !== false)
        {
          if (($data = json_decode($strData, true)) !== false && is_array($data))
          {
            $nSize = sizeof($data);
            for ($i = 0; !$bProcessed && $i < $nSize; $i++)
            {
              if (isset($data[$i]['code']) && $data[$i]['code'] == strtoupper($requestArray['Currency']))
              {
                $bProcessed = true;
                $response = $data[$i];
                $response['Time'] = time();
                $response['Price'] = $data[$i]['rate'];
              }
            }
            unset($data);
          }
          else
          {
            $strError = 'Failed to parse BitCoin Prices JSON results.';
          }
        }
        else
        {
          $strError = 'Failed to connect to the BitCoin Prices URL.';
        }
      }
      else
      {
        if (($strData = @file_get_contents('https://bitcoin-prices.herokuapp.com/api/v1/'.$requestArray['Symbol'].'/'.$requestArray['Currency'].'/24h')) !== false)
        {
          if (($data = json_decode($strData, true)) !== false && isset($data['prices']) && is_array($data['prices']) && ($nSize = sizeof($data['prices'])) > 0)
          {
            $response = array();
            $bProcessed = true;
            $response['Time'] = $data['prices'][$nSize - 1]['t'];
            $response['Price'] = $data['prices'][$nSize - 1]['p'];
            unset($data);
          }
        }
        if (!$bProcessed)
        {
          if (($strData = @file_get_contents('http://api.bitcoincharts.com/v1/markets.json')) !== false)
          {
            if (($data = json_decode($strData, true)) !== false && is_array($data))
            {
              $nSize = sizeof($data);
              for ($i = 0; !$bProcessed && $i < $nSize; $i++)
              {
                if (isset($data[$i]['symbol']) && $data[$i]['symbol'] == $requestArray['Symbol'].strtoupper($requestArray['Currency']))
                {
                  $bProcessed = true;
                  $response = $data[$i];
                  $response['Time'] = $data[$i]['latest_trade'];
                  $response['Price'] = $data[$i]['avg'];
                }
              }
              unset($data);
            }
            else
            {
              $strError = 'Failed to parse BitCoin Prices JSON results.';
            }
          }
          else
          {
            $strError = 'Failed to connect to the BitCoin Prices URL.';
          }
        }
      }
    }
    else
    {
      $strError = 'Please provide the Symbol.';
    }
  }
  // }}}
  // {{{ commodity
  else if ($requestArray['Function'] == 'commodity')
  {
    if (isset($requestArray['Symbol']) && $requestArray['Symbol'] != '' && ($requestArray['Symbol'] == 'gold' || $requestArray['Symbol'] == 'silver' || $requestArray['Symbol'] == 'platinum' || $requestArray['Symbol'] == 'palladium'))
    {
      $strHeader = null;
      $strContent = null;
      $strUrl = 'http://www.kitco.com/market/';
      if (fetchPage($strUrl, null, null, $strHeader, $strContent, $strError))
      {
        $nPosition = array(0, 0);
        if (($nPosition[0] = strpos($strContent, '</span>'.strtoupper($requestArray['Symbol']).'</a>')) !== false)
        {
          $bProcessed = true;
          $response = array();
          $nPosition[0] = strpos($strContent, '<td', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '>', $nPosition[0]);
          $nPosition[1] = strpos($strContent, '<', $nPosition[0]);
          $response['Date'] = substr($strContent, $nPosition[0] + 1, $nPosition[1] - ($nPosition[0] + 1));
          $nPosition[0] = strpos($strContent, '<td', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '>', $nPosition[0]);
          $nPosition[1] = strpos($strContent, '<', $nPosition[0]);
          $response['Time'] = substr($strContent, $nPosition[0] + 1, $nPosition[1] - ($nPosition[0] + 1));
          $nPosition[0] = strpos($strContent, '<td', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '>', $nPosition[0]);
          $nPosition[1] = strpos($strContent, '<', $nPosition[0]);
          $response['Bid'] = substr($strContent, $nPosition[0] + 1, $nPosition[1] - ($nPosition[0] + 1));
          $nPosition[0] = strpos($strContent, '<td', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '>', $nPosition[0]);
          $nPosition[1] = strpos($strContent, '<', $nPosition[0]);
          $response['Ask'] = substr($strContent, $nPosition[0] + 1, $nPosition[1] - ($nPosition[0] + 1));
          $nPosition[0] = strpos($strContent, '<td', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '<p', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '>', $nPosition[0]);
          $nPosition[1] = strpos($strContent, '<', $nPosition[0]);
          $response['Change'] = substr($strContent, $nPosition[0] + 1, $nPosition[1] - ($nPosition[0] + 1));
          $nPosition[0] = strpos($strContent, '<td', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '<p', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '>', $nPosition[0]);
          $nPosition[1] = strpos($strContent, '<', $nPosition[0]);
          $response['PercentageChange'] = substr($strContent, $nPosition[0] + 1, $nPosition[1] - ($nPosition[0] + 1));
          $nPosition[0] = strpos($strContent, '<td', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '>', $nPosition[0]);
          $nPosition[1] = strpos($strContent, '<', $nPosition[0]);
          $response['Low'] = substr($strContent, $nPosition[0] + 1, $nPosition[1] - ($nPosition[0] + 1));
          $nPosition[0] = strpos($strContent, '<td', $nPosition[0]);
          $nPosition[0] = strpos($strContent, '>', $nPosition[0]);
          $nPosition[1] = strpos($strContent, '<', $nPosition[0]);
          $response['High'] = substr($strContent, $nPosition[0] + 1, $nPosition[1] - ($nPosition[0] + 1));
        }
        else
        {
          $strError = 'Failed to locate commodity.';
        }
        unset($nPosition);
      }
    }
    else
    {
      $strError = 'Please provide a valid Symbol:  gold, silver, platinum, or palladium.';
    }
  }
  // }}}
  // {{{ federal
  else if ($requestArray['Function'] == 'federal')
  {
    if (isset($requestArray['Symbol']) && $requestArray['Symbol'] != '' && ($requestArray['Symbol'] == 'debt'))
    {
      $strHeader = null;
      $strContent = null;
      $strUrl = 'https://treasurydirect.gov/NP_WS/debt/current?format=json';
      if (fetchPage($strUrl, null, null, $strHeader, $strContent, $strError))
      {
        $data = json_decode($strContent, true);
        if (isset($data['totalDebt']))
        {
          $bProcessed = true;
          $response = array();
          $response['Value'] = $data['totalDebt'] / 1000000000;
        }
        else
        {
          $strError = 'Failed to locate federal debt.';
        }
        unset($nPosition);
      }
    }
    else
    {
      $strError = 'Please provide a valid Symbol:  debt.';
    }
  }
  // }}}
  // {{{ fidelity
  else if ($requestArray['Function'] == 'fidelity')
  {
    if (isset($requestArray['User']) && $requestArray['User'] != '')
    {
      if (isset($requestArray['Password']) && $requestArray['Password'] != '')
      {
        $bRedirect = true;
        $strCookies = '/tmp/servicejunction_cookies.'.posix_getpid();
        $strHeader = null;
        $strContent = null;
        if (file_exists($strCookies))
        {
          unlink($strCookies);
        }
        $strUrl = 'http://www.fidelity.mobi/';
        while ($bRedirect && fetchPage($strUrl, $strCookies, null, $strHeader, $strContent, $strError, true))
        {
          $nPosition = array(0, 0, 0, 0, 0, 0, 0, 0, 0);
          if (($nPosition[0] = strpos($strContent, '<meta http-equiv="Refresh" content="0;URL=', 0)) !== false)
          {
            if (($nPosition[0] = strpos($strContent, 'URL=', $nPosition[0])) !== false && ($nPosition[1] = strpos($strContent, '"', $nPosition[0])) !== false)
            {
              $strUrl = substr($strContent, $nPosition[0] + 4, $nPosition[1] - ($nPosition[0] - 4));
            }
            else
            {
              $strError = 'Failed to parse redirect link.';
            }
          }
          else
          {
            $bFoundLogin = false;
            $bRedirect = false;
            while (!$bFoundLogin && ($nPosition[0] = strpos($strContent, 'href="', $nPosition[0] + 6)) !== false)
            {
              if (($nPosition[1] = strpos($strContent, '>', $nPosition[0])) !== false && ($nPosition[2] = strpos($strContent, '<', $nPosition[1] + 1)) !== false && trim(substr($strContent, $nPosition[1] + 1, $nPosition[2] - ($nPosition[1] + 1))) == 'Log In')
              {
                $bFoundLogin = true;
              }
            }
            if ($bFoundLogin && ($nPosition[1] = strpos($strContent, '"', $nPosition[0] + 6)) !== false && ($nPosition[2] = strpos($strUrl, '//')) !== false && ($nPosition[3] = strpos($strUrl, '/', $nPosition[2] + 2)) !== false)
            {
              $strUrl = substr($strUrl, 0, $nPosition[3]).substr($strContent, $nPosition[0] + 6, $nPosition[1] - ($nPosition[0] + 6));
              if (fetchPage($strUrl, $strCookies, null, $strHeader, $strContent, $strError, true))
              {
                if (($nPosition[0] = strpos($strUrl, '//')) !== false && ($nPosition[1] = strpos($strUrl, '/', $nPosition[0] + 2)) !== false && ($nPosition[2] = strpos($strContent, 'action="')) !== false && ($nPosition[3] = strpos($strContent, '"', $nPosition[2] + 8)) !== false)
                {
                  $strUrl = substr($strUrl, 0, $nPosition[1]).substr($strContent, $nPosition[2] + 8, $nPosition[3] - ($nPosition[2] + 8));
                  $strPost  = 'userid='.urlencode($requestArray['User']);
                  $strPost .= '&pin='.urlencode($requestArray['Password']);
                  if (fetchPage($strUrl, $strCookies, $strPost, $strHeader, $strContent, $strError, true))
                  {
                    if (($nPosition[0] = strpos($strContent, 'Portfolio Total')) !== false && ($nPosition[0] = strpos($strContent, '$', $nPosition[0])) && ($nPosition[1] = strpos($strContent, '<', $nPosition[0] + 1)))
                    {
                      $bProcessed = true;
                      $requestArray['Balance'] = str_replace(',', '', trim(substr($strContent, $nPosition[0] + 1, $nPosition[1] - ($nPosition[0] + 1))));
                      $unIndex = 0;
                      $extras = array();
                      $nPosition[3] = 0;
                      while (($nPosition[2] = strpos($strContent, 'width:60%', $nPosition[3])) !== false)
                      {
                        if (($nPosition[0] = strpos($strUrl, '//')) !== false && ($nPosition[1] = strpos($strUrl, '/', $nPosition[0] + 2)) !== false && ($nPosition[2] = strpos($strContent, 'href="', $nPosition[2])) !== false && ($nPosition[3] = strpos($strContent, '"', $nPosition[2] + 6)) !== false && ($nPosition[4] = strpos($strContent, '>', $nPosition[3] + 1)) !== false && ($nPosition[5] = strpos($strContent, '<', $nPosition[4])) !== false && ($nPosition[6] = strpos($strContent, 'width:40%', $nPosition[5] + 1)) !== false && ($nPosition[7] = strpos($strContent, '$', $nPosition[6] + 9)) !== false && ($nPosition[8] = strpos($strContent, '<', $nPosition[7] + 1)) !== false && ($nBalance = str_replace(',', '', substr($strContent, $nPosition[7] + 1, $nPosition[8] - ($nPosition[7] + 1)))) > 0)
                        {
                          $strLink = substr($strUrl, 0, $nPosition[1]).substr($strContent, $nPosition[2] + 6, $nPosition[3] - ($nPosition[2] + 6));
                          $strAccount = html_entity_decode(trim(substr($strContent, $nPosition[4] + 1, $nPosition[5] - ($nPosition[4] + 1))));
                          if (!isset($requestArray['Account']) || $requestArray['Account'] == '' || $requestArray['Account'] == $strAccount)
                          {
                            $extras[$unIndex] = array();
                            if (!isset($requestArray['Symbol']) || $requestArray['Symbol'] == '')
                            {
                              $extras[$unIndex]['Account'] = $strAccount;
                              $extras[$unIndex]['Balance'] = $nBalance;
                            }
                            if (fetchPage($strLink, $strCookies, null, $strHeader, $strSubContent, $strError, true))
                            {
                              $nSubIndex = 0;
                              $detail = array();
                              $nPosition[1] = 0;
                              while (($nPosition[0] = strpos($strSubContent, '<td class="blueborder" align="left">&nbsp;', $nPosition[1])) !== false)
                              {
                                $nDiff = 0;
                                if ($strSubContent[$nPosition[0]+42] == '<' && ($nPosition[4] = strpos($strSubContent, '>', $nPosition[0] + 42)) !== false)
                                {
                                  $nDiff = $nPosition[4] - ($nPosition[0] + 41);
                                }
                                if (($nPosition[1] = strpos($strSubContent, '<', $nPosition[0] + $nDiff + 42)) !== false && ($nPosition[4] = strpos($strSubContent, '$', $nPosition[1] + 1)) !== false && ($nPosition[5] = strpos($strSubContent, '<', $nPosition[4] + 1)) !== false)
                                {
                                  $strSymbol = trim(html_entity_decode(substr($strSubContent, $nPosition[0] + $nDiff + 42, $nPosition[1] - ($nPosition[0] + $nDiff + 42))));
                                  $nBalance = str_replace(',', '', trim(substr($strSubContent, $nPosition[4] + 1, $nPosition[5] - ($nPosition[4] + 1))));
                                  if (!isset($requestArray['Symbol']) || $requestArray['Symbol'] == '')
                                  {
                                    $detail[$nSubIndex] = array();
                                    $detail[$nSubIndex]['Symbol'] = $strSymbol;
                                    $detail[$nSubIndex]['Balance'] = $nBalance;
                                    $nSubIndex++;
                                  }
                                  else if ($requestArray['Symbol'] == $strSymbol)
                                  {
                                    $extras[$unIndex]['Symbol'] = $strSymbol;
                                    $extras[$unIndex]['Balance'] = $nBalance;
                                  }
                                }
                                $nPosition[1] = $nPosition[5];
                              }
                              $nPosition[1] = 0;
                              while (($nPosition[0] = strpos($strSubContent, '<div class="small">', $nPosition[1])) !== false)
                              {
                                if (($nPosition[1] = strpos($strSubContent, '<', $nPosition[0] + 19)) !== false && ($nPosition[4] = strpos($strSubContent, '$', $nPosition[1])) !== false && ($nPosition[5] = strpos($strSubContent, '<', $nPosition[4] + 1)) !== false)
                                {
                                  $strSymbol = trim(html_entity_decode(substr($strSubContent, $nPosition[0] + 19, $nPosition[1] - ($nPosition[0] + 19))));
                                  $nBalance = str_replace(',', '', trim(substr($strSubContent, $nPosition[4] + 1, $nPosition[5] - ($nPosition[4] + 1))));
                                  if (!isset($requestArray['Symbol']) || $requestArray['Symbol'] == '' || $requestArray['Symbol'] == $strSymbol)
                                  {
                                    $detaul[$nSubIndex] = array();
                                    $detail[$nSubIndex]['Symbol'] = $strSymbol;
                                    $detail[$nSubIndex]['Balance'] = $nBalance;
                                    $nSubIndex++;
                                  }
                                  else if ($requestArray['Symbol'] == $strSymbol)
                                  {
                                    $extras[$unIndex]['Symbol'] = $strSymbol;
                                    $extras[$unIndex]['Balance'] = $nBalance;
                                  }
                                }
                                $nPosition[1] = $nPosition[5];
                              }
                              if (!empty($detail))
                              {
                                $extras[$unIndex]['Details'] = $detail;
                              }
                              unset($detail);
                            }
                            else
                            {
                              $strError = 'Failed to fetch '.$strLink.' page.  '.$strError;
                            }
                            $unIndex++;
                          }
                        }
                      }
                    }
                    else
                    {
                      $strError = 'Failed to locate portfolio total.';
                    }
                  }
                  else
                  {
                    $strError = 'Failed to fetch '.$strUrl.' page.  '.$strError;
                  }
                }
                else
                {
                  $strError = 'Failed to locate login form.';
                }
              }
              else
              {
                $strError = 'Failed to fetch '.$strUrl.' page.  '.$strError;
              }
            }
            else
            {
              $strError = 'Failed to locate login link.';
            }
          }
          unset($nPosition);
        }
        if ($bRedirect)
        {
          $strError = 'Failed to fetch '.$strUrl.' page.  '.$strError;
        }
        unlink($strCookies);
      }
      else
      {
        $strError = 'Please provide a valid Password.';
      }
    }
    else
    {
      $strError = 'Please provide a valid User.';
    }
  }
  // }}}
  // {{{  stock
  else if ($requestArray['Function'] == 'stock')
  {
    if (isset($requestArray['Symbol']) && $requestArray['Symbol'] != '')
    {
      if (isset($requestArray['Key']) && $requestArray['Key'] != '')
      {
        if (($strData = @file_get_contents('https://alphavantage.co/query?function=GLOBAL_QUOTE&symbol='.$requestArray['Symbol'].'&apikey='.$requestArray['Key'])) !== false)
        {
          if (($data = json_decode($strData, true)) !== false)
          {
            if (isset($data['Global Quote']) && is_array($data['Global Quote']))
            {
              if (isset($data['Global Quote']['01. symbol']) && $data['Global Quote']['01. symbol'] != '')
              {
                $response = array();
                $bProcessed = true;
                $response['Symbol'] = $data['Global Quote']['01. symbol'];
                $response['Last'] = str_replace(',', '', $data['Global Quote']['05. price']);
                $response['Date'] = $data['Global Quote']['07. latest trading day'];
                $response['Change'] = $data['Global Quote']['09. change'];
                $response['PercentageChange'] = $data['Global Quote']['10. change percent'];
              }
              else
              {
                $strError = 'Please provide a valid Symbol.';
              }
            }
            else if (isset($data['Error Message']) && $data['Error Message'] != '')
            {
              $strError = $data['Error Message'];
            }
            else
            {
              $strError = 'Encountered an unknown error.  '.json_encode($data);
            }
            unset($data);
          }
        }
        else
        {
          $strError = 'Failed to connect to the Alphavantage URL.';
        }
      }
      else if (($strData = @file_get_contents('https://stooq.pl/q/l/?s='.$requestArray['Symbol'].'.US&f=sd2t2ohlcv&h&e=json')) !== false)
      {
        if (($data = json_decode($strData, true)) !== false)
        {
          if (isset($data['symbols']) && is_array($data['symbols']))
          {
            $bFound = false;
            for ($i = 0; !$bFound && $i < sizeof($data['symbols']); $i++)
            {
              if (is_array($data['symbols'][$i]) && isset($data['symbols'][$i]['symbol']) && $data['symbols'][$i]['symbol'] == strtoupper($requestArray['Symbol']).'.US')
              {
                $response = array();
                $bProcessed = true;
                $response['Symbol'] = substr($data['symbols'][$i]['symbol'], 0, (strlen($data['symbols'][$i]['symbol']) - 3));
                $response['Last'] = str_replace(',', '', $data['symbols'][$i]['close']);
                $response['Date'] = $data['symbols'][$i]['date'];
              }
              else
              {
                $strError = 'Please provide a valid Symbol.';
              }
            }
          }
          else
          {
            $strError = 'Encountered an unknown error.  '.json_encode($data);
          }
          unset($data);
        }
      }
      else
      {
        $strError = 'Failed to connect to the Stooq URL.';
      }
    }
    else
    {
      $strError = 'Please provide the Symbol.';
    }
  }
  // }}}
}
else
{
  $strError = 'Please provide a valid Function:  bitcoin, commodity, federal, fidelity, or stock.';
}
$requestArray['Status'] = ($bProcessed)?'okay':'error';
if ($strError != null)
{
  $requestArray['Error'] = $strError;
}
echo json_encode($requestArray)."\n";
if (is_array($response))
{
  echo json_encode($response)."\n";
}
if (is_array($extras))
{
  $nSize = sizeof($extras);
  for ($i = 0; $i < $nSize; $i++)
  {
    if (!empty($extras[$i]))
    {
      echo json_encode($extras[$i])."\n";
    }
  }
}
unset($requestArray);
unset($request);
unset($response);
unset($extras);
?>
