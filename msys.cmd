@echo off
set BIN="%~dp0msys\bin\"
call %COMSPEC% /c %BIN%bash --login -i
