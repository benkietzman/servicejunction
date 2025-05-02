<?php
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2011-08-15
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
require('functions.php');
$requestArray = null;
$request = null;
loadRequest($requestArray, $request);
$strData = '/data/servicejunction';
if ($argc == 2)
{
  $strData = $argv[1];
}
if (($strContent = file_get_contents($strData.'/services.conf')) !== false)
{
  $requestArray['Status'] = 'okay';
  echo json_encode($requestArray)."\n";
  echo $strContent;
}
else
{
  $requestArray['Status'] = 'error';
  $requestArray['Error'] = 'Failed to read services configuration.';
  echo json_encode($requestArray)."\n";
}
unset($requestArray);
unset($request);
?>
