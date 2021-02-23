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

require('nusoap/nusoap.php');
require('functions.php');
$bProcessed = false;
$requestArray = null;
$request = null;
$response = null;
$strError = null;
$strTimeout = '120';
loadRequest($requestArray, $request);
if (isset($requestArray['Timeout']) && $requestArray['Timeout'] != '')
{
  $strTimeout = $requestArray['Timeout'];
}
if (isset($requestArray['Country']) && $requestArray['Country'] != '' && isset($requestArray['City']) && $requestArray['City'] != '')
{
  $client = new nusoap_client('http://www.webservicex.com/globalweather.asmx?WSDL', 'wsdl', true, false, false, false, $strTimeout, $strTimeout);
  if (($strError = $client->getError()) == '')
  {
    $client->soap_defencoding = 'UTF-8';
    $result = $client->call('GetWeather', array('CityName'=>$requestArray['City'], 'CountryName'=>$requestArray['Country']));
    if (($strError = $client->getError()) == '')
    {
      if (isset($result['GetWeatherResult']) && $result['GetWeatherResult'] != '')
      {
        $bProcessed = true;
        $response = simplexml_load_string(str_replace('<?xml version="1.0" encoding="utf-16"?>'."\n", '', $result['GetWeatherResult']));
      }
      else
      {
        $strError = 'Failed to find GetWeatherResult in the response.';
      }
    }
    unset($result);
  }
}
else if (isset($requestArray['ZIP']) && $requestArray['ZIP'] != '')
{
  $client = new nusoap_client('http://wsf.cdyne.com/WeatherWS/Weather.asmx?WSDL', 'wsdl', true, false, false, false, $strTimeout, $strTimeout);
  if (($strError = $client->getError()) == '')
  {
    $client->soap_defencoding = 'UTF-8';
    $result = $client->call('GetCityWeatherByZIP', array('ZIP'=>$requestArray['ZIP']));
    if (($strError = $client->getError()) == '')
    {
      if (isset($result['GetCityWeatherByZIPResult']) && is_array($result['GetCityWeatherByZIPResult']))
      {
        $bProcessed = true;
        $response = $result['GetCityWeatherByZIPResult'];
      }
      else
      {
        $strError = 'Failed to find GetCityWeatherByZIPResult in the response.';
      }
    }
    unset($result);
  }
}
else
{
  $strError = 'Please provide the Country and City or provide the ZIP.';
}
$requestArray['Status'] = ($bProcessed)?'okay':'error';
if ($strError != null)
{
  $requestArray['Error'] = $strError;
}
echo json_encode($requestArray)."\n";
if ($response != null)
{
  echo json_encode($response)."\n";
}
unset($requestArray);
unset($request);
unset($response);
?>
