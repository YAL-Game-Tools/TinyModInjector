@echo off
set dllPath=%~1
set solutionDir=%~2
set projectDir=%~3
set arch=%~4
set config=%~5

copy /Y "%dllPath%" "%solutionDir%TinyModGameMakerGame\datafiles\NativeMods\TinySampleMod.dll"