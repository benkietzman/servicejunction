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

error_reporting(0);
include('/src/common/www/database/Database.php');
include('/scripts/common/www/database/Database.php');
require('functions.php');
$bProcessed = false;
$requestArray = null;
$responseArray = null;
$request = null;
$strError = null;
$database = new Database('Oracle');
loadRequest($requestArray, $request);
if (isset($requestArray['Schema']) && $requestArray['Schema'] != '' && isset($requestArray['Password']) && $requestArray['Password'] != '' && isset($requestArray['tnsName']) && $requestArray['tnsName'] != '' && ((isset($requestArray['Query']) && $requestArray['Query'] != '') || (isset($requestArray['Update']) && $requestArray['Update'] != '')))
{
  $db = $database->connect($requestArray['Schema'], $requestArray['Password'], $requestArray['tnsName']);
  if (!$db->errorExist())
  {
    $strQuery = null;
    if (isset($requestArray['Query']) && $requestArray['Query'] != '')
    {
      $strQuery = $requestArray['Query'];
    }
    else if (isset($requestArray['Update']) && $requestArray['Update'] != '')
    {
      $strQuery = $requestArray['Update'];
    }
    $query = $db->parse($strQuery);
    if (!$query->errorExist())
    {
      if ($query->execute())
      {
        $bProcessed = true;
        if (isset($requestArray['Query']) && $requestArray['Query'] != '')
        {
          $unIndex = 0;
          $responseArray = array();
          while (($row = $query->fetch('assoc')))
          {
            foreach ($row as $key => $value)
            {
              if (is_object($row[$key]))
              {
                $lob = $row[$key]->load();
                $row[$key]->free();
                $row[$key] = base64_encode($lob);
              }
            }
            $responseArray[$unIndex++] = $row;
          }
        }
        else if (isset($requestArray['Update']) && $requestArray['Update'] != '')
        {
          $requestArray['Rows'] = $query->numRows();
        }
        $db->free($query);
      }
      else
      {
        $strError = $query->getError();
      }
    }
    else
    {
      $strError = $query->getError();
    }
  }
  else
  {
    $strError = $db->getError();
  }
  $database->disconnect($db);
}
else if (!isset($requestArray['Schema']) || $requestArray['Schema'] == '')
{
  $strError = 'Please provide the Oracle Schema.';
}
else if (!isset($requestArray['Password']) || $requestArray['Password'] == '')
{
  $strError = 'Please provide the Oracle Password.';
}
else if (!isset($requestArray['tnsName']) || $requestArray['tnsName'] == '')
{
  $strError = 'Please provide the Oracle tnsName.';
}
else if ((!isset($requestArray['Query']) || $requestArray['Query'] == '') && (!isset($requestArray['Update']) || $requestArray['Update'] == ''))
{
  $strError = 'Please provide the Oracle Query or Update.';
}
$requestArray['Status'] = ($bProcessed)?'okay':'error';
if ($strError != null)
{
  $requestArray['Error'] = $strError;
}
echo json_encode($requestArray)."\n";
if ($responseArray != null)
{
  $unSize = sizeof($responseArray);
  for ($i = 0; $i < $unSize; $i++)
  {
    echo json_encode($responseArray[$i])."\n";
  }
  unset($responseArray);
}
unset($requestArray);
unset($request);
?>
