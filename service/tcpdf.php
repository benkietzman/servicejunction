<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2020-07-27
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

require('/usr/share/php/tcpdf/tcpdf.php');
require('functions.php');
$bProcessed = false;
$requestArray = null;
$request = null;
$response = null;
$strError = null;
loadRequest($requestArray, $request);
if (isset($requestArray['Function']) && $requestArray['Function'] != '')
{
  if ($requestArray['Function'] == 'html2pdf')
  {
    if (sizeof($request) == 2)
    {
      $data = json_decode($request[1], true);
      if (isset($data['Html']) && $data['Html'] != '')
      {
        $bProcessed = true;
        $strFilename = 'document.pdf';
        $pdf = new TCPDF(PDF_PAGE_ORIENTATION, PDF_UNIT, PDF_PAGE_FORMAT, true, 'UTF-8', false);
        if (isset($data['Author']) && $data['Author'] != '')
        {
          $pdf->SetAuthor($data['Author']);
        }
        if (isset($data['Creator']) && $data['Creator'] != '')
        {
          $pdf->SetCreator($data['Creator']);
        }
        if (isset($data['Filename']) && $data['Filename'] != '')
        {
          $strFilename = $data['Filename'];
        }
        if (isset($data['Header']) && $data['Header'] != '')
        {
          $strSubHeader = null;
          if (isset($data['SubHeader']) && $data['SubHeader'] != '')
          {
            $strSubHeader = $data['SubHeader'];
          }
          $pdf->SetHeaderData(null, 0, $data['Header'], $strSubHeader);
        }
        if (isset($data['Title']) && $data['Title'] != '')
        {
          $pdf->SetTitle($data['Title']);
        }
        $pdf->setHeaderFont(Array(PDF_FONT_NAME_MAIN, '', PDF_FONT_SIZE_MAIN));
        $pdf->setFooterFont(Array(PDF_FONT_NAME_DATA, '', PDF_FONT_SIZE_DATA));
        $pdf->SetDefaultMonospacedFont(PDF_FONT_MONOSPACED);
        $pdf->SetMargins(PDF_MARGIN_LEFT, PDF_MARGIN_TOP, PDF_MARGIN_RIGHT);
        $pdf->SetHeaderMargin(PDF_MARGIN_HEADER);
        $pdf->SetFooterMargin(PDF_MARGIN_FOOTER);
        $pdf->SetAutoPageBreak(TRUE, PDF_MARGIN_BOTTOM);
        $pdf->setImageScale(PDF_IMAGE_SCALE_RATIO);
        $pdf->AddPage();
        $pdf->writeHTML($data['Html'], true, false, true, false, '');
        $pdf->lastPage();
        $attachment = explode("\r\n\r\n", $pdf->Output($strFilename, 'E'));
        $response = [];
        $response['Data'] = $attachment[1];
        unset($attachment);
      }
      else
      {
        $strError = 'Please provide the Html.';
      }
      unset($data);
    }
    else
    {
      $strError = 'Received '.sizeof($request).' lines when expecting 2 lines in the request.';
    }
  }
  else
  {
    $strError = 'Please provide a valid Function:  html2pdf.';
  }
}
else
{
  $strError = 'Please provide the Function.';
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
unset($response);
unset($requestArray);
unset($request);
?>
