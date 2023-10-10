# wsh
An aptly named "Wisconsin" shell\n

This shell implements some basic commands such as:\n
cd <path to directory> - change directory\n
Any command located in the usr/bin folder, ie ls, cat, uniq, sort etc.\n
bg - Allows you to run any program in the background using an ampersand '&' after the command name. ie sleep 10 &\n
fg - puts a background process in the foreground\n
jobs - lists all jobs that are stopped or running in the bacground\n
Allows you to pipe several commands together. ie cat myfile.txt | sort | uniq\n
