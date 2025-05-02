<?php
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2016-02-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
$strPath = dirname(__FILE__).'/../..';
if ($argc == 2)
{
  $strPath = $argv[1];
}
require($strPath.'/common/www/Central.php');
require('functions.php');
$bProcessed = false;
$requestArray = null;
$request = null;
$response = null;
$strError = null;
loadRequest($requestArray, $request);
if (isset($requestArray['User']) && $requestArray['User'] != '')
{
  if (isset($requestArray['Password']) && $requestArray['Password'] != '')
  {
      if (isset($requestArray['Database']) && $requestArray['Database'] != '')
      {
        if (isset($requestArray['Function']) && $requestArray['Function'] != '')
        {
          $strServer = null;
          if (isset($requestArray['Server']) && $requestArray['Server'] != '')
          {
            $strServer = $requestArray['Server'];
          }
          $bProcessed = true;
          $central = new Central($requestArray['User'], $requestArray['Password'], $strServer, $requestArray['Database']);
          if (isset($requestArray['Jwt']) && $requestArray['Jwt'] != '')
          {
            if (!$central->loadJwt($requestArray['Jwt'], $strError))
            {
              $bProcessed = false;
            }
          }
          else if (isset($requestArray['SessionID']) && $requestArray['SessionID'] != '')
          {
            if (!$central->loadSession($requestArray['SessionID'], $strError))
            {
              $bProcessed = false;
            }
          }
          if ($bProcessed)
          {
            $bProcessed = false;
            $in = null;
            if (sizeof($request) == 2)
            {
              $in = json_decode($request[1], true);
            }
            if (!is_array($in))
            {
              $in = array();
            }
            if (method_exists($central, $requestArray['Function']))
            {
              if ($central->{$requestArray['Function']}($in, $response, $strError))
              {
                $bProcessed = true;
              }
            }
            else
            {
              $strError = 'Please provide a valid Function.';
            }
            unset($in);
          }
        }
        else
        {
          $strError = 'Please provide the Function.';
        }
      }
      else
      {
        $strError = 'Please provide the Database.';
      }
  }
  else
  {
    $strError = 'Please provide the Password.';
  }
}
else
{
  $strError = 'Please provide the User.';
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
unset($requestArray);
unset($request);
?>
