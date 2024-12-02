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

require('Mail.php');
require('Mail/mime.php');
require('functions.php');
$bProcessed = false;
$requestArray = null;
$request = null;
$strError = null;
loadRequest($requestArray, $request);
if (isset($requestArray['To']) && $requestArray['To'] != '')
{
  $header = array();
  if (isset($requestArray['From']) && $requestArray['From'] != '')
  {
    $header['From'] = $requestArray['From'];
    $header['Reply-To'] = $requestArray['From'];
  }
  if (isset($requestArray['CC']) && $requestArray['CC'] != '')
  {
    $header['Cc'] = $requestArray['CC'];
  }
  if (isset($requestArray['BCC']) && $requestArray['BCC'] != '')
  {
    $header['Bcc'] = $requestArray['BCC'];
  }
  if (isset($requestArray['Subject']) && $requestArray['Subject'] != '')
  {
    $header['Subject'] = $requestArray['Subject'];
  }
  $mime = new Mail_mime(array('eol' => "\n"));
  if (isset($requestArray['Text']) && $requestArray['Text'] != '')
  {
    $mime->setTXTBody($requestArray['Text']);
  }
  if (isset($requestArray['HTML']) && $requestArray['HTML'] != '')
  {
    $mime->setHTMLBody($requestArray['HTML']);
  }
  $nSize = sizeof($request);
  for ($i = 1; $i < $nSize; $i++)
  {
    $data = null;
    if (($data = json_decode($request[$i], true)) != null)
    {
      if (isset($data['Data']) && isset($data['Name']) && $data['Name'] != '')
      {
        if (!isset($data['Type']) || $data['Type'] == '')
        {
          $data['Type'] = 'application/octet-stream';
        }
        $mime->addAttachment(((isset($data['Encode']) && $data['Encode'] == 'base64')?base64_decode($data['Data']):$data['Data']), $data['Type'], $data['Name'], false);
      }
    }
    else
    {
      $strError = 'Invalid JSON formatting on file attachment.';
    }
    unset($data);
  }
  $body = $mime->get();
  $headers = $mime->headers($header);
  unset($header);
  $mail = Mail::factory('mail');
  if ($mail->send($requestArray['To'], $headers, $body))
  {
    $bProcessed = true;
    $strError = 'Successfully sent the email.';
  }
  else
  {
    $strError = 'Could not send the email!';
  }
}
else
{
  $strError = 'Please provide the comma separated To email addresses.';
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
