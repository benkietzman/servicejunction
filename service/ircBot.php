<?php
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2011-08-15
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
require('functions.php');
$bProcessed = false;
$requestArray = null;
$request = null;
$strError = null;
loadRequest($requestArray, $request);
if (!isset($requestArray['Server']) || $requestArray['Server'] == '')
{
  $requestArray['Server'] = 'localhost';
  if (file_exists('/etc/central.conf') && ($handle = fopen('/etc/central.conf', 'r')) !== false)
  {
    $conf = json_decode(fgets($handle), true);
    if (isset($conf['ChatBot']) && $conf['ChatBot'] != '')
    {
      $requestArray['Server'] = $conf['ChatBot'];
    }
  }
}
if (!isset($requestArray['Port']) || $requestArray['Port'] == '')
{
  $requestArray['Port'] = '7267';
}
$strSender = 'system';
if (isset($requestArray['Sender']) && $requestArray['Sender'] != '')
{
  $strSender = $requestArray['Sender'];
}
if (isset($requestArray['Room']) && $requestArray['Room'] != '')
{
  if (isset($requestArray['Message']) && $requestArray['Message'] != '')
  {
    if (($handle = fsockopen('ssl://'.$requestArray['Server'], $requestArray['Port'])))
    {
      $strRoom = $requestArray['Room'];
      if ($strRoom[0] != '@' && $strRoom[0] != '#')
      {
        $strRoom = '@'.$strRoom;
      }
      $data = array();
      $data['Channel'] = $strRoom;
      $data['Sender'] = $strSender;
      $data['Message'] = $requestArray['Room'].' '.$requestArray['Message'];
      $strRequest = json_encode($data)."\n";
      unset($data);
      if (($nReturn = fwrite($handle, $strRequest)) !== false && $nReturn == strlen($strRequest))
      {
        $bProcessed = true;
        $strError = 'Successfully posted the message.';
      }
      else
      {
        $strError = 'Could not write message!';
      }
      fclose($handle);
    }
    else
    {
      $strError = 'Failed to open socket connection to the IRC Bot service.';
    }
  }
  else
  {
    $strError = 'Please provide the Message.';
  }
}
else
{
  $strError = 'Please provide the Room.';
}
$requestArray['Status'] = ($bProcessed)?'okay':'error';
if ($strError != null)
{
  $requestArray['Error'] = $strError;
}
echo json_encode($requestArray)."\n";
unset($requestArray);
unset($request);
?>
