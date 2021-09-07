@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass "& {& '%~dp0build_lib.ps1' %*}"