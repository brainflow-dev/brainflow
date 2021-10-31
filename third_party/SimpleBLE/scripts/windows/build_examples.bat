@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass "& {& '%~dp0build_examples.ps1' %*}"