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
$bProcessed = false;
$requestArray = null;
$responseArray = null;
$extras = null;
$request = null;
$strError = null;
loadRequest($requestArray, $request);
if ($argc == 2)
{
  $requestArray['sysInfoServer'] = $argv[1];
}
if (!isset($requestArray['sysInfoPort']) || $requestArray['sysInfoPort'] == '')
{
  $requestArray['sysInfoPort'] = '4636';
}
if (isset($requestArray['sysInfoServer']) && $requestArray['sysInfoServer'] != '')
{
  if (isset($requestArray['Action']) && $requestArray['Action'] != '')
  {
    $strRequest = $requestArray['Action'];
    if ($requestArray['Action'] == 'message')
    {
      if (!isset($requestArray['Type']) || ($requestArray['Type'] != 'alert' && $requestArray['Type'] != 'info'))
      {
        $requestArray['Type'] = 'info';
      }
      if (!isset($requestArray['Start']))
      {
        $requestArray['Start'] = time();
      }
      if (!isset($requestArray['End']))
      {
        $requestArray['End'] = $requestArray['Start'] + 60;
      }
      if (isset($requestArray['Message']) && $requestArray['Message'] != '')
      {
        $strRequest .= ' '.$requestArray['Type'].';'.$requestArray['Application'].';'.$requestArray['Start'].';'.$requestArray['End'].';'.$requestArray['Message']."\n";
        if (($handle = fsockopen($requestArray['sysInfoServer'], $requestArray['sysInfoPort'])))
        {
          if (($nReturn = fwrite($handle, $strRequest)) !== false && $nReturn == strlen($strRequest))
          {
            $bProcessed = true;
          }
          else
          {
            $strError = 'Failed to write request.';
          }
          fclose($handle);
        }
        else
        {
          $strError = 'Failed to open connection to the System Information service.';
        }
      }
      else if (!isset($requestArray['Message']) || $requestArray['Message'] == '')
      {
        $strError = 'Please provide the Message.';
      }
    }
    else if ($requestArray['Action'] == 'process')
    {
      if (isset($requestArray['Server']) && $requestArray['Server'] != '' && isset($requestArray['Process']) && $requestArray['Process'] != '')
      {
        $strRequest .= ' '.$requestArray['Server'].' '.$requestArray['Process']."\n";
        if (($handle = fsockopen($requestArray['sysInfoServer'], $requestArray['sysInfoPort'])))
        {
          if (($nReturn = fwrite($handle, $strRequest)) !== false && $nReturn == strlen($strRequest))
          {
            if (($strResponse = trim(fgets($handle))) != '')
            {
              $data = explode(';', $strResponse);
              if (sizeof($data) == 10)
              {
                $bProcessed = true;
                $unIndex = 0;
                $responseArray = array();
                $responseArray['StartTime'] = $data[$unIndex++];
                $responseArray['Owners'] = array();
                $ownerArray = explode(',', $data[$unIndex++]);
                $unSize = sizeof($ownerArray);
                for ($i = 0; $i < $unSize; $i++)
                {
                  $item = explode('(', $ownerArray[$i]);
                  if (sizeof($item) >= 2)
                  {
                    $responseArray['Owners'][$item[0]] = str_replace(')', '', $item[1]);
                  }
                  unset($item);
                }
                unset($ownerArray);
                $responseArray['NumberOfProcesses'] = $data[$unIndex++];
                $responseArray['ImageSize'] = $data[$unIndex++];
                $responseArray['MinImageSize'] = $data[$unIndex++];
                $responseArray['MaxImageSize'] = $data[$unIndex++];
                $responseArray['ResidentSize'] = $data[$unIndex++];
                $responseArray['MinResidentSize'] = $data[$unIndex++];
                $responseArray['MaxResidentSize'] = $data[$unIndex++];
                $responseArray['Alarms'] = $data[$unIndex++];
              }
              else
              {
                $strError = 'Invalid number of fields returned.';
              }
            }
            else
            {
              $strError = 'Failed to read response.';
            }
          }
          else
          {
            $strError = 'Failed to write request.';
          }
          fclose($handle);
        }
        else
        {
          $strError = 'Failed to open connection to the System Information service.';
        }
      }
      else if (!isset($requestArray['Server']) || $requestArray['Server'] == '')
      {
        $strError = 'Please provide the Server.';
      }
      else if (!isset($requestArray['Process']) || $requestArray['Process'] == '')
      {
        $strError = 'Please provide the Process.';
      }
    }
    else if ($requestArray['Action'] == 'system')
    {
      if (isset($requestArray['Server']) && $requestArray['Server'] != '')
      {
        $strRequest .= ' '.$requestArray['Server'];
      }
      $strRequest .= "\n";
      if (($handle = fsockopen($requestArray['sysInfoServer'], $requestArray['sysInfoPort'])))
      {
        if (($nReturn = fwrite($handle, $strRequest)) !== false && $nReturn == strlen($strRequest))
        {
          $unIndex = 0;
          $extras = array();
          while (!feof($handle))
          {
            $data = explode(';', trim(fgets($handle)));
            if (sizeof($data) == 14)
            {
              $bProcessed = true;
              $unSubIndex = 0;
              $extras[$unIndex] = array();
              $extras[$unIndex]['Server'] = $data[$unSubIndex++];
              $extras[$unIndex]['OperatingSystem'] = $data[$unSubIndex++];
              $extras[$unIndex]['SystemRelease'] = $data[$unSubIndex++];
              $extras[$unIndex]['NumberOfProcessors'] = $data[$unSubIndex++];
              $extras[$unIndex]['CpuSpeed'] = $data[$unSubIndex++];
              $extras[$unIndex]['NumberOfProcesses'] = $data[$unSubIndex++];
              $extras[$unIndex]['CpuUsage'] = $data[$unSubIndex++];
              $extras[$unIndex]['UpTime'] = $data[$unSubIndex++];
              $extras[$unIndex]['MainUsed'] = $data[$unSubIndex++];
              $extras[$unIndex]['MainTotal'] = $data[$unSubIndex++];
              $extras[$unIndex]['SwapUsed'] = $data[$unSubIndex++];
              $extras[$unIndex]['SwapTotal'] = $data[$unSubIndex++];
              $extras[$unIndex]['Partitions'] = array();
              $partArray = explode(',', $data[$unSubIndex++]);
              $unSize = sizeof($partArray);
              for ($i = 0; $i < $unSize; $i++)
              {
                $item = explode('=', $partArray[$i]);
                $extras[$unIndex]['Partitions'][$item[0]] = $item[1];
              }
              unset($partArray);
              $extras[$unIndex]['Alarms'] = $data[$unSubIndex++];
              $unIndex++;
            }
            else
            {
              $strError = 'Invalid number of fields returned.';
            }
          }
        }
        else
        {
          $strError = 'Could not write request.';
        }
        fclose($handle);
      }
      else
      {
        $strError = 'Failed to open connection to the System Information service.';
      }
    }
    else if ($requestArray['Action'] == 'update')
    {
      $strRequest = "update\n";
      if (($handle = fsockopen($requestArray['sysInfoServer'], $requestArray['sysInfoPort'])))
      {
        if (($nReturn = fwrite($handle, $strRequest)) !== false && $nReturn == strlen($strRequest))
        {
          if (($strResponse = trim(fgets($handle))) == 'okay')
          {
            $bProcessed = true;
          }
          else
          {
            $strError = 'Failed to update.';
          }
        }
        else
        {
          $strError = 'Could not write request.';
        }
        fclose($handle);
      }
      else
      {
        $strError = 'Failed to open connection to the System Information service.';
      }
    }
    else
    {
      $strError = 'Please provide a valid Action.';
    }
  }
  else
  {
    $strError = 'Please provide the Action.';
  }
}
else
{
  $strError = 'Please provide the sysInfoServer.';
}
$requestArray['Status'] = ($bProcessed)?'okay':'error';
if ($strError != null)
{
  $requestArray['Error'] = $strError;
}
echo json_encode($requestArray)."\n";
if ($responseArray != null)
{
  echo json_encode($responseArray)."\n";
  unset($responseArray);
}
if ($extras != null)
{
  $nSize = sizeof($extras);
  for ($i = 0; $i < $nSize; $i++)
  {
    echo json_encode($extras[$i])."\n";
  }
  unset($extras);
}
unset($requestArray);
unset($request);
?>
