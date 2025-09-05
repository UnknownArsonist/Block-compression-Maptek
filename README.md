PROJECT LAYOUT :

STREAM_PROCESSSOR
|
-> PROCESSOR MODULE
|
-> INPUT_STREAM_READER MODULE
|
-----> COMPRESSOR MODULE
|--------> OctreeCompression
|--------> BruteForceCompression  
|--------> Alg1
|
-> DISPLAY_OUTPUT MODULE
|
-> STREAM BUFFER MODULE

RULES FOR GROUP MEMBERS
-Could use fwrite() from "cstdio" library instead of printf() for printing
-Use your own branch, don't use main
