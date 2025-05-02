<?php
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2011-09-15
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
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
