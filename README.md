# wsh
An aptly named "Wisconsin" shell

This shell implements some basic commands such as:
cd <path to directory> - change directory
Any command located in the usr/bin folder, ie ls, cat, uniq, sort etc. 
bg - Allows you to run any program in the background using an ampersand '&' after the command name. ie sleep 10 &
fg - puts a background process in the foreground
jobs - lists all jobs that are stopped or running in the bacground 
Allows you to pipe several commands together. ie cat myfile.txt | sort | uniq
