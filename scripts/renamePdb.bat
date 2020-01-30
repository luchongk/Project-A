@echo off

set hr=%time:~0,2%
set hr=%hr: =0%
set datetime=%date:~-4,4%-%date:~-10,2%-%date:~-7,2%_%hr%-%time:~3,2%-%time:~6,2%
ren ..\bin\game.pdb "game%datetime%.pdb"