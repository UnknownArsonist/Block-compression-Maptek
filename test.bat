@echo off
set START_TIME=%time%
echo Program started at: %START_TIME%

test_fast.exe 4 < the_fast_one_6412992_2x2x2.csv > test_output.txt

set END_TIME=%time%
echo Program finished at: %END_TIME%