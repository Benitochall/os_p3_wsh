# wsh
An aptly named "Wisconsin" shell<br>

This shell implements some basic commands such as:<br>
cd <path to directory> - change directory<br>
Any command located in the usr/bin folder, ie ls, cat, uniq, sort etc.<br>
bg - Allows you to run any program in the background using an ampersand '&' after the command name. ie sleep 10 &<br>
fg - puts a background process in the foreground<br>
jobs - lists all jobs that are stopped or running in the bacground<br>
Allows you to pipe several commands together. ie cat myfile.txt | sort | uniq<br>
