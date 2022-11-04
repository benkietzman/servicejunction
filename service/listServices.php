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
