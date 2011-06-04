@echo off
rem ------[ DEFAULT VARS ]------
set signtool=signtool.exe
set cert_store=PrivateCertStore
set cert_name="TestCertforWDK"
set ts_srv="http://timestamp.globalsign.com/scripts/timstamp.dll"
set sys_path=%1
rem --------------------

echo --------------[ sign ]--------------
%signtool% sign /v /s %cert_store% /n %cert_name% /t %ts_srv% %sys_path%

pause

rem ***(C) vol4ok, 2009***