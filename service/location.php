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
$strError = null;
loadRequest($requestArray, $request);
if (isset($requestArray['State']) && $requestArray['State'] != '')
{
  if (isset($requestArray['City']) && $requestArray['City'] != '')
  {
    if (isset($requestArray['Address']) && $requestArray['Address'] != '')
    {
      try
      {
        $client = new SoapClient('http://geocoder.us/dist/eg/clients/GeoCoderPHP.wsdl', []);
        $response = $client->geocode(array($requestArray['Address'].', '.$requestArray['City'].', '.$requestArray['State']));
        $bProcessed = true;
      }
      catch (Exception $e)
      {
        $strError = $e->getMessage();
      }
    }
    else
    {
      $strError = 'Please provide the Address.';
    }
  }
  else
  {
    $strError = 'Please provide the City.';
  }
}
else
{
  $strError = 'Please provide the Country.';
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
