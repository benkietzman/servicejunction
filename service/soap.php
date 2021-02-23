<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2011-09-15
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
if (isset($requestArray['WSDL']) && $requestArray['WSDL'] != '')
{
  if (isset($requestArray['Function']) && $requestArray['Function'] != '')
  {
    try
    {
      $option = array();
      $option['trace'] = true;
      if (isset($requestArray['Location']) && $requestArray['Location'] != '')
      {
        $option['location'] = $requestArray['Location'];
      }
      $client = new SoapClient($requestArray['WSDL'], $option);
      unset($option);
      if ($requestArray['Function'] == 'getFunctions')
      {
        $response = $client->__getFunctions();
      }
      else
      {
        $data = json_decode($request[1], true);
        $response = $client->$requestArray['Function']($data);
        unset($data);
      }
      $bProcessed = true;
    }
    catch (Exception $e)
    {
      $strError = $e->getMessage();
    }
  }
  else
  {
    $strError = 'Please provide the Function.';
  }
}
else
{
  $strError = 'Please provide the WSDL.';
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
