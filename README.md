# wsh

An aptly named "Wisconsin" shell.

## Implemented Commands

- `cd <path to directory>`: Change directory.
- Any command located in the `usr/bin` folder, e.g., `ls`, `cat`, `uniq`, `sort`, etc.
- `bg`: Allows you to run any program in the background using an ampersand '&' after the command name. Example: `sleep 10 &`.
- `fg`: Puts a background process in the foreground.
- `jobs`: Lists all jobs that are stopped or running in the background.
- Allows you to pipe several commands together. Example: `cat myfile.txt | sort | uniq`.

